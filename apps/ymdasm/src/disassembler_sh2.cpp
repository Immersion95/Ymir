#include "disassembler_sh2.hpp"

#include "fetcher.hpp"

#include <ymir/hw/sh2/sh2_disasm.hpp>

struct SH2Disassembler {
    SH2Disassembler(Disassembler &disasm)
        : disasm(disasm) {}

    SH2Disassembler &Address() {
        disasm.Address(address);
        return *this;
    }

    SH2Disassembler &Opcode(uint16 opcode) {
        disasm.Opcode(opcode);
        return *this;
    }

    SH2Disassembler &DelaySlotPrefix() {
        disasm.Print(disasm.colors.delaySlot, "> ");
        return *this;
    }

    SH2Disassembler &Instruction(const ymir::sh2::DisassembledInstruction &instr, bool delaySlot) {
        if (delaySlot) {
            if (!instr.validInDelaySlot) {
                disasm.IllegalMnemonic();
                return *this;
            }
            DelaySlotPrefix();
        }

        {
            using enum ymir::sh2::Mnemonic;

            switch (instr.mnemonic) {
            case NOP: disasm.Mnemonic("nop"); break;
            case SLEEP: disasm.Mnemonic("sleep"); break;
            case MOV: disasm.Mnemonic("mov"); break;
            case MOVA: disasm.Mnemonic("mova"); break;
            case MOVT: disasm.Mnemonic("movt"); break;
            case CLRT: disasm.Mnemonic("clrt"); break;
            case SETT: disasm.Mnemonic("sett"); break;
            case EXTU: disasm.Mnemonic("extu"); break;
            case EXTS: disasm.Mnemonic("exts"); break;
            case SWAP: disasm.Mnemonic("swap"); break;
            case XTRCT: disasm.Mnemonic("xtrct"); break;
            case LDC: disasm.Mnemonic("ldc"); break;
            case LDS: disasm.Mnemonic("lds"); break;
            case STC: disasm.Mnemonic("stc"); break;
            case STS: disasm.Mnemonic("sts"); break;
            case ADD: disasm.Mnemonic("add"); break;
            case ADDC: disasm.Mnemonic("addc"); break;
            case ADDV: disasm.Mnemonic("addv"); break;
            case AND: disasm.Mnemonic("and"); break;
            case NEG: disasm.Mnemonic("neg"); break;
            case NEGC: disasm.Mnemonic("negc"); break;
            case NOT: disasm.Mnemonic("not"); break;
            case OR: disasm.Mnemonic("or"); break;
            case ROTCL: disasm.Mnemonic("rotcl"); break;
            case ROTCR: disasm.Mnemonic("rotcr"); break;
            case ROTL: disasm.Mnemonic("rotl"); break;
            case ROTR: disasm.Mnemonic("rotr"); break;
            case SHAL: disasm.Mnemonic("shal"); break;
            case SHAR: disasm.Mnemonic("shar"); break;
            case SHLL: disasm.Mnemonic("shll"); break;
            case SHLL2: disasm.Mnemonic("shll2"); break;
            case SHLL8: disasm.Mnemonic("shll8"); break;
            case SHLL16: disasm.Mnemonic("shll16"); break;
            case SHLR: disasm.Mnemonic("shlr"); break;
            case SHLR2: disasm.Mnemonic("shlr2"); break;
            case SHLR8: disasm.Mnemonic("shlr8"); break;
            case SHLR16: disasm.Mnemonic("shlr16"); break;
            case SUB: disasm.Mnemonic("sub"); break;
            case SUBC: disasm.Mnemonic("subc"); break;
            case SUBV: disasm.Mnemonic("subv"); break;
            case XOR: disasm.Mnemonic("xor"); break;
            case DT: disasm.Mnemonic("dt"); break;
            case CLRMAC: disasm.Mnemonic("clrmac"); break;
            case MAC: disasm.Mnemonic("mac"); break;
            case MUL: disasm.Mnemonic("mul"); break;
            case MULS: disasm.Mnemonic("muls"); break;
            case MULU: disasm.Mnemonic("mulu"); break;
            case DMULS: disasm.Mnemonic("dmuls"); break;
            case DMULU: disasm.Mnemonic("dmulu"); break;
            case DIV0S: disasm.Mnemonic("div0s"); break;
            case DIV0U: disasm.Mnemonic("div0u"); break;
            case DIV1: disasm.Mnemonic("div1"); break;
            case CMP_EQ: disasm.Mnemonic("cmp").Operator("/").Cond("eq"); break;
            case CMP_GE: disasm.Mnemonic("cmp").Operator("/").Cond("ge"); break;
            case CMP_GT: disasm.Mnemonic("cmp").Operator("/").Cond("gt"); break;
            case CMP_HI: disasm.Mnemonic("cmp").Operator("/").Cond("hi"); break;
            case CMP_HS: disasm.Mnemonic("cmp").Operator("/").Cond("hs"); break;
            case CMP_PL: disasm.Mnemonic("cmp").Operator("/").Cond("pl"); break;
            case CMP_PZ: disasm.Mnemonic("cmp").Operator("/").Cond("pz"); break;
            case CMP_STR: disasm.Mnemonic("cmp").Operator("/").Cond("str"); break;
            case TAS: disasm.Mnemonic("tas"); break;
            case TST: disasm.Mnemonic("tst"); break;
            case BF: disasm.Mnemonic("b").Cond("f"); break;
            case BFS: disasm.Mnemonic("b").Cond("f").Operator("/").Mnemonic("s"); break;
            case BT: disasm.Mnemonic("b").Cond("t"); break;
            case BTS: disasm.Mnemonic("b").Cond("t").Operator("/").Mnemonic("s"); break;
            case BRA: disasm.Mnemonic("bra"); break;
            case BRAF: disasm.Mnemonic("braf"); break;
            case BSR: disasm.Mnemonic("bsr"); break;
            case BSRF: disasm.Mnemonic("bsrf"); break;
            case JMP: disasm.Mnemonic("jmp"); break;
            case JSR: disasm.Mnemonic("jsr"); break;
            case TRAPA: disasm.Mnemonic("trapa"); break;
            case RTE: disasm.Mnemonic("rte"); break;
            case RTS: disasm.Mnemonic("rts"); break;
            case Illegal: disasm.IllegalMnemonic(); break;
            default: disasm.UnknownMnemonic(); break;
            }
        }

        {
            using enum ymir::sh2::OperandSize;

            switch (instr.opSize) {
            case Byte: disasm.SizeSuffix("b"); break;
            case Word: disasm.SizeSuffix("w"); break;
            case Long: disasm.SizeSuffix("l"); break;
            default: break;
            }
        }

        return *this;
    }

