#pragma once

#include "peripheral_base.hpp"

namespace ymir::peripheral {

class NullPeripheral final : public BasePeripheral {
public:
    NullPeripheral()
        : BasePeripheral(PeripheralType::None, 0x0, {}) {}

    bool IsConnected() const final {
        return false;
    }

    void UpdateInputs() final {}

    uint8 GetReportLength() const final {
        return 0;
    }

    void Read(std::span<uint8> out) final {}

    uint8 WritePDR(uint8 ddr, uint8 value) final {
        return 0;
    }
};

} // namespace ymir::peripheral
