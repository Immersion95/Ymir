#pragma once

#include "peripheral_base.hpp"

#include "peripheral_state_common.hpp"

namespace ymir::peripheral {

/// @brief Implements the 3D Control Pad (ID 0x0 in digital mode, 0x1 in analog mode) with:
/// - 6 digital buttons: ABC XYZ
/// - 2 analog triggers: L R
/// - Directional pad
/// - Analog stick
/// - Start button
/// - Analog/digital mode toggle
///
/// In digital mode, the peripheral behaves exactly like a regular Control Pad, with L and R translated to digital
/// values based on the following thresholds:
/// - The button state is set to ON when the trigger value is 145 or higher
/// - The button state is set to OFF when the trigger value is 85 or lower
class AnalogPad final : public BasePeripheral {
public:
    AnalogPad(CBPeripheralReport callback);

    void UpdateInputs() final;

    uint8 GetReportLength() const final;

    void Read(std::span<uint8> out) final;

    uint8 WritePDR(uint8 ddr, uint8 value) final;

    void SetAnalogMode(bool mode);

private:
    bool m_analogMode = false;

    AnalogPadReport m_report;

    uint8 m_reportPos = 0;
    bool m_tl = false;
};

} // namespace ymir::peripheral
