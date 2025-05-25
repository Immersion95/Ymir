#include <ymir/media/filesystem.hpp>

#include <ymir/util/scope_guard.hpp>

#include <xxh3.h>

#include <cassert>
#include <vector>

using namespace ymir::media::iso9660;

namespace ymir::media::fs {

void Filesystem::Clear() {
    m_directories.clear();
    m_currDirectory = ~0;
    m_currFileOffset = 0;
    m_hash.fill(0);
}

bool Filesystem::Read(const Disc &disc) {
    Clear();
    util::ScopeGuard sgInvalidate = [&] { Clear(); };

    // TODO: test multisession discs

    // Bail out if there are no sessions in the disc
    if (disc.sessions.empty()) {
        return false;
    }

    // The Saturn uses the volume descriptor from the final session on the disc
    const Session &session = disc.sessions.back();

    // The volume descriptor is at frame address 166 (00:02:16) from the start of the session
    const uint32 absVolumeDescAddress = session.startFrameAddress + 166;

    // Find the track containing the frame address
    const Track *pTrack = session.FindTrack(absVolumeDescAddress);
    if (pTrack == nullptr) {
        // Could not find track with the specified frame address
        return false;
    }
    const Track &track = *pTrack;
    if (track.controlADR != 0x41) {
        // Not a data track
        return false;
    }

    // Buffer for sector data
    std::array<uint8, 2048> buf{};

    // Found the track; begin hashing the first 16 sectors
    XXH3_state_t *xxh3State = XXH3_createState();
    assert(xxh3State != NULL && "Out of memory!");
    util::ScopeGuard sgFreeXXH3State{[&] { XXH3_freeState(xxh3State); }};
    XXH3_128bits_reset(xxh3State);
    for (uint32 sectorIndex = 150; sectorIndex < 166; sectorIndex++) {
        // Fail if we can't read the sector
        if (!track.ReadSectorUserData(sectorIndex, buf)) {
            return false;
        }
        XXH3_128bits_update(xxh3State, buf.data(), buf.size());
    }

    // Read volume descriptors; hash these sectors as well
    const uint32 volumeDescAddress = absVolumeDescAddress;
    for (uint32 sectorIndex = volumeDescAddress;; sectorIndex++) {
        // Fail if we can't read the sector
        if (!track.ReadSectorUserData(sectorIndex, buf)) {
            return false;
        }
        XXH3_128bits_update(xxh3State, buf.data(), buf.size());

        // Try reading volume descriptor; fail if invalid
        VolumeDescriptorHeader volDescHeader{};
        if (!volDescHeader.Read(buf)) {
            return false;
        }

        // Succeed if we found a terminator
        if (volDescHeader.type == VolumeDescriptorType::Terminator) {
            sgInvalidate.Cancel();
            if (!IsValid()) {
                return false;
            } else {
                m_currDirectory = 0;
                m_currFileOffset = 0;
                XXH128_hash_t hash = XXH3_128bits_digest(xxh3State);
                XXH128_canonical_t canonicalHash{};
                XXH128_canonicalFromHash(&canonicalHash, hash);
                std::copy_n(canonicalHash.digest, m_hash.size(), m_hash.begin());
                return true;
            }
        }

        // Parse the different types of volume descriptors
        // TODO: parse supplementary/enhanced volume descriptors, and maybe volume partition descriptors too
        if (volDescHeader.type == VolumeDescriptorType::Primary) {
            VolumeDescriptor volDesc{};
            if (!volDesc.Read(buf)) {
                return false;
            }

            // Try reading the path table records from the disc; fail on error
            if (!ReadPathTableRecords(track, volDesc)) {
                return false;
            }
        }
    }

    // Fail if we somehow get here without finding a terminator
    return false;
}

bool Filesystem::ChangeDirectory(uint32 fileID) {
    if (!IsValid()) {
        return false;
    }

    if (fileID == 0xFFFFFF) {
        // Go to root directory; should be the first in the list
        m_currDirectory = 0;
    } else if (fileID == 0) {
        // Self directory; no change
    } else if (m_currDirectory != ~0 && fileID == 1) {
        // Go to parent directory
        m_currDirectory = m_directories[m_currDirectory].m_parent - 1;
    } else if (m_currDirectory != ~0 && fileID - 2 + m_currFileOffset < m_directories.size()) {
        // Go to specified directory
        m_currDirectory = fileID - 2 + m_currFileOffset;
    } else {
        // File ID out of range or invalid current directory
        return false;
    }

    m_currFileOffset = 0;
    return true;
}

std::string Filesystem::GetCurrentPath() const {
    if (!IsValid()) {
        return "";
    }
    if (!HasCurrentDirectory()) {
        return "";
    }
    assert(m_currDirectory < m_directories.size());

    if (m_currDirectory == 0) {
        // Root directory
        return "/";
    }

    // Build path from components
    std::vector<uint32> fullPath{};
    uint32 currDir = m_currDirectory;
    fullPath.push_back(currDir);
    while (currDir != 0 && fullPath.size() < 32) {
        currDir = m_directories[currDir].m_parent - 1; // 1-indexed
        if (currDir != 0) {
            fullPath.push_back(currDir);
        }
    }

    std::string out{};

    bool first = true;
    for (auto it = fullPath.rbegin(); it != fullPath.rend(); ++it) {
        if (first) {
            first = false;
        } else {
            out += "/";
        }
        out += m_directories[*it].m_name;
    }
    return out;
}

uint32 Filesystem::GetFileCount() const {
    if (!IsValid()) {
        // No file system loaded
        return 0;
    }
    if (!HasCurrentDirectory()) {
        // Invalid directory
        return 0;
    }
    assert(m_currDirectory < m_directories.size());

    return m_directories[m_currDirectory].GetContents().size() - 2;
}

const FileInfo &Filesystem::GetFileInfoWithOffset(uint8 fileID) const {
    if (!IsValid()) {
        // No file system loaded
        return kEmptyFileInfo;
    }
    if (!HasCurrentDirectory()) {
        // Invalid directory
        return kEmptyFileInfo;
    }
    const auto offset = fileID + m_currFileOffset;
    const auto &currDirContents = m_directories[m_currDirectory].GetContents();
    if (offset >= currDirContents.size()) {
        return kEmptyFileInfo;
    }
    return currDirContents[offset].GetFileInfo();
}

const FileInfo &media::fs::Filesystem::GetFileInfo(uint32 fileID) const {
    if (!IsValid()) {
        // No file system loaded
        return kEmptyFileInfo;
    }
    if (!HasCurrentDirectory()) {
        // Invalid directory
        return kEmptyFileInfo;
    }
    const auto &currDirContents = m_directories[m_currDirectory].GetContents();
    if (fileID >= currDirContents.size()) {
        return kEmptyFileInfo;
    }
    return currDirContents[fileID].GetFileInfo();
}

bool Filesystem::ReadPathTableRecords(const Track &track, const VolumeDescriptor &volDesc) {
    // Fail if there is no LSB path table
    // TODO: support MSB path table
    if (volDesc.pathTableLPos == 0) {
        return false;
    }

    // Buffer for path table sector data
    std::array<uint8, 2048> pathTableBuf{};

    // Buffer for directory record sector data
    std::array<uint8, 2048> dirRecBuf{};

    // Read all path table records
    const uint32 pathSectorCount = (volDesc.pathTableSize + 2047) / 2048;
    for (uint32 pathSectorIndex = 0; pathSectorIndex < pathSectorCount; pathSectorIndex++) {
        if (!track.ReadSectorUserData(volDesc.pathTableLPos + pathSectorIndex + 150, pathTableBuf)) {
            return false;
        }

        PathTableRecord pathTableRecord{};
        for (uint32 pathRecIndex = 0;; pathRecIndex += pathTableRecord.recordSize) {
            // Try reading the path table record
            if (!pathTableRecord.Read({pathTableBuf.begin() + pathRecIndex, pathTableBuf.end()})) {
                return false;
            }
            // Bail out if this is the last record in the current sector
            if (pathTableRecord.recordSize == 0) {
                break;
            }
            // TODO: read extended attributes if present

            // Try reading the directory record
            DirectoryRecord dirRecord{};
            if (!track.ReadSectorUserData(pathTableRecord.extentPos + 150, dirRecBuf)) {
                return false;
            }
            if (!dirRecord.Read(dirRecBuf)) {
                return false;
            }
            // Fail if there is no directory record
            if (dirRecord.recordSize == 0) {
                return false;
            }
            // Fail if it's not a directory
            if (!bit::test<1>(dirRecord.flags)) {
                return false;
            }

            // Create a directory entry
            Directory &directory =
                m_directories.emplace_back(dirRecord, pathTableRecord.parentDirNumber, pathTableRecord.directoryID);
            auto &contents = directory.GetContents();

            // Read directory contents
            const uint32 dirSectorCount = (dirRecord.dataSize + 2047) / 2048;
            for (uint32 dirSectorIndex = 0; dirSectorIndex < dirSectorCount; dirSectorIndex++) {
                if (!track.ReadSectorUserData(dirRecord.extentPos + dirSectorIndex + 150, dirRecBuf)) {
                    return false;
                }

                uint32 dirRecOffset = 0;
                DirectoryRecord subdirRecord{};
                for (;;) {
                    // Try reading the directory record
                    if (!subdirRecord.Read({dirRecBuf.begin() + dirRecOffset, dirRecBuf.end()})) {
                        break;
                    }
                    // Bail out if this is the end of the list
                    if (subdirRecord.recordSize == 0) {
                        break;
                    }
                    // TODO: read extended attributes if present
                    const uint8 fileNum = 0;

                    // Add record to directory
                    contents.emplace_back(subdirRecord, pathRecIndex + 1, fileNum);

                    dirRecOffset += subdirRecord.recordSize;
                }
            }
        }
    }

    return true;
}

} // namespace ymir::media::fs
