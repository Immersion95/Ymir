#include "cdblock_cmd_trace_window.hpp"

namespace app::ui {

CDBlockCommandTraceWindow::CDBlockCommandTraceWindow(SharedContext &context)
    : CDBlockWindowBase(context)
    , m_cmdTraceView(context) {

    m_windowConfig.name = "CD Block command trace";
}

void CDBlockCommandTraceWindow::PrepareWindow() {
    ImGui::SetNextWindowSizeConstraints(ImVec2(450 * m_context.displayScale, 180 * m_context.displayScale),
                                        ImVec2(FLT_MAX, FLT_MAX));
}

void CDBlockCommandTraceWindow::DrawContents() {
    m_cmdTraceView.Display();
}

} // namespace app::ui
