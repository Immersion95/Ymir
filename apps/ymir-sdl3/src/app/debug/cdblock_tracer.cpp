#include "cdblock_tracer.hpp"

namespace app {

void CDBlockTracer::ClearCommands() {
    commands.Clear();
    m_commandCounter = 0;
}

void CDBlockTracer::ProcessCommand(uint16 cr1, uint16 cr2, uint16 cr3, uint16 cr4) {
    if (!traceCommands) {
        return;
    }

    commands.Write({.index = m_commandCounter++, .request = {cr1, cr2, cr3, cr4}, .processed = false});
}

void CDBlockTracer::ProcessCommandResponse(uint16 cr1, uint16 cr2, uint16 cr3, uint16 cr4) {
    if (!traceCommands) {
        return;
    }

    auto &cmd = commands.GetLast();
    cmd.response[0] = cr1;
    cmd.response[1] = cr2;
    cmd.response[2] = cr3;
    cmd.response[3] = cr4;
    cmd.processed = true;
}

} // namespace app
