#include "disassembler_m68k.hpp"

#include "fetcher.hpp"

#include <ymir/hw/m68k/m68k_disasm.hpp>

#include <ymir/util/bit_ops.hpp>

struct M68KDisassembler {
    M68KDisassembler(Disassembler &disasm)
        : disasm(disasm) {}

    M68KDisassembler &Address(uint32 address) {
        disasm.Address(address);
        return *this;
    }

    M68KDisassembler &Opcodes(std::span<const uint16> opcodes) {
        disasm.Opcodes<5>(opcodes);
        return *this;
    }

    M68KDisassembler &Mnemonic(std::string_view mnemonic) {
        disasm.Mnemonic(mnemonic);
        return *this;
    }

    M68KDisassembler &Cond(ymir::m68k::Condition cond) {
        using enum ymir::m68k::Condition;

        switch (cond) {
        case T: disasm.Cond("t"); break;
        case F: disasm.Cond("f"); break;
        case HI: disasm.Cond("hi"); break;
        case LS: disasm.Cond("ls"); break;
        case CC: disasm.Cond("cc"); break;
        case CS: disasm.Cond("cs"); break;
        case NE: disasm.Cond("ne"); break;
        case EQ: disasm.Cond("eq"); break;
        case VC: disasm.Cond("vc"); break;
        case VS: disasm.Cond("vs"); break;
        case PL: disasm.Cond("pl"); break;
        case MI: disasm.Cond("mi"); break;
        case GE: disasm.Cond("ge"); break;
        case LT: disasm.Cond("lt"); break;
        case GT: disasm.Cond("gt"); break;
        case LE: disasm.Cond("le"); break;
        }

        return *this;
    }

    M68KDisassembler &Instruction(const ymir::m68k::DisassembledInstruction &instr) {
        {
            using enum ymir::m68k::Mnemonic;

            switch (instr.info.mnemonic) {
            case Move: Mnemonic("move"); break;
            case MoveA: Mnemonic("movea"); break;
            case MoveM: Mnemonic("movem"); break;
            case MoveP: Mnemonic("movep"); break;
            case MoveQ: Mnemonic("moveq"); break;
            case Clr: Mnemonic("clr"); break;
            case Exg: Mnemonic("exg"); break;
            case Ext: Mnemonic("ext"); break;
            case Swap: Mnemonic("swap"); break;
            case ABCD: Mnemonic("abcd"); break;
            case NBCD: Mnemonic("nbcd"); break;
            case SBCD: Mnemonic("sbcd"); break;
            case Add: Mnemonic("add"); break;
            case AddA: Mnemonic("adda"); break;
            case AddI: Mnemonic("addi"); break;
            case AddQ: Mnemonic("addq"); break;
            case AddX: Mnemonic("addx"); break;
            case And: Mnemonic("and"); break;
            case AndI: Mnemonic("andi"); break;
            case Eor: Mnemonic("eor"); break;
            case EorI: Mnemonic("eori"); break;
            case Neg: Mnemonic("neg"); break;
            case NegX: Mnemonic("negx"); break;
            case Not: Mnemonic("not"); break;
            case Or: Mnemonic("or"); break;
            case OrI: Mnemonic("ori"); break;
            case Sub: Mnemonic("sub"); break;
            case SubA: Mnemonic("suba"); break;
            case SubI: Mnemonic("subi"); break;
            case SubQ: Mnemonic("subq"); break;
            case SubX: Mnemonic("subx"); break;
            case DivS: Mnemonic("divs"); break;
            case DivU: Mnemonic("divu"); break;
            case MulS: Mnemonic("muls"); break;
            case MulU: Mnemonic("mulu"); break;
            case BChg: Mnemonic("bchg"); break;
            case BClr: Mnemonic("bclr"); break;
            case BSet: Mnemonic("bset"); break;
            case BTst: Mnemonic("btst"); break;
            case ASL: Mnemonic("asl"); break;
            case ASR: Mnemonic("asr"); break;
            case LSL: Mnemonic("lsl"); break;
            case LSR: Mnemonic("lsr"); break;
            case ROL: Mnemonic("rol"); break;
            case ROR: Mnemonic("ror"); break;
            case ROXL: Mnemonic("roxl"); break;
            case ROXR: Mnemonic("roxr"); break;
            case Cmp: Mnemonic("cmp"); break;
            case CmpA: Mnemonic("cmpa"); break;
            case CmpI: Mnemonic("cmpi"); break;
            case CmpM: Mnemonic("cmpm"); break;
            case Scc: Mnemonic("s").Cond(instr.info.cond); break;
            case TAS: Mnemonic("tas"); break;
            case Tst: Mnemonic("tst"); break;
            case LEA: Mnemonic("lea"); break;
            case PEA: Mnemonic("pea"); break;
            case Link: Mnemonic("link"); break;
            case Unlink: Mnemonic("unlk"); break;
            case BRA: Mnemonic("bra"); break;
            case BSR: Mnemonic("bsr"); break;
            case Bcc: Mnemonic("b").Cond(instr.info.cond); break;
            case DBcc: Mnemonic("db").Cond(instr.info.cond); break;
            case JSR: Mnemonic("jsr"); break;
            case Jmp: Mnemonic("jmp"); break;
            case RTE: Mnemonic("rte"); break;
            case RTR: Mnemonic("rtr"); break;
            case RTS: Mnemonic("rts"); break;
            case Chk: Mnemonic("chk"); break;
            case Reset: Mnemonic("reset"); break;
            case Stop: Mnemonic("stop"); break;
            case Trap: Mnemonic("trap"); break;
            case TrapV: Mnemonic("trapv"); break;
            case Noop: Mnemonic("nop"); break;

            case Illegal1010: disasm.IllegalMnemonic("(illegal 1010)"); break;
            case Illegal1111: disasm.IllegalMnemonic("(illegal 1111)"); break;
            case Illegal: disasm.IllegalMnemonic(); break;
            }
        }

        {
            using enum ymir::m68k::OperandSize;

            switch (instr.info.opSize) {
            case Byte: disasm.SizeSuffix("b"); break;
            case Word: disasm.SizeSuffix("w"); break;
            case Long: disasm.SizeSuffix("l"); break;
            default: break;
            }
        }

        return *this;
    }

