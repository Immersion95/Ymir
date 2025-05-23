#pragma once

#include <ymir/debug/cdblock_tracer_base.hpp>

#include <util/ring_buffer.hpp>

#include <array>

namespace app {

struct CDBlockTracer final : ymir::debug::ICDBlockTracer {
    struct CommandInfo {
        uint32 index;
        std::array<uint16, 4> request;
        std::array<uint16, 4> response;
        bool processed;
    };

    void ClearCommands();

    bool traceCommands = false;

    util::RingBuffer<CommandInfo, 1024> commands;

private:
    uint32 m_commandCounter = 0;

    // -------------------------------------------------------------------------
    // ICDBlockTracer implementation

    void ProcessCommand(uint16 cr1, uint16 cr2, uint16 cr3, uint16 cr4) final;
    void ProcessCommandResponse(uint16 cr1, uint16 cr2, uint16 cr3, uint16 cr4) final;
};

} // namespace app
