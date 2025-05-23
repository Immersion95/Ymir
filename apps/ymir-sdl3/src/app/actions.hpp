#pragma once

#include <app/input/input_action.hpp>

namespace app::actions {

namespace general {

    inline constexpr auto OpenSettings = input::Action::Trigger(0x001000, "General", "Open settings");
    inline constexpr auto ToggleWindowedVideoOutput =
        input::Action::Trigger(0x001001, "General", "Toggle windowed video output");
    inline constexpr auto ToggleFullScreen = input::Action::Trigger(0x001002, "General", "Toggle full screen");

} // namespace general

namespace audio {

    inline constexpr auto ToggleMute = input::Action::Trigger(0x001100, "Audio", "Toggle mute");
    inline constexpr auto IncreaseVolume =
        input::Action::RepeatableTrigger(0x001101, "Audio", "Increase volume by 10%");
    inline constexpr auto DecreaseVolume =
        input::Action::RepeatableTrigger(0x001102, "Audio", "Decrease volume by 10%");

} // namespace audio

namespace cd_drive {

    inline constexpr auto LoadDisc = input::Action::Trigger(0x002000, "CD drive", "Load disc");
    inline constexpr auto EjectDisc = input::Action::Trigger(0x002001, "CD drive", "Eject disc");
    inline constexpr auto OpenCloseTray = input::Action::Trigger(0x002002, "CD drive", "Open/close tray");

} // namespace cd_drive

namespace save_states {

    inline constexpr auto QuickLoadState = input::Action::Trigger(0x003000, "Save states", "Quick load state");
    inline constexpr auto QuickSaveState = input::Action::Trigger(0x003001, "Save states", "Quick save state");

    inline constexpr auto SelectState1 = input::Action::Trigger(0x003011, "Save states", "Select state 1");
    inline constexpr auto SelectState2 = input::Action::Trigger(0x003012, "Save states", "Select state 2");
    inline constexpr auto SelectState3 = input::Action::Trigger(0x003013, "Save states", "Select state 3");
    inline constexpr auto SelectState4 = input::Action::Trigger(0x003014, "Save states", "Select state 4");
    inline constexpr auto SelectState5 = input::Action::Trigger(0x003015, "Save states", "Select state 5");
    inline constexpr auto SelectState6 = input::Action::Trigger(0x003016, "Save states", "Select state 6");
    inline constexpr auto SelectState7 = input::Action::Trigger(0x003017, "Save states", "Select state 7");
    inline constexpr auto SelectState8 = input::Action::Trigger(0x003018, "Save states", "Select state 8");
    inline constexpr auto SelectState9 = input::Action::Trigger(0x003019, "Save states", "Select state 9");
    inline constexpr auto SelectState10 = input::Action::Trigger(0x00301A, "Save states", "Select state 10");

    inline constexpr auto LoadState1 = input::Action::Trigger(0x003021, "Save states", "Load state 1");
    inline constexpr auto LoadState2 = input::Action::Trigger(0x003022, "Save states", "Load state 2");
    inline constexpr auto LoadState3 = input::Action::Trigger(0x003023, "Save states", "Load state 3");
    inline constexpr auto LoadState4 = input::Action::Trigger(0x003024, "Save states", "Load state 4");
    inline constexpr auto LoadState5 = input::Action::Trigger(0x003025, "Save states", "Load state 5");
    inline constexpr auto LoadState6 = input::Action::Trigger(0x003026, "Save states", "Load state 6");
    inline constexpr auto LoadState7 = input::Action::Trigger(0x003027, "Save states", "Load state 7");
    inline constexpr auto LoadState8 = input::Action::Trigger(0x003028, "Save states", "Load state 8");
    inline constexpr auto LoadState9 = input::Action::Trigger(0x003029, "Save states", "Load state 9");
    inline constexpr auto LoadState10 = input::Action::Trigger(0x00302A, "Save states", "Load state 10");

    inline constexpr auto SaveState1 = input::Action::Trigger(0x003031, "Save states", "Save state 1");
    inline constexpr auto SaveState2 = input::Action::Trigger(0x003032, "Save states", "Save state 2");
    inline constexpr auto SaveState3 = input::Action::Trigger(0x003033, "Save states", "Save state 3");
    inline constexpr auto SaveState4 = input::Action::Trigger(0x003034, "Save states", "Save state 4");
    inline constexpr auto SaveState5 = input::Action::Trigger(0x003035, "Save states", "Save state 5");
    inline constexpr auto SaveState6 = input::Action::Trigger(0x003036, "Save states", "Save state 6");
    inline constexpr auto SaveState7 = input::Action::Trigger(0x003037, "Save states", "Save state 7");
    inline constexpr auto SaveState8 = input::Action::Trigger(0x003038, "Save states", "Save state 8");
    inline constexpr auto SaveState9 = input::Action::Trigger(0x003039, "Save states", "Save state 9");
    inline constexpr auto SaveState10 = input::Action::Trigger(0x00303A, "Save states", "Save state 10");

} // namespace save_states

namespace sys {

    inline constexpr auto HardReset = input::Action::Trigger(0xE01000, "System", "Hard reset");
    inline constexpr auto SoftReset = input::Action::Trigger(0xE01001, "System", "Soft reset");
    inline constexpr auto ResetButton = input::Action::Button(0xE01002, "System", "Reset button");

} // namespace sys

namespace emu {