    template <std::integral T>
    M68KDisassembler &ImmHex(T imm, bool hashPrefix) {
        disasm.ImmHex(imm, (hashPrefix ? "#" : ""), "$");
        return *this;
    }

    M68KDisassembler &RegSublist(uint16 regList, bool read, bool write, char regPrefix, bool &printedOnce) {
        uint16 bits = bit::extract<0, 7>(regList);
        uint16 pos = 0;
        uint16 numOnes = std::countr_one(bits);
        while (bits != 0) {
            if (numOnes >= 1) {
                if (printedOnce) {
                    disasm.Operator("/");
                }
                if (numOnes == 1) {
                    disasm.Operand(fmt::format("{}{}", regPrefix, pos), read, write);
                } else {
                    disasm.Operand(fmt::format("{0}{1}-{0}{2}", regPrefix, pos, pos + numOnes - 1), read, write);
                }
                printedOnce = true;
            }

            bits >>= numOnes;
            const uint16 numZeros = std::countr_zero(bits);
            bits >>= numZeros;
            pos += numOnes + numZeros;
            numOnes = std::countr_one(bits);
        }

        return *this;
    }

    M68KDisassembler &RegList(uint16 regList, bool read, bool write) {
        bool printedOnce = false;
        RegSublist(bit::extract<0, 7>(regList), read, write, 'd', printedOnce);
        RegSublist(bit::extract<8, 15>(regList), read, write, 'a', printedOnce);

        return *this;
    }

