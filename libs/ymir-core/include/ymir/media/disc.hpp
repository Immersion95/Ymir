#pragma once

#include <ymir/core/types.hpp>

#include <ymir/media/binary_reader/binary_reader.hpp>

#include "saturn_header.hpp"
#include "subheader.hpp"

// #include <fmt/format.h>

#include <array>
#include <cassert>
#include <memory>
#include <span>
#include <vector>

namespace ymir::media {

struct Index {
    uint32 startFrameAddress = 0;
    uint32 endFrameAddress = 0;
};

struct Track {
    std::unique_ptr<IBinaryReader> binaryReader;
    uint32 index = 0;
    uint32 sectorSize = 0;
    uint32 userDataOffset = 0;
    uint8 controlADR = 0;
    bool interleavedSubchannel = false; // true=96-byte PW subchannel, interleaved
    bool bigEndian = false;             // indicates audio data endianness on audio tracks

    uint32 startFrameAddress = 0;
    uint32 endFrameAddress = 0;

    std::vector<Index> indices;

    uint8 FindIndex(uint32 frameAddress) const {
        auto it = std::find_if(indices.begin(), indices.end(), [=](const Index &index) {
            return frameAddress >= index.startFrameAddress && frameAddress <= index.endFrameAddress;
        });

        if (it == indices.end()) {
            return 0xFF;
        } else {
            return std::distance(indices.begin(), it) + 1;
        }
    }

    void SetSectorSize(uint32 size) {
        sectorSize = size;
        userDataOffset = size >= 2352 ? 16 : size >= 2340 ? 4 : 0;
    }

    // Reads the user data portion of a sector.
    // Returns true if the sector was read successfully.
    // Returns false if the sector could not be fully read or the frame address is out of range.
    // TODO: support CD-ROM XA mode 2 form 2 user data (2324 bytes)
    bool ReadSectorUserData(uint32 frameAddress, std::span<uint8, 2048> outBuf) const {
        if (frameAddress < startFrameAddress || frameAddress > endFrameAddress) [[unlikely]] {
            return false;
        }

        const uint32 sectorOffset = (frameAddress - startFrameAddress) * sectorSize;
        return binaryReader->Read(sectorOffset + userDataOffset, 2048, outBuf) == 2048;
    }

    // Reads a sector of the specified size from the given absolute frame address.
    // If the output size is larger than the track sector size, the missing parts are synthesized in the output buffer.
    // The following sizes are supported:
    // - 2048 bytes: reads only the user data portion of the sector
    // - 2336 bytes: appends the subheader (checksums and ECC)
    // - 2340 bytes: prepends the header (min:sec:frac and mode)
    // - 2352 bytes: prepends the sync bytes
    // Returns true if the sector was read successfully.
    // Returns false if the sector could not be fully read, the frame address is out of range or the requested size is
    // unsupported.
    bool ReadSector(uint32 frameAddress, std::span<uint8, 2352> outBuf, uint32 targetSize) const {
        assert(targetSize == 2048 || targetSize == 2336 || targetSize == 2340 || targetSize == 2352);

        if (frameAddress < startFrameAddress || frameAddress > endFrameAddress) [[unlikely]] {
            return false;
        }

        // Audio tracks always have 2352 bytes
        if (controlADR == 0x01) {
            const uint32 sectorOffset = (frameAddress - startFrameAddress) * sectorSize;
            const uintmax_t readSize = binaryReader->Read(sectorOffset, 2352, outBuf);
            return readSize == 2352;
        }

        // Determine which components are needed and which are present
        const bool needsSyncBytes = targetSize >= 2352;
        const bool needsHeader = targetSize >= 2340;
        const bool needsSubheader = targetSize >= 2336;

        const bool hasSyncBytes = sectorSize >= 2352;
        const bool hasHeader = sectorSize >= 2340;
        const bool hasSubheader = sectorSize >= 2336;

        // Determine how much data needs to be skipped from the raw sector read and where we need to write it
        const uint32 readOffset = (!needsSyncBytes && hasSyncBytes) * 12 + (!needsHeader && hasHeader) * 4;
        const uint32 writeOffset = (needsSyncBytes && !hasSyncBytes) * 12 + (needsHeader && !hasHeader) * 4;

        // Try to read raw sector data based on specifications
        const uint32 sectorOffset = (frameAddress - startFrameAddress) * sectorSize + readOffset;
        const std::span<uint8> output{outBuf.begin() + writeOffset, targetSize};
        const uintmax_t readSize = binaryReader->Read(sectorOffset, targetSize, output);
        if (readSize != targetSize) {
            return false;
        }

        /*fmt::println("== SECTOR DUMP - FAD {:X} ==", frameAddress);
        fmt::println("Track sector size: {} bytes", sectorSize);
        fmt::println("Requested size:    {} bytes", targetSize);*/

        // Fill in any missing data
        if (needsSyncBytes && !hasSyncBytes) {
            static constexpr std::array<uint8, 12> syncBytes = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                                                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};
            std::copy(syncBytes.begin(), syncBytes.end(), outBuf.begin());
            // fmt::println("  Added sync bytes");
        }
        if (needsHeader && !hasHeader) {
            // Convert absolute frame address to min:sec:frac
            outBuf[0xC] = frameAddress / 75 / 60;
            outBuf[0xD] = (frameAddress / 75) % 60;
            outBuf[0xE] = frameAddress % 75;

            // Determine mode based on track type and sector size
            if (controlADR == 0x41) {
                // Data track
                if (sectorSize == 2336) {
                    // Mode 2 form 1
                    outBuf[0xF] = 0x02;
                } else {
                    // Mode 1
                    outBuf[0xF] = 0x01;
                }
            } else {
                // Audio track
                outBuf[0xF] = 0x00;
            }
            // fmt::println("  Added header");
        }
        if (needsSubheader && !hasSubheader) {
            // Fill out checksums after user data
            // TODO: actually calculate checksums
            // for now, we'll fill the checksums with zeros and hope that no software ever checks them
            std::fill_n(outBuf.begin() + writeOffset + 2048, 288, 0x00);
            // fmt::println("  Added subheader");
        }
        /*fmt::println("Raw sector data:");
        for (uint32 i = 0; i < targetSize; i++) {
            fmt::print("{:02X}", outBuf[i]);
            if (i % 32 == 31) {
                fmt::println("");
            } else {
                fmt::print(" ");
            }
        }
        fmt::println("");*/