    inline constexpr auto TurboSpeed = input::Action::Button(0xE02000, "Emulation", "Turbo speed");
    // TODO: inline constexpr auto SlowMotion = input::Action::Button(0xE02001, "Emulation", "Slow motion");
    inline constexpr auto PauseResume = input::Action::Trigger(0xE02002, "Emulation", "Pause/resume");
    inline constexpr auto ForwardFrameStep =
        input::Action::RepeatableTrigger(0xE02003, "Emulation", "Forward frame step");
    inline constexpr auto ReverseFrameStep =
        input::Action::RepeatableTrigger(0xE02004, "Emulation", "Reverse frame step");
    inline constexpr auto Rewind = input::Action::Button(0xE02005, "Emulation", "Rewind");

    inline constexpr auto TurboSpeedHold = input::Action::Trigger(0xE020F0, "Emulation", "Turbo speed (hold)");

    inline constexpr auto ToggleRewindBuffer = input::Action::Trigger(0xE02100, "Emulation", "Toggle rewind buffer");

} // namespace emu

namespace dbg {

    inline constexpr auto ToggleDebugTrace = input::Action::Trigger(0xE03000, "Debugger", "Toggle tracing");
    inline constexpr auto DumpMemory = input::Action::Trigger(0xE03001, "Debugger", "Dump all memory");

} // namespace dbg

namespace control_pad {

    inline constexpr auto A = input::Action::Button(0xC01000, "Saturn Control Pad", "A");
    inline constexpr auto B = input::Action::Button(0xC01001, "Saturn Control Pad", "B");
    inline constexpr auto C = input::Action::Button(0xC01002, "Saturn Control Pad", "C");
    inline constexpr auto X = input::Action::Button(0xC01003, "Saturn Control Pad", "X");
    inline constexpr auto Y = input::Action::Button(0xC01004, "Saturn Control Pad", "Y");
    inline constexpr auto Z = input::Action::Button(0xC01005, "Saturn Control Pad", "Z");
    inline constexpr auto L = input::Action::Button(0xC01006, "Saturn Control Pad", "L");
    inline constexpr auto R = input::Action::Button(0xC01007, "Saturn Control Pad", "R");
    inline constexpr auto Start = input::Action::Button(0xC01008, "Saturn Control Pad", "Start");
    inline constexpr auto Up = input::Action::Button(0xC01009, "Saturn Control Pad", "Up");
    inline constexpr auto Down = input::Action::Button(0xC0100A, "Saturn Control Pad", "Down");
    inline constexpr auto Left = input::Action::Button(0xC0100B, "Saturn Control Pad", "Left");
    inline constexpr auto Right = input::Action::Button(0xC0100C, "Saturn Control Pad", "Right");
    inline constexpr auto DPad = input::Action::Axis2D(0xC0100D, "Saturn Control Pad", "D-Pad axis");

} // namespace control_pad

namespace analog_pad {

    inline constexpr auto A = input::Action::Button(0xC02000, "Saturn 3D Control Pad", "A");
    inline constexpr auto B = input::Action::Button(0xC02001, "Saturn 3D Control Pad", "B");
    inline constexpr auto C = input::Action::Button(0xC02002, "Saturn 3D Control Pad", "C");
    inline constexpr auto X = input::Action::Button(0xC02003, "Saturn 3D Control Pad", "X");
    inline constexpr auto Y = input::Action::Button(0xC02004, "Saturn 3D Control Pad", "Y");
    inline constexpr auto Z = input::Action::Button(0xC02005, "Saturn 3D Control Pad", "Z");
    inline constexpr auto L = input::Action::Button(0xC02006, "Saturn 3D Control Pad", "L");
    inline constexpr auto R = input::Action::Button(0xC02007, "Saturn 3D Control Pad", "R");
    inline constexpr auto Start = input::Action::Button(0xC02008, "Saturn 3D Control Pad", "Start");
    inline constexpr auto Up = input::Action::Button(0xC02009, "Saturn 3D Control Pad", "Up");
    inline constexpr auto Down = input::Action::Button(0xC0200A, "Saturn 3D Control Pad", "Down");
    inline constexpr auto Left = input::Action::Button(0xC0200B, "Saturn 3D Control Pad", "Left");
    inline constexpr auto Right = input::Action::Button(0xC0200C, "Saturn 3D Control Pad", "Right");
    inline constexpr auto DPad = input::Action::Axis2D(0xC0200D, "Saturn 3D Control Pad", "D-Pad axis");
    inline constexpr auto AnalogStick = input::Action::Axis2D(0xC0200E, "Saturn 3D Control Pad", "Analog stick");
    inline constexpr auto AnalogL = input::Action::Axis1D(0xC0200F, "Saturn 3D Control Pad", "Analog L");
    inline constexpr auto AnalogR = input::Action::Axis1D(0xC02010, "Saturn 3D Control Pad", "Analog R");
    inline constexpr auto SwitchMode = input::Action::Trigger(0xC02011, "Saturn 3D Control Pad", "Switch mode");

} // namespace analog_pad

} // namespace app::actions