    M68KDisassembler &Operand(const ymir::m68k::Operand &op, const ymir::m68k::OperandDetails &det) {
        using OpType = ymir::m68k::Operand::Type;

        switch (op.type) {
        case OpType::None: break;
        case OpType::Dn: disasm.Operand(fmt::format("d{}", op.rn), op.read, op.write); break;
        case OpType::An: disasm.Operand(fmt::format("a{}", op.rn), op.read, op.write); break;
        case OpType::AtAn:
            disasm.Operand("(", op.read, op.write)
                .OperandRead(fmt::format("a{}", op.rn))
                .Operand(")", op.read, op.write);
            break;
        case OpType::AtAnPlus:
            disasm.Operand("(", op.read, op.write)
                .OperandReadWrite(fmt::format("a{}", op.rn))
                .Operand(")", op.read, op.write)
                .AddrInc();
            break;
        case OpType::MinusAtAn:
            disasm.AddrDec()
                .Operand("(", op.read, op.write)
                .OperandReadWrite(fmt::format("a{}", op.rn))
                .Operand(")", op.read, op.write);
            break;
        case OpType::AtDispAn:
            ImmHex<sint16>(det.immDisp, false);
            disasm.Operand("(", op.read, op.write)
                .OperandRead(fmt::format("a{}", op.rn))
                .Operand(")", op.read, op.write);
            break;
        case OpType::AtDispAnIx:
            ImmHex<sint16>(det.immDisp, false);
            disasm.Operand("(", op.read, op.write)
                .OperandRead(fmt::format("a{}", op.rn))
                .Comma()
                .OperandRead(fmt::format("{}{}", (det.ix >= 8 ? 'a' : 'd'), det.ix & 7))
                .SizeSuffix(det.ixLong ? "l" : "w")
                .Operand(")", op.read, op.write);
            break;
        case OpType::AtDispPC:
            ImmHex<uint32>(det.immDisp + address, false);
            disasm.Operand("(", op.read, op.write).OperandRead("pc").Operand(")", op.read, op.write);
            break;
        case OpType::AtDispPCIx:
            ImmHex<uint32>(det.immDisp + address, false);
            disasm.Operand("(", op.read, op.write)
                .OperandRead("pc")
                .Comma()
                .OperandRead(fmt::format("{}{}", (det.ix >= 8 ? 'a' : 'd'), det.ix & 7))
                .SizeSuffix(det.ixLong ? "l" : "w")
                .Operand(")", op.read, op.write);
            break;
        case OpType::AtImmWord:
            ImmHex<uint16>(det.immDisp, false);
            disasm.SizeSuffix("w");
            break;
        case OpType::AtImmLong:
            ImmHex<uint32>(det.immDisp, false);
            disasm.SizeSuffix("l");
            break;
        case OpType::UImmEmbedded: ImmHex<uint32>(op.simm, true); break;
        case OpType::UImm8Fetched: ImmHex<uint8>(det.immDisp, true); break;
        case OpType::UImm16Fetched: ImmHex<uint32>(det.immDisp, true); break;
        case OpType::UImm32Fetched: ImmHex<uint32>(det.immDisp, true); break;

        case OpType::WordDispPCEmbedded: ImmHex<uint32>(op.simm + address, false); break;
        case OpType::WordDispPCFetched: ImmHex<uint32>(det.immDisp + address, false); break;

        case OpType::CCR: disasm.Operand("ccr", op.read, op.write); break;
        case OpType::SR: disasm.Operand("sr", op.read, op.write); break;
        case OpType::USP: disasm.Operand("usp", op.read, op.write); break;

        case OpType::RegList: RegList(det.regList, op.read, op.write); break;
        case OpType::RevRegList: RegList(det.regList, op.read, op.write); break;
        }

        return *this;
    }

    M68KDisassembler &Operand1(const ymir::m68k::DisassembledInstruction &instr) {
        Operand(instr.info.op1, instr.op1);
        return *this;
    }

    M68KDisassembler &Operand2(const ymir::m68k::DisassembledInstruction &instr) {
        Operand(instr.info.op2, instr.op2);
        return *this;
    }

    M68KDisassembler &Disassemble(auto fetcher) {
        uint32 baseAddress = address;

        const ymir::m68k::DisassembledInstruction instr = ymir::m68k::Disassemble(fetcher);

        if (!valid) {
            return *this;
        }

        address += instr.opcodes.size() * sizeof(uint16);

        Address(baseAddress).Opcodes(instr.opcodes).Instruction(instr);
        disasm.Align(9);
        Operand1(instr);
        if (instr.info.op1.type != ymir::m68k::Operand::Type::None &&
            instr.info.op2.type != ymir::m68k::Operand::Type::None) {
            disasm.Comma();
        }
        Operand2(instr);

        disasm.NewLine();

        return *this;
    };

    Disassembler &disasm;
    uint32 address = 0;
    bool valid = true;
};

using M68KOpcode = uint16;
using M68KOpcodeFetcher = IOpcodeFetcher<M68KOpcode>;
using CommandLineM68KOpcodeFetcher = CommandLineOpcodeFetcher<M68KOpcode>;
using StreamM68KOpcodeFetcher = StreamOpcodeFetcher<M68KOpcode>;

bool DisassembleM68K(Disassembler &disasm, std::string_view origin, const std::vector<std::string> &args,
                     const std::string &inputFile) {
    auto maybeAddress = ParseHex<uint32>(origin);
    if (!maybeAddress) {
        fmt::println("Invalid origin address: {}", origin);
        return false;
    }

    M68KDisassembler m68kDisasm{disasm};
    m68kDisasm.address = *maybeAddress;

    auto fetcher =
        MakeFetcher<M68KOpcodeFetcher, CommandLineM68KOpcodeFetcher, StreamM68KOpcodeFetcher>(args, inputFile);
    if (!fetcher) {
        return false;
    }

    while (m68kDisasm.valid) {
        m68kDisasm.Disassemble([&]() -> uint16 {
            if (!m68kDisasm.valid) {
                return 0;
            }

            auto result = fetcher->Fetch();
            if (std::holds_alternative<M68KOpcode>(result)) {
                return std::get<M68KOpcode>(result);
            } else {
                if (std::holds_alternative<OpcodeFetchError>(result)) {
                    auto &error = std::get<OpcodeFetchError>(result);
                    fmt::println("{}", error.message);
                }
                m68kDisasm.valid = false;
                return 0;
            }
        });
    }
    return true;
}