        return true;
    }

    void ReadSectorSubheader(uint32 frameAddress, Subheader &subheader) const {
        subheader.fileNum = 0;
        subheader.chanNum = 0;
        subheader.submode = 0;
        subheader.codingInfo = 0;

        if (frameAddress < startFrameAddress || frameAddress > endFrameAddress) [[unlikely]] {
            return;
        }

        // Subheader is only present when the raw sector size is at least 2336 bytes long
        if (sectorSize < 2336) {
            return;
        }

        // Read mode byte and subheader
        const bool hasSyncBytes = sectorSize >= 2352;
        std::array<uint8, 5> modeAndSubheader{};
        if (binaryReader->Read(hasSyncBytes ? 0xF : 0x3, 5, modeAndSubheader) < 5) {
            return;
        }

        // Subheader is only present in mode 2 tracks
        if (modeAndSubheader[0] != 0x02) {
            return;
        }

        // Fill in subheader data
        subheader.fileNum = modeAndSubheader[1];
        subheader.chanNum = modeAndSubheader[2];
        subheader.submode = modeAndSubheader[3];
        subheader.codingInfo = modeAndSubheader[4];
    }
};

struct Session {
    std::array<Track, 99> tracks;
    uint32 numTracks = 0;
    uint32 firstTrackIndex = 0;
    uint32 lastTrackIndex = 0;

    uint32 startFrameAddress = 0;
    uint32 endFrameAddress = 0;

    Session() {
        for (int i = 0; i < tracks.size(); i++) {
            tracks[i].index = i + 1;
        }
        toc.fill(0xFFFFFFFF);
    }

    const Track *FindTrack(uint32 absFrameAddress) const {
        const uint8 trackIndex = FindTrackIndex(absFrameAddress);
        if (trackIndex != 0xFF) {
            return &tracks[trackIndex];
        }
        return nullptr;
    }

    uint8 FindTrackIndex(uint32 absFrameAddress) const {
        for (int i = 0; i < numTracks; i++) {
            const auto &track = tracks[firstTrackIndex + i];
            if (absFrameAddress >= track.startFrameAddress && absFrameAddress <= track.endFrameAddress) {
                return firstTrackIndex + i;
            }
        }
        return 0xFF;
    }

    // The table of contents contains the following entries:
    // (partially from https://www.ecma-international.org/wp-content/uploads/ECMA-394_1st_edition_december_2010.pdf)
    //
    // 0-98: One entry per track in the following format:
    //   31-24  track control/ADR
    //   23-0   track start frame address
    // Unused tracks contain 0xFFFFFFFF
    //
    // 99: Point A0
    //   31-24  first track control/ADR
    //   23-16  first track number (PMIN)
    //   15-8   program area format (PSEC):
    //            0x00: CD-DA and CD-ROM
    //            0x10: CD-i
    //            0x20: CD-ROM-XA
    //    7-0   PFRAME - always zero
    //
    // 100: Point A1
    //   31-24  last track control/ADR
    //   23-16  last track number (PMIN)
    //   15-8   PSEC - always zero
    //    7-0   PFRAME - always zero
    //
    // 101: Point A2
    //   31-24  leadout track control/ADR
    //   23-0   leadout frame address

    std::array<uint32, 99 + 3> toc;

    // Build table of contents using track information
    void BuildTOC() {
        uint32 firstTrackNum = 0;
        uint32 lastTrackNum = 0;
        for (int i = 0; i < 99; i++) {
            auto &track = tracks[i];
            if (track.controlADR != 0x00) {
                toc[i] = (track.controlADR << 24u) | track.startFrameAddress;
                if (firstTrackNum == 0) {
                    firstTrackNum = i + 1;
                }
                lastTrackNum = i + 1;
            } else {
                toc[i] = 0xFFFFFFFF;
            }
        }

        if (firstTrackNum != 0) {
            toc[99] = (tracks[firstTrackNum - 1].controlADR << 24u) | (firstTrackNum << 16u);
            toc[100] = (tracks[lastTrackNum - 1].controlADR << 24u) | (lastTrackNum << 16u);
            toc[101] = (tracks[lastTrackNum - 1].controlADR << 24u) | endFrameAddress;
        } else {
            toc[99] = toc[100] = toc[101] = 0xFFFFFFFF;
        }
    }
};

struct Disc {
    std::vector<Session> sessions;

    SaturnHeader header;

    Disc() {
        Invalidate();
    }

    Disc(const Disc &) = delete;
    Disc(Disc &&) = default;

    Disc &operator=(const Disc &) = delete;
    Disc &operator=(Disc &&) = default;

    void Swap(Disc &&disc) {
        sessions.swap(disc.sessions);
        header.Swap(std::move(disc.header));
    }

    void Invalidate() {
        sessions.clear();
        header.Invalidate();
    }
};

} // namespace ymir::media
