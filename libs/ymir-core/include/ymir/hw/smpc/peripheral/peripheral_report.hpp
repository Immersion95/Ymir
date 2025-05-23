#pragma once

#include "peripheral_defs.hpp"
#include "peripheral_state_common.hpp"

namespace ymir::peripheral {

/// @brief Control Pad report data.
struct ControlPadReport {
    Button buttons; ///< Button states (1=released, 0=pressed).
};

/// @brief 3D Control Pad report data.
struct AnalogPadReport {
    Button buttons; ///< Button states (1=released, 0=pressed).
    bool analog;    ///< Whether to use analog mode (`true`) or digital mode (`false`).
    uint8 x;        ///< Analog stick X coordinate (`0x00`=left, `0x80`=center, `0xFF`=right).
    uint8 y;        ///< Analog stick Y coordinate (`0x00`=top, `0x80`=center, `0xFF`=bottom).
    uint8 l;        ///< Left analog trigger value (`0x00`=fully released, `0xFF`=fully pressed).
    uint8 r;        ///< Right analog trigger value (`0x00`=fully released, `0xFF`=fully pressed).
};

/// @brief A report to be filled when a peripheral is read.
struct PeripheralReport {
    /// @brief The peripheral type being read.
    PeripheralType type;

    /// @brief The peripheral data report.
    union Report {
        /// @brief Control Pad report data.
        ///
        /// Valid when `type` is `PeripheralType::ControlPad`.
        ControlPadReport controlPad;

        /// @brief 3D Control Pad report data.
        ///
        /// Valid when `type` is `PeripheralType::AnalogPad`.
        AnalogPadReport analogPad;
    } report;
};

} // namespace ymir::peripheral
