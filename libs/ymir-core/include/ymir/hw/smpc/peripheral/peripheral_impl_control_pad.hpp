#pragma once

#include "peripheral_base.hpp"

#include "peripheral_state_common.hpp"

namespace ymir::peripheral {

/// @brief Implements the Saturn Control Pad (ID 0x0) with:
/// - 6 digital buttons: ABC XYZ
/// - 2 shoulder buttons: L R
/// - Directional pad
/// - Start button
class ControlPad final : public BasePeripheral {
public:
    ControlPad(CBPeripheralReport callback);

    void UpdateInputs() final;

    uint8 GetReportLength() const final;

    void Read(std::span<uint8> out) final;

    uint8 WritePDR(uint8 ddr, uint8 value) final;

private:
    ControlPadReport m_report;
};

} // namespace ymir::peripheral
