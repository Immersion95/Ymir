#include <ymir/hw/scsp/scsp_slot.hpp>

#include <ymir/util/bit_ops.hpp>
#include <ymir/util/data_ops.hpp>

#include <algorithm>

namespace ymir::scsp {

Slot::Slot() {
    Reset();
}

void Slot::Reset() {
    startAddress = 0;
    loopStartAddress = 0;
    loopEndAddress = 0;
    pcm8Bit = false;
    keyOnBit = false;
    loopControl = LoopControl::Off;
    sampleXOR = 0x0000;
    soundSource = SoundSource::SoundRAM;

    attackRate = 0;
    decay1Rate = 0;
    decay2Rate = 0;
    releaseRate = 0;

    decayLevel = 0;

    keyRateScaling = 0;

    egHold = false;
    egBypass = false;

    loopStartLink = false;

    modLevel = 0;
    modXSelect = 0;
    modYSelect = 0;
    stackWriteInhibit = false;

    totalLevel = 0;
    soundDirect = false;

    octave = 0;
    freqNumSwitch = 0x400u;
    maskMode = false;

    lfoReset = false;
    lfofRaw = 0;
    lfoStepInterval = s_lfoStepTbl[0];
    ampLFOSens = 0;
    ampLFOWaveform = Waveform::Saw;
    pitchLFOWaveform = Waveform::Saw;

    inputMixingLevel = 0;
    inputSelect = 0;
    directSendLevel = 0;
    directPan = 0;
    effectSendLevel = 0;
    effectPan = 0;

    extraBits0C = 0;
    extraBits14 = 0;

    active = false;

    egState = EGState::Release;

    egLevel = 0x3FF;

    sampleCount = 0;
    currSample = 0;
    currPhase = 0;
    nextPhase = 0;
    modulation = 0;
    reverse = false;
    crossedLoopStart = false;

    lfoCycles = 0;
    lfoStep = 0;

    alfoOutput = 0;

    sample1 = 0;
    sample2 = 0;

    output = 0;
}

bool Slot::TriggerKey() {
    // Key ON only triggers when EG is in Release state
    // Key OFF only triggers when EG is in any other state
    const bool trigger = (egState == EGState::Release) == keyOnBit;
    if (trigger) {
        if (keyOnBit) {
            active = true;

            egState = EGState::Attack;

            if (keyRateScaling == 0xF) {
                egAttackBug = false;
                egLevel = 0x280;
            } else {
                CheckAttackBug();
                egLevel = egAttackBug ? 0x000 : 0x280;
            }

            sampleCount = 0;
            currSample = 0;
            currPhase = 0;
            nextPhase = 0;
            modulation = 0;
            reverse = false;
            crossedLoopStart = false;

            sample1 = 0;
            sample2 = 0;

            output = 0;
        } else {
            egState = EGState::Release;
        }
    }
    return trigger;
}

template <typename T>
T Slot::ReadReg(uint32 address) {
    static constexpr bool is16 = std::is_same_v<T, uint16>;
    auto shiftByte = [](uint16 value) {
        if constexpr (is16) {
            return value;
        } else {
            return value >> 8u;
        }
    };

    switch (address) {
    case 0x00: return shiftByte(ReadReg00<is16, true>());
    case 0x01: return ReadReg00<true, is16>();
    case 0x02: return shiftByte(ReadReg02());
    case 0x03: return ReadReg02();
    case 0x04: return shiftByte(ReadReg04());
    case 0x05: return ReadReg04();
    case 0x06: return shiftByte(ReadReg06());
    case 0x07: return ReadReg06();
    case 0x08: return shiftByte(ReadReg08<is16, true>());
    case 0x09: return ReadReg08<true, is16>();
    case 0x0A: return shiftByte(ReadReg0A<is16, true>());
    case 0x0B: return ReadReg0A<true, is16>();
    case 0x0C: return shiftByte(ReadReg0C<is16, true>());
    case 0x0D: return ReadReg0C<true, is16>();
    case 0x0E: return shiftByte(ReadReg0E<is16, true>());
    case 0x0F: return ReadReg0E<true, is16>();
    case 0x10: return shiftByte(ReadReg10<is16, true>());
    case 0x11: return ReadReg10<true, is16>();
    case 0x12: return shiftByte(ReadReg12<is16, true>());
    case 0x13: return ReadReg12<true, is16>();
    case 0x14: return shiftByte(ReadReg14<is16, true>());
    case 0x15: return ReadReg14<true, is16>();
    case 0x16: return shiftByte(ReadReg16<is16, true>());
    case 0x17: return ReadReg16<true, is16>();
    default: return 0;
    }
}

template uint8 Slot::ReadReg<uint8>(uint32 address);
template uint16 Slot::ReadReg<uint16>(uint32 address);

template <typename T>
void Slot::WriteReg(uint32 address, T value) {
    static constexpr bool is16 = std::is_same_v<T, uint16>;
    uint16 value16 = value;
    if constexpr (!is16) {
        if ((address & 1) == 0) {
            value16 <<= 8u;
        }
    }

    switch (address) {
    case 0x00: WriteReg00<is16, true>(value16); break;
    case 0x01: WriteReg00<true, is16>(value16); break;
    case 0x02: WriteReg02<is16, true>(value16); break;
    case 0x03: WriteReg02<true, is16>(value16); break;
    case 0x04: WriteReg04<is16, true>(value16); break;
    case 0x05: WriteReg04<true, is16>(value16); break;
    case 0x06: WriteReg06<is16, true>(value16); break;
    case 0x07: WriteReg06<true, is16>(value16); break;
    case 0x08: WriteReg08<is16, true>(value16); break;
    case 0x09: WriteReg08<true, is16>(value16); break;
    case 0x0A: WriteReg0A<is16, true>(value16); break;
    case 0x0B: WriteReg0A<true, is16>(value16); break;
    case 0x0C: WriteReg0C<is16, true>(value16); break;
    case 0x0D: WriteReg0C<true, is16>(value16); break;
    case 0x0E: WriteReg0E<is16, true>(value16); break;
    case 0x0F: WriteReg0E<true, is16>(value16); break;
    case 0x10: WriteReg10<is16, true>(value16); break;
    case 0x11: WriteReg10<true, is16>(value16); break;
    case 0x12: WriteReg12<is16, true>(value16); break;
    case 0x13: WriteReg12<true, is16>(value16); break;
    case 0x14: WriteReg14<is16, true>(value16); break;
    case 0x15: WriteReg14<true, is16>(value16); break;
    case 0x16: WriteReg16<is16, true>(value16); break;
    case 0x17: WriteReg16<true, is16>(value16); break;
    }
}

template void Slot::WriteReg<uint8>(uint32 address, uint8 value);
template void Slot::WriteReg<uint16>(uint32 address, uint16 value);

template <bool lowerByte, bool upperByte>
uint16 Slot::ReadReg00() const {
    uint16 value = 0;
    if constexpr (lowerByte) {
        bit::deposit_into<0, 3>(value, bit::extract<16, 19>(startAddress));
        bit::deposit_into<4>(value, pcm8Bit);
        bit::deposit_into<5, 6>(value, static_cast<uint16>(loopControl));
    }

    util::SplitReadWord<lowerByte, upperByte, 7, 8>(value, static_cast<uint16>(soundSource));

    if constexpr (upperByte) {
        bit::deposit_into<9, 10>(value, bit::extract<14, 15>(sampleXOR));
        bit::deposit_into<11>(value, keyOnBit);
    }
    return value;
}

template <bool lowerByte, bool upperByte>
void Slot::WriteReg00(uint16 value) {
    if constexpr (lowerByte) {
        bit::deposit_into<16, 19>(startAddress, bit::extract<0, 3>(value));
        pcm8Bit = bit::test<4>(value);
        loopControl = static_cast<LoopControl>(bit::extract<5, 6>(value));
    }

    auto soundSourceValue = static_cast<uint16>(soundSource);
    util::SplitWriteWord<lowerByte, upperByte, 7, 8>(soundSourceValue, value);
    soundSource = static_cast<SoundSource>(soundSourceValue);

    if constexpr (upperByte) {
        static constexpr uint16 kSampleXORTable[] = {0x0000, 0x7FFF, 0x8000, 0xFFFF};
        sampleXOR = kSampleXORTable[bit::extract<9, 10>(value)];
        keyOnBit = bit::test<11>(value);
        // NOTE: bit 12 is KYONEX, handled in SCSP::WriteReg
    }
}

uint16 Slot::ReadReg02() const {
    return bit::extract<0, 15>(startAddress);
}

template <bool lowerByte, bool upperByte>
void Slot::WriteReg02(uint16 value) {
    static constexpr uint32 lb = lowerByte ? 0 : 8;
    static constexpr uint32 ub = upperByte ? 15 : 7;
    bit::deposit_into<lb, ub>(startAddress, bit::extract<lb, ub>(value));
}

uint16 Slot::ReadReg04() const {
    return loopStartAddress;
}

template <bool lowerByte, bool upperByte>
void Slot::WriteReg04(uint16 value) {
    static constexpr uint32 lb = lowerByte ? 0 : 8;
    static constexpr uint32 ub = upperByte ? 15 : 7;
    bit::deposit_into<lb, ub>(loopStartAddress, bit::extract<lb, ub>(value));
}

uint16 Slot::ReadReg06() const {
    return loopEndAddress;
}

template <bool lowerByte, bool upperByte>
void Slot::WriteReg06(uint16 value) {
    static constexpr uint32 lb = lowerByte ? 0 : 8;
    static constexpr uint32 ub = upperByte ? 15 : 7;
    bit::deposit_into<lb, ub>(loopEndAddress, bit::extract<lb, ub>(value));
}

template <bool lowerByte, bool upperByte>
uint16 Slot::ReadReg08() const {
    uint16 value = 0;
    if constexpr (lowerByte) {
        bit::deposit_into<0, 4>(value, attackRate);
        bit::deposit_into<5>(value, egHold);
    }

    util::SplitReadWord<lowerByte, upperByte, 6, 10>(value, decay1Rate);

    if constexpr (upperByte) {
        bit::deposit_into<11, 15>(value, decay2Rate);
    }
    return value;
}

template <bool lowerByte, bool upperByte>
void Slot::WriteReg08(uint16 value) {
    if constexpr (lowerByte) {
        attackRate = bit::extract<0, 4>(value);
        egHold = bit::test<5>(value);
        CheckAttackBug();
    }

    util::SplitWriteWord<lowerByte, upperByte, 6, 10>(decay1Rate, value);

    if constexpr (upperByte) {
        decay2Rate = bit::extract<11, 15>(value);
    }
}

template <bool lowerByte, bool upperByte>
uint16 Slot::ReadReg0A() const {
    uint16 value = 0;
    if constexpr (lowerByte) {
        bit::deposit_into<0, 4>(value, releaseRate);
    }

    util::SplitReadWord<lowerByte, upperByte, 5, 9>(value, decayLevel);

    if constexpr (upperByte) {
        bit::deposit_into<10, 13>(value, keyRateScaling);
        bit::deposit_into<14>(value, loopStartLink);
        bit::deposit_into<15>(value, egBypass);
    }
    return value;
}

template <bool lowerByte, bool upperByte>
void Slot::WriteReg0A(uint16 value) {
    if constexpr (lowerByte) {
        releaseRate = bit::extract<0, 4>(value);
    }

    util::SplitWriteWord<lowerByte, upperByte, 5, 9>(decayLevel, value);

    if constexpr (upperByte) {
        keyRateScaling = bit::extract<10, 13>(value);
        loopStartLink = bit::test<14>(value);
        egBypass = bit::test<15>(value);
        CheckAttackBug();
    }
}

template <bool lowerByte, bool upperByte>
uint16 Slot::ReadReg0C() const {
    uint16 value = 0;
    if constexpr (lowerByte) {
        bit::deposit_into<0, 7>(value, totalLevel);
    }

    if constexpr (upperByte) {
        bit::deposit_into<8>(value, soundDirect);
        bit::deposit_into<9>(value, stackWriteInhibit);
        bit::deposit_into<10, 11>(value, bit::extract<10, 11>(extraBits0C));
    }
    return value;
}

template <bool lowerByte, bool upperByte>
void Slot::WriteReg0C(uint16 value) {
    if constexpr (lowerByte) {
        totalLevel = bit::extract<0, 7>(value);
    }

    if constexpr (upperByte) {
        soundDirect = bit::test<8>(value);
        stackWriteInhibit = bit::test<9>(value);
        bit::deposit_into<10, 11>(extraBits0C, bit::extract<10, 11>(value));
    }
}

template <bool lowerByte, bool upperByte>
uint16 Slot::ReadReg0E() const {
    uint16 value = 0;
    if constexpr (lowerByte) {
        bit::deposit_into<0, 5>(value, modYSelect);
    }

    util::SplitReadWord<lowerByte, upperByte, 6, 11>(value, modXSelect);

    if constexpr (upperByte) {
        bit::deposit_into<12, 15>(value, modLevel);
    }
    return value;
}

template <bool lowerByte, bool upperByte>
void Slot::WriteReg0E(uint16 value) {
    if constexpr (lowerByte) {
        modYSelect = bit::extract<0, 5>(value);
    }

    util::SplitWriteWord<lowerByte, upperByte, 6, 11>(modXSelect, value);

    if constexpr (upperByte) {
        modLevel = bit::extract<12, 15>(value);
    }
}

template <bool lowerByte, bool upperByte>
uint16 Slot::ReadReg10() const {
    uint16 value = 0;

    util::SplitReadWord<lowerByte, upperByte, 0, 10>(value, freqNumSwitch ^ 0x400u);

    if constexpr (upperByte) {
        bit::deposit_into<11, 14>(value, octave);
        bit::deposit_into<15>(value, maskMode);
    }
    return value;
}

template <bool lowerByte, bool upperByte>
void Slot::WriteReg10(uint16 value) {
    util::SplitWriteWord<lowerByte, upperByte, 0, 10>(freqNumSwitch, value);

    if constexpr (upperByte) {
        freqNumSwitch ^= 0x400u;
        octave = bit::extract<11, 14>(value);
        maskMode = bit::test<15>(value);
        CheckAttackBug();
    }
}

template <bool lowerByte, bool upperByte>
uint16 Slot::ReadReg12() const {
    uint16 value = 0;
    if constexpr (lowerByte) {
        bit::deposit_into<0, 2>(value, ampLFOSens);
        bit::deposit_into<3, 4>(value, static_cast<uint16>(ampLFOWaveform));
        bit::deposit_into<5, 7>(value, pitchLFOSens);
    }

    if constexpr (upperByte) {
        bit::deposit_into<8, 9>(value, static_cast<uint16>(pitchLFOWaveform));
        bit::deposit_into<10, 14>(value, lfofRaw);
        bit::deposit_into<15>(value, lfoReset);
    }
    return value;
}

template <bool lowerByte, bool upperByte>
void Slot::WriteReg12(uint16 value) {
    if constexpr (lowerByte) {
        ampLFOSens = bit::extract<0, 2>(value);
        ampLFOWaveform = static_cast<Waveform>(bit::extract<3, 4>(value));
        pitchLFOSens = bit::extract<5, 7>(value);
    }

    if constexpr (upperByte) {
        pitchLFOWaveform = static_cast<Waveform>(bit::extract<8, 9>(value));
        lfofRaw = bit::extract<10, 14>(value);
        lfoStepInterval = s_lfoStepTbl[lfofRaw];
        lfoReset = bit::test<15>(value);
    }
}

template <bool lowerByte, bool upperByte>
uint16 Slot::ReadReg14() const {
    uint16 value = 0;
    if constexpr (lowerByte) {
        bit::deposit_into<0, 2>(value, inputMixingLevel);
        bit::deposit_into<3, 6>(value, inputSelect);
        bit::deposit_into<7>(value, bit::extract<7>(extraBits14));
    }
    return value;
}

template <bool lowerByte, bool upperByte>
void Slot::WriteReg14(uint16 value) {
    if constexpr (lowerByte) {
        inputMixingLevel = bit::extract<0, 2>(value);
        inputSelect = bit::extract<3, 6>(value);
        bit::deposit_into<7>(extraBits14, bit::extract<7>(value));
    }
}

template <bool lowerByte, bool upperByte>
uint16 Slot::ReadReg16() const {
    uint16 value = 0;
    if constexpr (lowerByte) {
        bit::deposit_into<0, 4>(value, effectPan);
        bit::deposit_into<5, 7>(value, effectSendLevel);
    }
    if constexpr (upperByte) {
        bit::deposit_into<8, 12>(value, directPan);
        bit::deposit_into<13, 15>(value, directSendLevel);
    }
    return value;
}

template <bool lowerByte, bool upperByte>
void Slot::WriteReg16(uint16 value) {
    if constexpr (lowerByte) {
        effectPan = bit::extract<0, 4>(value);
        effectSendLevel = bit::extract<5, 7>(value);
    }
    if constexpr (upperByte) {
        directPan = bit::extract<8, 12>(value);
        directSendLevel = bit::extract<13, 15>(value);
    }
}

void Slot::SaveState(state::SCSPSlotState &state) const {
    state.SA = startAddress;
    state.LSA = loopStartAddress;
    state.LEA = loopEndAddress;
    state.PCM8B = pcm8Bit;
    state.KYONB = keyOnBit;
    state.SBCTL = sampleXOR;

    switch (loopControl) {
    default: [[fallthrough]];
    case LoopControl::Off: state.LPCTL = state::SCSPSlotState::LoopControl::Off; break;
    case LoopControl::Normal: state.LPCTL = state::SCSPSlotState::LoopControl::Normal; break;
    case LoopControl::Reverse: state.LPCTL = state::SCSPSlotState::LoopControl::Reverse; break;
    case LoopControl::Alternate: state.LPCTL = state::SCSPSlotState::LoopControl::Alternate; break;
    }

    switch (soundSource) {
    default: [[fallthrough]];
    case SoundSource::SoundRAM: state.SSCTL = state::SCSPSlotState::SoundSource::SoundRAM; break;
    case SoundSource::Noise: state.SSCTL = state::SCSPSlotState::SoundSource::Noise; break;
    case SoundSource::Silence: state.SSCTL = state::SCSPSlotState::SoundSource::Silence; break;
    case SoundSource::Unknown: state.SSCTL = state::SCSPSlotState::SoundSource::Unknown; break;
    }

    state.AR = attackRate;
    state.D1R = decay1Rate;
    state.D2R = decay2Rate;
    state.RR = releaseRate;
    state.DL = decayLevel;

    state.KRS = keyRateScaling;
    state.EGHOLD = egHold;
    state.LPSLNK = loopStartLink;
    state.EGBYPASS = egBypass;

    state.MDL = modLevel;
    state.MDXSL = modXSelect;
    state.MDYSL = modYSelect;
    state.STWINH = stackWriteInhibit;

    state.TL = totalLevel;
    state.SDIR = soundDirect;

    state.OCT = octave;
    state.FNS = freqNumSwitch;
    state.MM = maskMode;

    auto castWaveform = [](Waveform waveform) {
        switch (waveform) {
        default: [[fallthrough]];
        case Waveform::Saw: return state::SCSPSlotState::Waveform::Saw;
        case Waveform::Square: return state::SCSPSlotState::Waveform::Square;
        case Waveform::Triangle: return state::SCSPSlotState::Waveform::Triangle;
        case Waveform::Noise: return state::SCSPSlotState::Waveform::Noise;
        }
    };

    state.LFORE = lfoReset;
    state.LFOF = lfofRaw;
    state.ALFOS = ampLFOSens;
    state.PLFOS = pitchLFOSens;
    state.ALFOWS = castWaveform(ampLFOWaveform);
    state.PLFOWS = castWaveform(pitchLFOWaveform);

    state.IMXL = inputMixingLevel;
    state.ISEL = inputSelect;
    state.DISDL = directSendLevel;
    state.DIPAN = directPan;

    state.EFSDL = effectSendLevel;
    state.EFPAN = effectPan;

    state.extra0C = extraBits0C;
    state.extra14 = extraBits14;

    state.active = active;

    switch (egState) {
    default: [[fallthrough]];
    case EGState::Attack: state.egState = state::SCSPSlotState::EGState::Attack; break;
    case EGState::Decay1: state.egState = state::SCSPSlotState::EGState::Decay1; break;
    case EGState::Decay2: state.egState = state::SCSPSlotState::EGState::Decay2; break;
    case EGState::Release: state.egState = state::SCSPSlotState::EGState::Release; break;
    }

    state.egLevel = egLevel;

    state.sampleCount = sampleCount;
    state.currSample = currSample;
    state.currPhase = currPhase;
    state.nextPhase = nextPhase;
    state.modulation = modulation;
    state.reverse = reverse;
    state.crossedLoopStart = crossedLoopStart;

    state.lfoCycles = lfoCycles;
    state.lfoStep = lfoStep;

    state.alfoOutput = alfoOutput;

    state.sample1 = sample1;
    state.sample2 = sample2;
    state.output = output;

    state.finalLevel = finalLevel;
}

bool Slot::ValidateState(const state::SCSPSlotState &state) const {
    switch (state.LPCTL) {
    case state::SCSPSlotState::LoopControl::Off: break;
    case state::SCSPSlotState::LoopControl::Normal: break;
    case state::SCSPSlotState::LoopControl::Reverse: break;
    case state::SCSPSlotState::LoopControl::Alternate: break;
    default: return false;
    }

    switch (state.SSCTL) {
    case state::SCSPSlotState::SoundSource::SoundRAM: break;
    case state::SCSPSlotState::SoundSource::Noise: break;
    case state::SCSPSlotState::SoundSource::Silence: break;
    case state::SCSPSlotState::SoundSource::Unknown: break;
    default: return false;
    }

    auto checkWaveform = [](state::SCSPSlotState::Waveform waveform) {
        switch (waveform) {
        case state::SCSPSlotState::Waveform::Saw: return true;
        case state::SCSPSlotState::Waveform::Square: return true;
        case state::SCSPSlotState::Waveform::Triangle: return true;
        case state::SCSPSlotState::Waveform::Noise: return true;
        default: return false;
        }
    };
    if (!checkWaveform(state.ALFOWS)) {
        return false;
    }
    if (!checkWaveform(state.PLFOWS)) {
        return false;
    }

    switch (state.egState) {
    case state::SCSPSlotState::EGState::Attack: break;
    case state::SCSPSlotState::EGState::Decay1: break;
    case state::SCSPSlotState::EGState::Decay2: break;
    case state::SCSPSlotState::EGState::Release: break;
    default: return false;
    }

    return true;
}

void Slot::LoadState(const state::SCSPSlotState &state) {
    startAddress = state.SA & 0xFFFFF;
    loopStartAddress = state.LSA & 0xFFFF;
    loopEndAddress = state.LEA & 0xFFFF;
    pcm8Bit = state.PCM8B;
    keyOnBit = state.KYONB;
    sampleXOR = state.SBCTL;

    switch (state.LPCTL) {
    default: [[fallthrough]];
    case state::SCSPSlotState::LoopControl::Off: loopControl = LoopControl::Off; break;
    case state::SCSPSlotState::LoopControl::Normal: loopControl = LoopControl::Normal; break;
    case state::SCSPSlotState::LoopControl::Reverse: loopControl = LoopControl::Reverse; break;
    case state::SCSPSlotState::LoopControl::Alternate: loopControl = LoopControl::Alternate; break;
    }

    switch (state.SSCTL) {
    default: [[fallthrough]];
    case state::SCSPSlotState::SoundSource::SoundRAM: soundSource = SoundSource::SoundRAM; break;
    case state::SCSPSlotState::SoundSource::Noise: soundSource = SoundSource::Noise; break;
    case state::SCSPSlotState::SoundSource::Silence: soundSource = SoundSource::Silence; break;
    case state::SCSPSlotState::SoundSource::Unknown: soundSource = SoundSource::Unknown; break;
    }

    attackRate = state.AR & 0x1F;
    decay1Rate = state.D1R & 0x1F;
    decay2Rate = state.D2R & 0x1F;
    releaseRate = state.RR & 0x1F;
    decayLevel = state.DL & 0x1F;

    keyRateScaling = state.KRS & 0xF;
    egHold = state.EGHOLD;
    loopStartLink = state.LPSLNK;
    egBypass = state.EGBYPASS;

    modLevel = state.MDL & 0xF;
    modXSelect = state.MDXSL & 0x3F;
    modYSelect = state.MDYSL & 0x3F;
    stackWriteInhibit = state.STWINH;

    totalLevel = state.TL;
    soundDirect = state.SDIR;

    octave = state.OCT & 0xF;
    freqNumSwitch = state.FNS & 0x7FF;
    maskMode = state.MM;

    auto castWaveform = [](state::SCSPSlotState::Waveform waveform) {
        switch (waveform) {
        default: [[fallthrough]];
        case state::SCSPSlotState::Waveform::Saw: return Waveform::Saw;
        case state::SCSPSlotState::Waveform::Square: return Waveform::Square;
        case state::SCSPSlotState::Waveform::Triangle: return Waveform::Triangle;
        case state::SCSPSlotState::Waveform::Noise: return Waveform::Noise;
        }
    };

    lfoReset = state.LFORE;
    lfofRaw = state.LFOF & 0x1F;
    lfoStepInterval = s_lfoStepTbl[lfofRaw];
    ampLFOSens = state.ALFOS & 0x7;
    pitchLFOSens = state.PLFOS & 0x7;
    ampLFOWaveform = castWaveform(state.ALFOWS);
    pitchLFOWaveform = castWaveform(state.PLFOWS);

    inputMixingLevel = state.IMXL & 0x7;
    inputSelect = state.ISEL & 0xF;
    directSendLevel = state.DISDL & 0x7;
    directPan = state.DIPAN & 0x1F;

    effectSendLevel = state.EFSDL & 0x7;
    effectPan = state.EFPAN & 0x1F;

    extraBits0C = state.extra0C;
    extraBits14 = state.extra14;

    active = state.active;

    switch (state.egState) {
    default: [[fallthrough]];
    case state::SCSPSlotState::EGState::Attack: egState = EGState::Attack; break;
    case state::SCSPSlotState::EGState::Decay1: egState = EGState::Decay1; break;
    case state::SCSPSlotState::EGState::Decay2: egState = EGState::Decay2; break;
    case state::SCSPSlotState::EGState::Release: egState = EGState::Release; break;
    }

    egLevel = state.egLevel & 0x3FF;

    sampleCount = state.sampleCount;
    currSample = state.currSample;
    currPhase = state.currPhase;
    nextPhase = state.nextPhase;
    modulation = state.modulation;
    reverse = state.reverse;
    crossedLoopStart = state.crossedLoopStart;

    lfoCycles = state.lfoCycles;
    lfoStep = state.lfoStep;

    alfoOutput = state.alfoOutput;

    sample1 = state.sample1;
    sample2 = state.sample2;
    output = state.output;

    finalLevel = state.finalLevel;
}

uint32 Slot::CalcEffectiveRate(uint8 rate) const {
    uint32 effectiveRate = rate;
    if (keyRateScaling < 0xF) {
        const sint32 krs = std::clamp(keyRateScaling + static_cast<sint32>(octave ^ 8) - 8, 0x0, 0xF);
        effectiveRate += krs;
    }
    return std::min<uint32>(effectiveRate << 1, 0x3F);
}

void Slot::CheckAttackBug() {
    const sint8 oct = static_cast<sint8>(octave ^ 8) - 8;
    const uint8 krs = std::clamp<uint8>(keyRateScaling + oct, 0x0, 0xF);
    egAttackBug = (attackRate + krs) >= 0x20;
}

uint8 Slot::GetCurrentEGRate() const {
    switch (egState) {
    case EGState::Attack: return attackRate;
    case EGState::Decay1: return decay1Rate;
    case EGState::Decay2: return decay2Rate;
    case EGState::Release: return releaseRate;
    default: return releaseRate; // should not happen
    }
}

uint16 Slot::GetEGLevel() const {
    if (egBypass || (egState == EGState::Attack && egHold)) {
        return 0x000;
    } else {
        return egLevel;
    }
}

void Slot::IncrementLFO() {
    lfoCycles++;
    if (lfoCycles >= lfoStepInterval) {
        lfoCycles = 0;
        lfoStep++;
    }
    if (lfoReset) {
        lfoStep = 0;
    }
}

void Slot::IncrementPhase(sint32 pitchLFO) {
    if (!active) {
        currPhase = 0;
        return;
    }
    currPhase = nextPhase;
    // NOTE: freqNumSwitch already has ^ 0x400u
    const uint32 phaseInc = ((freqNumSwitch + pitchLFO) << (octave ^ 8u)) >> 4u;
    nextPhase = (nextPhase & 0x3FFF) + phaseInc;
}

void Slot::IncrementSampleCounter() {
    currSample += currPhase >> 14u;

    if (!crossedLoopStart) {
        const uint16 nextSample = currSample + 1;
        if (nextSample > loopStartAddress) {
            crossedLoopStart = true;
            if (loopControl == LoopControl::Reverse) {
                currSample -= loopStartAddress + loopEndAddress;
                reverse = true;
            }
        }
    } else {
        const uint16 nextSample = (reverse ? ~currSample : currSample) + 1;
        const uint16 loopPoint =
            (reverse && (loopControl == LoopControl::Reverse || loopControl == LoopControl::Alternate))
                ? loopStartAddress
                : loopEndAddress;
        const bool crossedLoop = nextSample > loopPoint;

        if (reverse != crossedLoop) {
            switch (loopControl) {
            case LoopControl::Off:
                active = false;
                reverse = false;
                crossedLoopStart = false;
                break;
            case LoopControl::Normal:
                if (reverse) {
                    currSample += loopEndAddress - loopStartAddress;
                } else {
                    currSample += loopStartAddress - loopEndAddress;
                }
                break;
            case LoopControl::Reverse: currSample += loopStartAddress - loopEndAddress; break;
            case LoopControl::Alternate:
                reverse ^= true;
                if (reverse) {
                    currSample -= loopEndAddress * 2u;
                } else {
                    currSample += loopStartAddress * 2u;
                }
                break;
            }
        }
    }
}

void Slot::IncrementEG(uint64 sampleCounter) {
    static constexpr uint32 kCounterShiftTable[] = {11, 11, 11, 11, // 0-3    (0x00-0x03)
                                                    10, 10, 10, 10, // 4-7    (0x04-0x07)
                                                    9,  9,  9,  9,  // 8-11   (0x08-0x0B)
                                                    8,  8,  8,  8,  // 12-15  (0x0C-0x0F)
                                                    7,  7,  7,  7,  // 16-19  (0x10-0x13)
                                                    6,  6,  6,  6,  // 20-23  (0x14-0x17)
                                                    5,  5,  5,  5,  // 24-27  (0x18-0x1B)
                                                    4,  4,  4,  4,  // 28-31  (0x1C-0x1F)
                                                    3,  3,  3,  3,  // 32-35  (0x20-0x23)
                                                    2,  2,  2,  2,  // 36-39  (0x24-0x27)
                                                    1,  1,  1,  1,  // 40-43  (0x28-0x2B)
                                                    0,  0,  0,  0,  // 44-47  (0x2C-0x2F)
                                                    0,  0,  0,  0,  // 48-51  (0x30-0x33)
                                                    0,  0,  0,  0,  // 52-55  (0x34-0x37)
                                                    0,  0,  0,  0,  // 56-59  (0x38-0x3B)
                                                    0,  0,  0,  0}; // 60-63  (0x3C-0x3F)

    static constexpr uint32 kIncrementTable[][8] = {
        {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, // 0-1    (0x00-0x01)
        {0, 1, 1, 1, 0, 1, 1, 1}, {0, 1, 1, 1, 1, 1, 1, 1}, // 2-3    (0x02-0x03)
        {0, 1, 0, 1, 0, 1, 0, 1}, {0, 1, 0, 1, 1, 1, 0, 1}, // 4-5    (0x04-0x05)
        {0, 1, 1, 1, 0, 1, 1, 1}, {0, 1, 1, 1, 1, 1, 1, 1}, // 6-7    (0x06-0x07)
        {0, 1, 0, 1, 0, 1, 0, 1}, {0, 1, 0, 1, 1, 1, 0, 1}, // 8-9    (0x08-0x09)
        {0, 1, 1, 1, 0, 1, 1, 1}, {0, 1, 1, 1, 1, 1, 1, 1}, // 10-11  (0x0A-0x0B)
        {0, 1, 0, 1, 0, 1, 0, 1}, {0, 1, 0, 1, 1, 1, 0, 1}, // 12-13  (0x0C-0x0D)
        {0, 1, 1, 1, 0, 1, 1, 1}, {0, 1, 1, 1, 1, 1, 1, 1}, // 14-15  (0x0E-0x0F)
        {0, 1, 0, 1, 0, 1, 0, 1}, {0, 1, 0, 1, 1, 1, 0, 1}, // 16-17  (0x10-0x11)
        {0, 1, 1, 1, 0, 1, 1, 1}, {0, 1, 1, 1, 1, 1, 1, 1}, // 18-19  (0x12-0x13)
        {0, 1, 0, 1, 0, 1, 0, 1}, {0, 1, 0, 1, 1, 1, 0, 1}, // 20-21  (0x14-0x15)
        {0, 1, 1, 1, 0, 1, 1, 1}, {0, 1, 1, 1, 1, 1, 1, 1}, // 22-23  (0x18-0x17)
        {0, 1, 0, 1, 0, 1, 0, 1}, {0, 1, 0, 1, 1, 1, 0, 1}, // 24-25  (0x18-0x19)
        {0, 1, 1, 1, 0, 1, 1, 1}, {0, 1, 1, 1, 1, 1, 1, 1}, // 26-27  (0x1A-0x1B)
        {0, 1, 0, 1, 0, 1, 0, 1}, {0, 1, 0, 1, 1, 1, 0, 1}, // 28-29  (0x1C-0x1D)
        {0, 1, 1, 1, 0, 1, 1, 1}, {0, 1, 1, 1, 1, 1, 1, 1}, // 30-31  (0x1E-0x1F)
        {0, 1, 0, 1, 0, 1, 0, 1}, {0, 1, 0, 1, 1, 1, 0, 1}, // 32-33  (0x20-0x21)
        {0, 1, 1, 1, 0, 1, 1, 1}, {0, 1, 1, 1, 1, 1, 1, 1}, // 34-35  (0x22-0x23)
        {0, 1, 0, 1, 0, 1, 0, 1}, {0, 1, 0, 1, 1, 1, 0, 1}, // 36-37  (0x24-0x25)
        {0, 1, 1, 1, 0, 1, 1, 1}, {0, 1, 1, 1, 1, 1, 1, 1}, // 38-39  (0x26-0x27)
        {0, 1, 0, 1, 0, 1, 0, 1}, {0, 1, 0, 1, 1, 1, 0, 1}, // 40-41  (0x28-0x29)
        {0, 1, 1, 1, 0, 1, 1, 1}, {0, 1, 1, 1, 1, 1, 1, 1}, // 42-43  (0x2A-0x2B)
        {0, 1, 0, 1, 0, 1, 0, 1}, {0, 1, 0, 1, 1, 1, 0, 1}, // 44-45  (0x2C-0x2D)
        {0, 1, 1, 1, 0, 1, 1, 1}, {0, 1, 1, 1, 1, 1, 1, 1}, // 46-47  (0x2E-0x2F)
        {1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 2, 1, 1, 1, 2}, // 48-49  (0x30-0x31)
        {2, 1, 2, 1, 2, 1, 2, 1}, {1, 2, 2, 2, 1, 2, 2, 2}, // 50-51  (0x32-0x33)
        {2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 4, 2, 2, 2, 4}, // 52-53  (0x34-0x35)
        {4, 2, 4, 2, 4, 2, 4, 2}, {2, 4, 4, 4, 2, 4, 4, 4}, // 54-55  (0x36-0x37)
        {4, 4, 4, 4, 4, 4, 4, 4}, {4, 4, 4, 8, 4, 4, 4, 8}, // 56-57  (0x38-0x39)
        {8, 4, 8, 4, 8, 4, 8, 4}, {4, 8, 8, 8, 4, 8, 8, 8}, // 58-59  (0x3A-0x3B)
        {8, 8, 8, 8, 8, 8, 8, 8}, {8, 8, 8, 8, 8, 8, 8, 8}, // 60-61  (0x3C-0x3D)
        {8, 8, 8, 8, 8, 8, 8, 8}, {8, 8, 8, 8, 8, 8, 8, 8}, // 62-63  (0x3E-0x3F)
    };

    const uint8 currRate = GetCurrentEGRate();
    const uint32 rate = CalcEffectiveRate(currRate);
    const uint32 shift = kCounterShiftTable[rate];
    const uint32 egCycle = sampleCounter >> 1;
    uint32 inc{};
    if ((sampleCounter & 1) == 1 || (egCycle & ((1 << shift) - 1)) != 0) {
        inc = 0;
    } else {
        inc = kIncrementTable[rate][(egCycle >> shift) & 7];
    }

    const uint32 currLevel = egLevel;

    switch (egState) {
    case EGState::Attack: //
        if (!egAttackBug && inc > 0 && egLevel > 0 && currRate > 0) {
            egLevel += static_cast<sint32>(~currLevel * inc) >> 4;
        }
        if (loopStartLink ? crossedLoopStart : currLevel == 0) {
            egState = EGState::Decay1;
        }
        break;
    case EGState::Decay1:
        if ((egLevel >> 5u) == decayLevel) {
            egState = EGState::Decay2;
        }
        [[fallthrough]];
    case EGState::Decay2: [[fallthrough]];
    case EGState::Release:
        if (currRate > 0) {
            egLevel = std::min<uint16>(egLevel + inc, 0x3FF);
        }
        break;
    }

    if (currLevel >= 0x3C0 && !egBypass) {
        active = false;
        reverse = false;
        crossedLoopStart = false;
    }
}

} // namespace ymir::scsp
