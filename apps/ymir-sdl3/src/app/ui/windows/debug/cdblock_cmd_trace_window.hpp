#pragma once

#include "cdblock_window_base.hpp"

#include <app/ui/views/debug/cdblock_cmd_trace_view.hpp>

namespace app::ui {

class CDBlockCommandTraceWindow : public CDBlockWindowBase {
public:
    CDBlockCommandTraceWindow(SharedContext &context);

protected:
    void PrepareWindow() override;
    void DrawContents() override;

private:
    CDBlockCommandTraceView m_cmdTraceView;
};

} // namespace app::ui
