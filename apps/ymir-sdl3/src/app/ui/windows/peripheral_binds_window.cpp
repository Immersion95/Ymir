#include "peripheral_binds_window.hpp"

using namespace ymir;

namespace app::ui {

PeripheralBindsWindow::PeripheralBindsWindow(SharedContext &context)
    : WindowBase(context)
    , m_controlPadView(context)
    , m_analogPadView(context) {}

void PeripheralBindsWindow::Open(uint32 portIndex, uint32 slotIndex) {
    m_portIndex = std::min(portIndex, 1u);
    m_slotIndex = std::min(slotIndex, 0u); // TODO: support multi-tap
    WindowBase::Open = true;
}

void PeripheralBindsWindow::PrepareWindow() {
    m_windowConfig.name = fmt::format("Port {} input binds###periph_binds_window", m_portIndex + 1);

    // TODO: include slot index in title when multi-tap is supported
    // m_windowConfig.name =
    //     fmt::format("Port {} slot {} input binds###periph_binds_window", m_portIndex + 1, m_slotIndex + 1);

    ImGui::SetNextWindowSizeConstraints(ImVec2(500 * m_context.displayScale, 300 * m_context.displayScale),
                                        ImVec2(FLT_MAX, FLT_MAX));
}

void PeripheralBindsWindow::DrawContents() {
    auto &port =
        m_portIndex == 0 ? m_context.saturn.SMPC.GetPeripheralPort1() : m_context.saturn.SMPC.GetPeripheralPort2();
    auto &periph = port.GetPeripheral(/* TODO: m_slotIndex */);

    auto &settings = m_portIndex == 0 ? m_context.settings.input.port1 : m_context.settings.input.port2;

    switch (periph.GetType()) {
    case peripheral::PeripheralType::None: break;
    case peripheral::PeripheralType::ControlPad:
        m_controlPadView.Display(settings.controlPadBinds, &m_context.controlPadInputs[m_portIndex]);
        break;
    case peripheral::PeripheralType::AnalogPad:
        m_analogPadView.Display(settings.analogPadBinds, &m_context.analogPadInputs[m_portIndex]);
        break;
    }
}

} // namespace app::ui