    SH2Disassembler &OperandRead(const char *op) {
        disasm.OperandRead(op);
        return *this;
    }

    SH2Disassembler &RnRead(uint8 rn) {
        disasm.OperandRead(fmt::format("r{}", rn));
        return *this;
    }

    SH2Disassembler &RnWrite(uint8 rn) {
        disasm.OperandWrite(fmt::format("r{}", rn));
        return *this;
    }

    SH2Disassembler &RnReadWrite(uint8 rn) {
        disasm.OperandReadWrite(fmt::format("r{}", rn));
        return *this;
    }

    SH2Disassembler &Rn(uint8 rn, bool read, bool write) {
        if (read && write) {
            RnReadWrite(rn);
        } else if (write) {
            RnWrite(rn);
        } else {
            RnRead(rn);
        }
        return *this;
    }

    SH2Disassembler &Imm(sint32 imm) {
        disasm.ImmHex(imm, "#");
        return *this;
    }

    SH2Disassembler &ReadWriteSymbol(const char *symbol, bool write) {
        disasm.ReadWriteSymbol(symbol, !write, write);
        return *this;
    }

    SH2Disassembler &AddrInc() {
        disasm.AddrInc();
        return *this;
    }

    SH2Disassembler &AddrDec() {
        disasm.AddrDec();
        return *this;
    }

    SH2Disassembler &Comma() {
        disasm.Comma();
        return *this;
    }

    SH2Disassembler &Operand(const ymir::sh2::Operand &op) {
        using OpType = ymir::sh2::Operand::Type;
        if (op.type == OpType::None) {
            return *this;
        }

        switch (op.type) {
        case OpType::Imm: Imm(op.immDisp); break;
        case OpType::Rn: Rn(op.reg, op.read, op.write); break;
        case OpType::AtRn: ReadWriteSymbol("@", op.write).RnRead(op.reg); break;
        case OpType::AtRnPlus: ReadWriteSymbol("@", op.write).RnReadWrite(op.reg).AddrInc(); break;
        case OpType::AtMinusRn: ReadWriteSymbol("@", op.write).AddrDec().RnReadWrite(op.reg); break;
        case OpType::AtDispRn:
            ReadWriteSymbol("@(", op.write).Imm(op.immDisp).Comma().RnRead(op.reg).ReadWriteSymbol(")", op.write);
            break;
        case OpType::AtR0Rn:
            ReadWriteSymbol("@(", op.write).RnRead(0).Comma().RnRead(op.reg).ReadWriteSymbol(")", op.write);
            break;
        case OpType::AtDispGBR:
            ReadWriteSymbol("@(", op.write).Imm(op.immDisp).Comma().OperandRead("gbr").ReadWriteSymbol(")", op.write);
            break;
        case OpType::AtR0GBR:
            ReadWriteSymbol("@(", op.write).RnRead(0).Comma().OperandRead("gbr").ReadWriteSymbol(")", op.write);
            break;
        case OpType::AtDispPC:
            ReadWriteSymbol("@(", false).Imm(address + op.immDisp).ReadWriteSymbol(")", false);
            break;
        case OpType::AtDispPCWordAlign:
            ReadWriteSymbol("@(", false).Imm((address & ~3) + op.immDisp).ReadWriteSymbol(")", false);
            break;
        case OpType::DispPC: Imm(address + op.immDisp); break;
        case OpType::RnPC: RnRead(op.reg); break;
        case OpType::SR: disasm.Operand("sr", op.read, op.write); break;
        case OpType::GBR: disasm.Operand("gbr", op.read, op.write); break;
        case OpType::VBR: disasm.Operand("vbr", op.read, op.write); break;
        case OpType::MACH: disasm.Operand("mach", op.read, op.write); break;
        case OpType::MACL: disasm.Operand("macl", op.read, op.write); break;
        case OpType::PR: disasm.Operand("pr", op.read, op.write); break;
        default: break;
        }

        return *this;
    }

