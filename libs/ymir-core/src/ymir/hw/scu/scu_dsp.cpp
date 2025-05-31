#include <ymir/hw/scu/scu_dsp.hpp>

#include <ymir/hw/scu/scu_defs.hpp>

#include "scu_devlog.hpp"

namespace ymir::scu {

// -----------------------------------------------------------------------------
// Debugger

template <bool debug>
FORCE_INLINE static void TraceDSPDMA(debug::ISCUTracer *tracer, bool toD0, uint32 addrD0, uint8 addrDSP, uint8 count,
                                     uint8 addrInc, bool hold) {
    if constexpr (debug) {
        if (tracer) {
            return tracer->DSPDMA(toD0, addrD0, addrDSP, count, addrInc, hold);
        }
    }
}

// -----------------------------------------------------------------------------
// Implementation

SCUDSP::SCUDSP(sys::Bus &bus)
    : m_bus(bus) {
    Reset(true);
}

void SCUDSP::Reset(bool hard) {
    if (hard) {
        programRAM.fill(DSPInstr{});
        for (auto &bank : dataRAM) {
            bank.fill(0);
        }
    }

    programExecuting = false;
    programPaused = false;
    programEnded = false;
    programStep = false;

    PC = 0;
    dataAddress = 0;

    nextPC = ~0;
    jmpCounter = 0;

    sign = false;
    zero = false;
    carry = false;
    overflow = false;

    CT.fill(0);
    incCT.fill(false);

    ALU.u64 = 0;
    AC.u64 = 0;
    P.u64 = 0;
    RX = 0;
    RY = 0;

    loopTop = 0;
    loopCount = 0;

    dmaRun = false;
    dmaToD0 = false;
    dmaHold = false;
    dmaCount = 0;
    dmaSrc = 0;
    dmaDst = 0;
    dmaReadAddr = 0;
    dmaWriteAddr = 0;
    dmaAddrInc = 0;
}

template <bool debug>
void SCUDSP::Run(uint64 cycles) {
    // TODO: proper cycle counting

    RunDMA<debug>(cycles);

    for (uint64 cy = 0; cy < cycles; cy++) {
        // Bail out if not executing
        if (!programExecuting && !programStep) {
            return;
        }

        // Bail out if paused
        if (programPaused) {
            return;
        }

        // Execute next command
        const DSPInstr &instruction = programRAM[PC++];
        switch (instruction.instructionInfo.instructionClass) {
        case 0b00: Cmd_Operation<debug>(instruction); break;
        case 0b10: Cmd_LoadImm<debug>(instruction); break;
        case 0b11: Cmd_Special<debug>(instruction); break;
        }

        // Update PC
        if (jmpCounter > 0) {
            jmpCounter--;
            if (jmpCounter == 0) {
                PC = nextPC;
                nextPC = ~0;
            }
        }

        // Update CT0-3
        for (int i = 0; i < 4; i++) {
            if (incCT[i]) {
                CT[i]++;
                CT[i] &= 0x3F;
            }
        }
        incCT.fill(false);

        // Clear stepping flag to ensure the DSP only runs one command when stepping
        programStep = false;
    }
}

template void SCUDSP::Run<false>(uint64 cycles);
template void SCUDSP::Run<true>(uint64 cycles);

template <bool debug>
void SCUDSP::RunDMA(uint64 cycles) {
    // TODO: proper cycle counting

    // Bail out if DMA is not running
    if (!dmaRun) {
        return;
    }

    const bool toD0 = dmaToD0;
    uint32 addrD0 = toD0 ? dmaWriteAddr : dmaReadAddr;
    const BusID bus = GetBusID(addrD0);
    if (bus == BusID::None) {
        dmaRun = false;
        return;
    }

    if (dmaToD0) {
        devlog::trace<grp::dsp>("Running DSP DMA transfer: DSP -> {:08X} (+{:X}), {} longwords", addrD0, dmaAddrInc,
                                dmaCount);
    } else {
        devlog::trace<grp::dsp>("Running DSP DMA transfer: {:08X} -> DSP (+{:X}), {} longwords", addrD0, dmaAddrInc,
                                dmaCount);
    }

    // Run transfer
    // TODO: should iterate through transfers based on cycle count
    const uint32 ctIndex = toD0 ? dmaSrc : dmaDst;
    const bool useDataRAM = ctIndex <= 3;
    const bool useProgramRAM = !toD0 && ctIndex == 4;
    uint8 programRAMIndex = 0; // TODO: check if this is correct

    TraceDSPDMA<debug>(m_tracer, dmaToD0, addrD0, ctIndex, dmaCount, dmaAddrInc, dmaHold);

    do {
        dmaCount--;
        if (toD0) {
            // Data RAM -> D0
            const uint32 value = useDataRAM ? dataRAM[ctIndex][CT[ctIndex]] : ~0u;
            if (bus == BusID::ABus) {
                // A-Bus -> one 32-bit write
                m_bus.Write<uint32>(addrD0, value);
                addrD0 += dmaAddrInc;
            } else if (bus == BusID::BBus) {
                // B-Bus -> two 16-bit writes
                m_bus.Write<uint16>(addrD0, value >> 16u);
                addrD0 += dmaAddrInc;
                m_bus.Write<uint16>(addrD0, value >> 0u);
                addrD0 += dmaAddrInc;
            } else if (bus == BusID::WRAM) {
                // WRAM -> one 32-bit write
                m_bus.Write<uint32>(addrD0 & ~3, value);
                addrD0 += dmaAddrInc;
            }
        } else {
            // D0 -> Data/Program RAM
            uint32 value;
            if (bus == BusID::ABus) {
                // A-Bus -> one 32-bit read
                value = m_bus.Read<uint32>(addrD0);
                addrD0 += dmaAddrInc;
            } else if (bus == BusID::BBus) {
                // B-Bus -> two 16-bit reads
                value = m_bus.Read<uint16>(addrD0 | 0) << 16u;
                value |= m_bus.Read<uint16>(addrD0 | 2) << 0u;
                addrD0 += 4;
            } else if (bus == BusID::WRAM) {
                // WRAM -> one 32-bit read
                value = m_bus.Read<uint32>(addrD0);
                addrD0 += dmaAddrInc;
            }
            if (useDataRAM) {
                dataRAM[ctIndex][CT[ctIndex]] = value;
            } else if (useProgramRAM) {
                programRAM[programRAMIndex++].u32 = value;
            }
        }
        addrD0 &= 0x7FF'FFFF;
        if (useDataRAM) {
            CT[ctIndex]++;
            CT[ctIndex] &= 0x3F;
        }
    } while (dmaCount != 0);

    // Update RA0/WA0 if not holding address
    if (!dmaHold) {
        if (toD0) {
            dmaWriteAddr = (addrD0 + 2) & ~3;
        } else {
            dmaReadAddr = addrD0;
        }
    }

    dmaRun = false;
}

void SCUDSP::SaveState(state::SCUDSPState &state) const {
    for (size_t i = 0; i < programRAM.size(); ++i) {
        state.programRAM[i] = programRAM[i].u32;
    }
    state.dataRAM = dataRAM;
    state.programExecuting = programExecuting;
    state.programPaused = programPaused;
    state.programEnded = programEnded;
    state.programStep = programStep;
    state.PC = PC;
    state.dataAddress = dataAddress;
    state.nextPC = nextPC;
    state.jmpCounter = jmpCounter;
    state.sign = sign;
    state.zero = zero;
    state.carry = carry;
    state.overflow = overflow;
    state.CT = CT;
    state.ALU = ALU.s64;
    state.AC = AC.s64;
    state.P = P.s64;
    state.RX = RX;
    state.RY = RY;
    state.LOP = loopCount;
    state.TOP = loopTop;
    state.dmaRun = dmaRun;
    state.dmaToD0 = dmaToD0;
    state.dmaHold = dmaHold;
    state.dmaCount = dmaCount;
    state.dmaSrc = dmaSrc;
    state.dmaDst = dmaDst;
    state.dmaReadAddr = dmaReadAddr;
    state.dmaWriteAddr = dmaWriteAddr;
    state.dmaAddrInc = dmaAddrInc;
}

bool SCUDSP::ValidateState(const state::SCUDSPState &state) const {
    if (state.dmaAddrInc != 0 && (!bit::is_power_of_two(state.dmaAddrInc) || state.dmaAddrInc == 1)) {
        return false;
    }
    return true;
}

void SCUDSP::LoadState(const state::SCUDSPState &state) {
    for (size_t i = 0; i < programRAM.size(); ++i) {
        programRAM[i].u32 = state.programRAM[i];
    }
    dataRAM = state.dataRAM;
    programExecuting = state.programExecuting;
    programPaused = state.programPaused;
    programEnded = state.programEnded;
    programStep = state.programStep;
    PC = state.PC;
    dataAddress = state.dataAddress;
    nextPC = state.nextPC;
    jmpCounter = state.jmpCounter;
    sign = state.sign;
    zero = state.zero;
    carry = state.carry;
    overflow = state.overflow;
    CT = state.CT;
    ALU.s64 = state.ALU;
    AC.s64 = state.AC;
    P.s64 = state.P;
    RX = state.RX;
    RY = state.RY;
    loopCount = state.LOP & 0xFFF;
    loopTop = state.TOP;
    dmaRun = state.dmaRun;
    dmaToD0 = state.dmaToD0;
    dmaHold = state.dmaHold;
    dmaCount = state.dmaCount;
    dmaSrc = state.dmaSrc & 3;
    dmaDst = state.dmaDst & 7;
    dmaReadAddr = state.dmaReadAddr & 0x7FFFFFC;
    dmaWriteAddr = state.dmaWriteAddr & 0x7FFFFFC;
    dmaAddrInc = state.dmaAddrInc;
}

template <bool debug>
FORCE_INLINE void SCUDSP::Cmd_Operation(DSPInstr instr) {
    // ALU
    ALU = AC;
    switch (instr.aluInfo.aluOp) {
    case 0b0000: break;            // NOP
    case 0b0001: ALU_AND(); break; // AND
    case 0b0010: ALU_OR(); break;  // OR
    case 0b0011: ALU_XOR(); break; // XOR
    case 0b0100: ALU_ADD(); break; // ADD
    case 0b0101: ALU_SUB(); break; // SUB
    case 0b0110: ALU_AD2(); break; // AD2
    case 0b1000: ALU_SR(); break;  // SR
    case 0b1001: ALU_RR(); break;  // RR
    case 0b1010: ALU_SL(); break;  // SL
    case 0b1011: ALU_RL(); break;  // RL
    case 0b1111: ALU_RL8(); break; // RL8
    }

    // X-Bus
    //
    // X-Bus writes simultaneously to P and X in some cases:
    // bits
    // 25-23  executed operations
    //  000
    //  001
    //  010   MOV MUL,P
    //  011   MOV [s],P
    //  100               MOV [s],X
    //  101               MOV [s],X
    //  110   MOV MUL,P   MOV [s],X
    //  111   MOV [s],P   MOV [s],X
    if ((instr.aluInfo.xBusOp & 0b11) == 0b10) {
        // MOV MUL,P
        P.s64 = static_cast<sint64>(RX) * static_cast<sint64>(RY);
    }
    if (instr.aluInfo.xBusOp >= 0b011) {
        const sint32 value = ReadSource<debug>(instr.aluInfo.xBusSource);
        if ((instr.aluInfo.xBusOp & 0b11) == 0b11) {
            // MOV [s],P
            P.s64 = static_cast<sint64>(value);
        }
        if (bit::test<2>(instr.aluInfo.xBusOp)) {
            // MOV [s],X
            RX = value;
        }
    }

    // Y-Bus
    //
    // Y-Bus writes simultaneously to A and Y in some cases:
    // bits
    // 19-17  executed operations
    // 000
    // 001    CLR A
    // 010    MOV ALU,A
    // 011    MOV [s],A
    // 100                MOV [s],Y
    // 101    CLR A       MOV [s],Y
    // 110    MOV ALU,A   MOV [s],Y
    // 111    MOV [s],A   MOV [s],Y
    if ((instr.aluInfo.yBusOp & 0b11) == 0b01) {
        // CLR A
        AC.s64 = 0;
    } else if ((instr.aluInfo.yBusOp & 0b11) == 0b10) {
        // MOV ALU,A
        AC.s64 = ALU.s64;
    }
    if (instr.aluInfo.yBusOp >= 0b11) {
        const sint32 value = ReadSource<debug>(instr.aluInfo.yBusSource);
        if ((instr.aluInfo.yBusOp & 0b11) == 0b11) {
            // MOV [s],A
            AC.s64 = static_cast<sint64>(value);
        }
        if (bit::test<2>(instr.aluInfo.yBusOp)) {
            // MOV [s],Y
            RY = value;
        }
    }

    // D1-Bus
    switch (instr.aluInfo.d1BusOp) {
    case 0b01: // MOV SImm, [d]
    {
        const sint32 imm = instr.aluInfo.d1BusImm;
        const uint8 dst = instr.aluInfo.d1BusDest;
        WriteD1Bus<debug>(dst, imm);
        break;
    }
    case 0b11: // MOV [s], [d]
    {
        const uint8 src = instr.aluInfo.d1BusImm & 0b1111;
        const uint8 dst = instr.aluInfo.d1BusDest;
        const uint32 value = ReadSource<debug>(src);
        WriteD1Bus<debug>(dst, value);
        break;
    }
    }
}

template <bool debug>
FORCE_INLINE void SCUDSP::Cmd_LoadImm(DSPInstr instr) {
    const uint8 dst = instr.loadInfo.loadControl.storageLocation;
    sint32 imm;
    if (instr.loadInfo.loadControl.conditionalLoad) {
        // Conditional transfer
        // MVI SImm,[d],<cond>
        imm = instr.loadInfo.conditional.imm;

        const uint8 cond = instr.loadInfo.conditional.condition;
        if (!CondCheck(cond)) {
            return;
        }
    } else {
        // Unconditional transfer
        // MVI SImm,[d]
        imm = instr.loadInfo.unconditional.imm;
    }

    WriteImm<debug>(dst, imm);
}

template <bool debug>
FORCE_INLINE void SCUDSP::Cmd_Special(DSPInstr instr) {
    const uint32 cmdSubcategory = instr.specialInfo.specialControl.specialClass;
    switch (cmdSubcategory) {
    case 0b00: Cmd_Special_DMA<debug>(instr); break;
    case 0b01: Cmd_Special_Jump(instr); break;
    case 0b10: Cmd_Special_Loop(instr); break;
    case 0b11: Cmd_Special_End(instr); break;
    }
}

template <bool debug>
FORCE_INLINE void SCUDSP::Cmd_Special_DMA(DSPInstr command) {
    // Finish previous DMA transfer
    if (dmaRun) {
        RunDMA<debug>(1); // TODO: cycles?
    }

    dmaRun = true;
    dmaToD0 = command.specialInfo.dmaInfo.direction;
    dmaHold = command.specialInfo.dmaInfo.hold;

    // Get DMA transfer length
    if (command.specialInfo.dmaInfo.sizeSource) {
        const uint8 ctIndex = command.specialInfo.dmaInfo.imm & 0b11;
        const bool inc = bit::test<2>(command.specialInfo.dmaInfo.imm);
        const uint32 ctAddr = CT[ctIndex];
        dmaCount = dataRAM[ctIndex][ctAddr];
        if (inc) {
            CT[ctIndex]++;
            CT[ctIndex] &= 0x3F;
        }
    } else {
        dmaCount = command.specialInfo.dmaInfo.imm;
    }

    // Get [RAM] source/destination register (CT) index and address increment
    const uint8 addrInc = command.specialInfo.dmaInfo.stride;
    if (dmaToD0) {
        // DMA [RAM],D0,SImm
        // DMA [RAM],D0,[s]
        // DMAH [RAM],D0,SImm
        // DMAH [RAM],D0,[s]
        dmaSrc = command.specialInfo.dmaInfo.address;
        dmaAddrInc = (1u << addrInc) & ~1u;
    } else {
        // DMA D0,[RAM],SImm
        // DMA D0,[RAM],[s]
        // DMAH D0,[RAM],SImm
        // DMAH D0,[RAM],[s]
        dmaDst = command.specialInfo.dmaInfo.address;
        dmaAddrInc = (1u << (addrInc & 0x2u)) & ~1u;
    }

    devlog::trace<grp::dsp>("DSP DMA command: {:04X} @ {:02X}", command.u32, PC);
}

FORCE_INLINE void SCUDSP::Cmd_Special_Jump(DSPInstr command) {
    // JMP <cond>,SImm
    // JMP SImm
    const uint32 cond = command.specialInfo.jumpInfo.condition;
    if (cond != 0 && !CondCheck(cond)) {
        return;
    }

    const uint8 target = command.specialInfo.jumpInfo.target;
    DelayedJump(target);
}

FORCE_INLINE void SCUDSP::Cmd_Special_Loop(DSPInstr command) {
    if (loopCount != 0) {
        if (command.specialInfo.loopInfo.repeat) {
            // LPS
            DelayedJump(PC - 1);
        } else {
            // BTM
            DelayedJump(loopTop);
        }
    }
    loopCount--;
    loopCount &= 0xFFF;
}

FORCE_INLINE void SCUDSP::Cmd_Special_End(DSPInstr command) {
    // END
    // ENDI
    programExecuting = false;
    programEnded = true;
    if (command.specialInfo.endInfo.interrupt) {
        m_cbTriggerDSPEnd();
    }
}

} // namespace ymir::scu
