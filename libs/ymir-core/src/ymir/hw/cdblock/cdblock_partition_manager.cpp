#include <ymir/hw/cdblock/cdblock.hpp>

#include "cdblock_devlog.hpp"

#include <cassert>
#include <numeric>
#include <utility>

namespace ymir::cdblock {

CDBlock::PartitionManager::PartitionManager() {
    Reset();
}

void CDBlock::PartitionManager::Reset() {
    m_partitions.fill({});
    m_freeBuffers = kNumBuffers;
    devlog::trace<grp::part_mgr>("Cleared partitions; free buffers = {}", m_freeBuffers);
}

uint8 CDBlock::PartitionManager::GetBufferCount(uint8 partitionIndex) const {
    assert(partitionIndex < m_partitions.size());
    devlog::trace<grp::part_mgr>("Partition {} has {} buffers", partitionIndex, m_partitions[partitionIndex].size());
    return m_partitions[partitionIndex].size();
}

uint32 CDBlock::PartitionManager::GetFreeBufferCount() const {
    devlog::trace<grp::part_mgr>("Free buffers = {}", m_freeBuffers);
    return m_freeBuffers;
}

void CDBlock::PartitionManager::InsertHead(uint8 partitionIndex, Buffer &buffer) {
    assert(partitionIndex < m_partitions.size());
    assert(m_freeBuffers > 0);
    auto &partition = m_partitions[partitionIndex];
    partition.push_back(buffer);
    m_freeBuffers--;
    devlog::trace<grp::part_mgr>("Inserted buffer into partition {} -> {} buffers; free buffers = {}", partitionIndex,
                                 partition.size(), m_freeBuffers);
}

Buffer *CDBlock::PartitionManager::GetTail(uint8 partitionIndex, uint8 offset) {
    assert(partitionIndex < m_partitions.size());
    auto &partition = m_partitions[partitionIndex];
    return partition.empty() ? nullptr : &*(partition.begin() + offset);
}

bool CDBlock::PartitionManager::RemoveTail(uint8 partitionIndex, uint8 offset) {
    assert(partitionIndex < m_partitions.size());
    auto &partition = m_partitions[partitionIndex];
    if (offset < partition.size()) {
        partition.erase(partition.begin() + offset);
        m_freeBuffers++;
        devlog::trace<grp::part_mgr>("Removed buffer from partition {} -> {} buffers; free buffers = {}",
                                     partitionIndex, partition.size(), m_freeBuffers);
        return true;
    }
    return false;
}

uint32 CDBlock::PartitionManager::DeleteSectors(uint8 partitionIndex, uint16 sectorPos, uint16 sectorCount) {
    assert(partitionIndex < m_partitions.size());

    auto &partition = m_partitions[partitionIndex];
    const uint32 totalSectors = partition.size();
    uint16 start, end;
    if (sectorPos == 0xFFFF) {
        start = totalSectors - 1;
    } else {
        start = sectorPos;
    }
    if (sectorCount == 0xFFFF) {
        end = totalSectors - 1;
    } else {
        end = start - sectorCount + 1;
    }
    sectorCount = std::min<uint16>(sectorCount, partition.size());
    start = std::min<uint16>(start, sectorCount - 1);
    end = std::min<uint16>(end, sectorCount - 1);
    partition.erase(partition.begin() + start, partition.begin() + end + 1);
    m_freeBuffers += end - start + 1;
    devlog::trace<grp::part_mgr>("Removed {} buffers from partition {} -> {} buffers; free buffers = {}",
                                 end - start + 1, partitionIndex, partition.size(), m_freeBuffers);
    return end - start + 1;
}

void CDBlock::PartitionManager::Clear(uint8 partitionIndex) {
    assert(partitionIndex < m_partitions.size());
    auto &partition = m_partitions[partitionIndex];
    m_freeBuffers += partition.size();
    devlog::trace<grp::part_mgr>("Cleared all {} buffers from partition {}; free buffers = {}", partition.size(),
                                 partitionIndex, m_freeBuffers);
    partition.clear();
}

uint32 CDBlock::PartitionManager::CalculateSize(uint8 partitionIndex, uint32 start, uint32 end) const {
    assert(partitionIndex < m_partitions.size());
    auto &partition = m_partitions[partitionIndex];
    start = std::min<uint32>(start, partition.size() - 1);
    end = std::min<uint32>(end, partition.size() - 1);
    const uint32 size = std::accumulate(partition.begin() + start, partition.begin() + end + 1, 0u,
                                        [](const uint32 lhs, const Buffer &rhs) { return lhs + rhs.size; });
    devlog::trace<grp::part_mgr>("Calculated partition {} size from {} to {} = {} bytes", partitionIndex, start, end,
                                 size);
    return size;
}

void CDBlock::PartitionManager::SaveState(state::CDBlockState &state) const {
    size_t bufferIndex = 0;
    for (size_t i = 0; i < m_partitions.size(); i++) {
        for (const auto &buffer : m_partitions[i]) {
            state.buffers[bufferIndex].data = buffer.data;
            state.buffers[bufferIndex].size = buffer.size;
            state.buffers[bufferIndex].frameAddress = buffer.frameAddress;
            state.buffers[bufferIndex].fileNum = buffer.subheader.fileNum;
            state.buffers[bufferIndex].chanNum = buffer.subheader.chanNum;
            state.buffers[bufferIndex].submode = buffer.subheader.submode;
            state.buffers[bufferIndex].codingInfo = buffer.subheader.codingInfo;
            state.buffers[bufferIndex].partitionIndex = i;
            bufferIndex++;
        }
    }
}

bool CDBlock::PartitionManager::ValidateState(const state::CDBlockState &state) const {
    for (const auto &buffer : state.buffers) {
        if (buffer.partitionIndex >= kNumPartitions && buffer.partitionIndex != 0xFF) {
            return false;
        }
    }
    return true;
}

void CDBlock::PartitionManager::LoadState(const state::CDBlockState &state) {
    for (auto &partition : m_partitions) {
        partition.clear();
    }
    m_freeBuffers = kNumBuffers;

    for (const auto &buffer : state.buffers) {
        if (buffer.partitionIndex < kNumPartitions) {
            auto &partBuffer = m_partitions[buffer.partitionIndex].emplace_back();
            partBuffer.data = buffer.data;
            partBuffer.size = buffer.size;
            partBuffer.frameAddress = buffer.frameAddress;
            partBuffer.subheader.fileNum = buffer.fileNum;
            partBuffer.subheader.chanNum = buffer.chanNum;
            partBuffer.subheader.submode = buffer.submode;
            partBuffer.subheader.codingInfo = buffer.codingInfo;
            --m_freeBuffers;
        }
    }
}

} // namespace ymir::cdblock
