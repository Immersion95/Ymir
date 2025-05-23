#pragma once

#include "settings_view_base.hpp"

#include <app/ui/widgets/input_widgets.hpp>
#include <app/ui/widgets/unbound_actions_widget.hpp>

namespace app::ui {

class AnalogPadBindsView : public SettingsViewBase {
public:
    AnalogPadBindsView(SharedContext &context);

    void Display(Settings::Input::Port::AnalogPadBinds &binds, void *context);

private:
    widgets::InputCaptureWidget m_inputCaptureWidget;
    widgets::UnboundActionsWidget m_unboundActionsWidget;
};

} // namespace app::ui