    SH2Disassembler &Operand1(const ymir::sh2::DisassembledInstruction &instr) {
        Operand(instr.op1);
        return *this;
    }

    SH2Disassembler &Operand2(const ymir::sh2::DisassembledInstruction &instr) {
        Operand(instr.op2);
        return *this;
    }

    SH2Disassembler &Disassemble(uint16 opcode, int forceDelaySlot = 0) {
        using OpType = ymir::sh2::Operand::Type;

        const ymir::sh2::DisassembledInstruction &instr = ymir::sh2::Disassemble(opcode);

        bool delaySlotState = (isDelaySlot || forceDelaySlot > 0) && forceDelaySlot >= 0;

        Address().Opcode(opcode).Instruction(instr, delaySlotState);
        disasm.Align(12);
        Operand1(instr);
        if (instr.op1.type != OpType::None && instr.op2.type != OpType::None) {
            Comma();
        }
        Operand2(instr);

        disasm.Align(34);
        if (forceDelaySlot > 0) {
            disasm.Comment("; delay slot override");
        } else if (forceDelaySlot < 0) {
            disasm.Comment("; non-delay slot override");
        }

        disasm.NewLine();

        isDelaySlot = instr.hasDelaySlot;
        address += sizeof(opcode);
        return *this;
    }

    Disassembler &disasm;
    uint32 address = 0;
    bool isDelaySlot = false;
};

struct SH2Opcode {
    uint16 opcode;
    int forceDelaySlot = 0;
};

struct SH2CommandLineOpcodeParser {
    static OpcodeFetchResult<SH2Opcode> Parse(std::string_view arg) {
        std::string_view strippedOpcode = arg;

        // 0 = no change; +1 = force delay slot, -1 = force non-delay slot
        int forceDelaySlot = 0;
        if (arg.starts_with('_')) {
            forceDelaySlot = +1;
            strippedOpcode = strippedOpcode.substr(1);
        } else if (arg.starts_with('!')) {
            forceDelaySlot = -1;
            strippedOpcode = strippedOpcode.substr(1);
        }

        auto maybeOpcode = ParseHex<uint16>(strippedOpcode);
        if (!maybeOpcode) {
            return OpcodeFetchError{fmt::format("Invalid opcode: {}", arg)};
        }

        const uint16 opcode = *maybeOpcode;
        return SH2Opcode{opcode, forceDelaySlot};
    }
};

struct SH2StreamOpcodeParser {
    static SH2Opcode Parse(std::istream &input) {
        uint16 opcode{};
        input.read((char *)&opcode, sizeof(uint16));
        opcode = bit::big_endian_swap(opcode);
        return SH2Opcode{opcode, 0};
    }
};

using SH2OpcodeFetcher = IOpcodeFetcher<SH2Opcode>;
using CommandLineSH2OpcodeFetcher = CommandLineOpcodeFetcher<SH2Opcode, SH2CommandLineOpcodeParser>;
using StreamSH2OpcodeFetcher = StreamOpcodeFetcher<SH2Opcode, SH2StreamOpcodeParser>;

bool DisassembleSH2(Disassembler &disasm, std::string_view origin, const std::vector<std::string> &args,
                    const std::string &inputFile) {
    auto maybeAddress = ParseHex<uint32>(origin);
    if (!maybeAddress) {
        fmt::println("Invalid origin address: {}", origin);
        return false;
    }

    SH2Disassembler sh2Disasm{disasm};
    sh2Disasm.address = *maybeAddress;

    auto fetcher = MakeFetcher<SH2OpcodeFetcher, CommandLineSH2OpcodeFetcher, StreamSH2OpcodeFetcher>(args, inputFile);
    if (!fetcher) {
        return false;
    }

    bool running = true;
    const auto visitor = [&](auto &value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, SH2Opcode>) {
            sh2Disasm.Disassemble(value.opcode, value.forceDelaySlot);
        } else if constexpr (std::is_same_v<T, OpcodeFetchError>) {
            fmt::println("{}", value.message);
            running = false;
        } else if constexpr (std::is_same_v<T, OpcodeFetchEnd>) {
            running = false;
        }
    };

    while (running) {
        auto result = fetcher->Fetch();
        std::visit(visitor, result);
    }

    return true;
}
