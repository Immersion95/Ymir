#pragma once

#include "cdblock_cmd_trace_window.hpp"

namespace app::ui {

struct CDBlockWindowSet {
    CDBlockWindowSet(SharedContext &context)
        : cmdTrace(context) {}

    void DisplayAll() {
        cmdTrace.Display();
    }

    CDBlockCommandTraceWindow cmdTrace;
};

} // namespace app::ui
