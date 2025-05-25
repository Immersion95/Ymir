#include "disassembler_scspdsp.hpp"

#include "fetcher.hpp"

#include <ymir/hw/scsp/scsp_dsp_instr.hpp>

struct SCSPDSPDisassembler {
    SCSPDSPDisassembler(Disassembler &disasm)
        : disasm(disasm) {}

    SCSPDSPDisassembler &Address() {
        disasm.Address(address);
        return *this;
    }

    SCSPDSPDisassembler &Opcode(uint64 opcode) {
        disasm.Opcode(opcode);
        return *this;
    }

    SCSPDSPDisassembler &InstructionRaw(ymir::scsp::DSPInstr instr) {
        if (instr.u64 == 0) {
            disasm.NOP("NOP");
            return *this;
        }

        disasm.Mnemonic("IRA").Operator("=");
        if (instr.IRA <= 0x1F) {
            disasm.OperandRead("MEMS").Operator("[").ImmHex<uint8, 2>(instr.IRA).Operator("]");
        } else if (instr.IRA <= 0x2F) {
            disasm.OperandRead("MIXS").Operator("[").ImmHex<uint8, 2>(instr.IRA & 0xF).Operator("]");
        } else if (instr.IRA <= 0x31) {
            disasm.OperandRead("EXTS").Operator("[").ImmHex<uint8, 2>(instr.IRA & 0x1).Operator("]");
        } else {
            disasm.IllegalMnemonic();
        }

        if (instr.IWT) {
            disasm.Align(15);
            disasm.Mnemonic("IWA").Operator("=").ImmHex<uint8, 2>(instr.IWA);
        }

        disasm.Align(24);
        disasm.Mnemonic("TRA").Operator("=").ImmHex<uint8, 2>(instr.TRA);

        if (instr.TWT) {
            disasm.Align(33);
            disasm.Mnemonic("TWA").Operator("=").ImmHex<uint8, 2>(instr.TWA);
        }

        disasm.Align(42);
        disasm.Mnemonic("XSEL").Operator("=");
        switch (instr.XSEL) {
        case 0: disasm.OperandRead("TEMP"); break;
        case 1: disasm.OperandRead("INPUTS"); break;
        }

        disasm.Align(54);
        disasm.Mnemonic("YSEL").Operator("=");
        switch (instr.YSEL) {
        case 0: disasm.OperandRead("FRC"); break;
        case 1: disasm.OperandRead("COEF").Operator("[").ImmHex<uint8, 2>(instr.CRA).Operator("]"); break;
        case 2: disasm.OperandRead("Y").BitRange(23, 11); break;
        case 3: disasm.OperandRead("Y").BitRange(15, 4); break;
        }

        if (instr.YRL) {
            disasm.Align(70);
            disasm.Mnemonic("YRL");
        }

        if (instr.FRCL) {
            disasm.Align(74);
            disasm.Mnemonic("FRCL");
        }

        disasm.Align(79);
        if (instr.ZERO) {
            disasm.Mnemonic("ZERO");
        } else {
            if (instr.NEGB) {
                disasm.Mnemonic("NEGB");
                if (instr.BSEL) {
                    disasm.Operator(" ");
                }
            }
            if (instr.BSEL) {
                disasm.Mnemonic("BSEL");
            }
        }

        if (instr.SHFT0) {
            disasm.Align(89);
            disasm.Mnemonic("SHFT0");
        }
        if (instr.SHFT1) {
            disasm.Align(95);
            disasm.Mnemonic("SHFT1");
        }

        if (instr.EWT) {
            disasm.Align(101);
            disasm.Mnemonic("EWA").Operator("=").ImmHex(instr.EWA);
        }

        if (instr.MRD || instr.MWT) {
            disasm.Align(109);
            disasm.Mnemonic("MASA").Operator("=").ImmHex<uint8, 2>(instr.MASA);
            if (instr.MRD) {
                disasm.Align(119);
                disasm.Mnemonic("MRD");
            }
            if (instr.MWT) {
                disasm.Align(123);
                disasm.Mnemonic("MWT");
            }
            if (instr.NXADR) {
                disasm.Align(127);
                disasm.Mnemonic("NXADR");
            }
            if (instr.ADREB) {
                disasm.Align(133);
                disasm.Mnemonic("ADREB");
            }
            if (instr.NOFL) {
                disasm.Align(139);
                disasm.Mnemonic("NOFL");
            }
            if (instr.TABLE) {
                disasm.Align(144);
                disasm.Mnemonic("TABLE");
            }
        }

        if (instr.ADRL) {
            disasm.Align(150);
            disasm.Mnemonic("ADRL");
        }

        return *this;
    }

    SCSPDSPDisassembler &Instruction(ymir::scsp::DSPInstr instr) {
        if (instr.u64 == 0) {
            disasm.NOP("NOP");
            return *this;
        }

        disasm.OperandWrite("INPUTS").Operator("<-");
        if (instr.IRA <= 0x1F) {
            disasm.OperandRead("MEMS").Operator("[").ImmHex<uint8, 2>(instr.IRA).Operator("]");
        } else if (instr.IRA <= 0x2F) {
            disasm.OperandRead("MIXS").Operator("[").ImmHex<uint8, 2>(instr.IRA & 0xF).Operator("]");
        } else if (instr.IRA <= 0x31) {
            disasm.OperandRead("EXTS").Operator("[").ImmHex<uint8, 2>(instr.IRA & 0x1).Operator("]");
        } else {
            disasm.IllegalMnemonic();
        }

        disasm.Align(20);
        disasm.OperandWrite("TMP")
            .Operator("<-")
            .OperandRead("TEMP")
            .Operator("[")
            .ImmHex<uint8, 2>(instr.TRA)
            .Operator("+")
            .OperandRead("MDEC_CT")
            .Operator("]");

        disasm.Align(45);
        disasm.OperandWrite("SFT").Operator("<-");
        switch (instr.XSEL) {
        case 0: disasm.OperandRead("TMP"); break;
        case 1: disasm.OperandRead("INPUTS"); break;
        }
        // disasm.Align(56);
        disasm.Operator("*");
        switch (instr.YSEL) {
        case 0: disasm.OperandRead("FRC"); break;
        case 1: disasm.OperandRead("COEF").Operator("[").ImmHex<uint8, 2>(instr.CRA).Operator("]"); break;
        case 2: disasm.OperandRead("Y").BitRange(23, 11); break;
        case 3: disasm.OperandRead("Y").BitRange(15, 4); break;
        }
        if (!instr.ZERO) {
            // disasm.Align(67);
            if (instr.NEGB) {
                disasm.Operator("-");
            } else {
                disasm.Operator("+");
            }
            if (instr.BSEL) {
                disasm.OperandRead("SFT");
            } else {
                disasm.OperandRead("TMP");
            }
        }
        if (instr.SHFT0 ^ instr.SHFT1) {
            // disasm.Align(71);
            disasm.Operator("<<").ImmDec(1);
        }

        if (instr.YRL) {
            disasm.Align(76);
            disasm.OperandWrite("Y").Operator("<-").OperandRead("INPUTS");
        }

        if (instr.FRCL) {
            disasm.Align(87);
            disasm.OperandWrite("FRC").Operator("<-").OperandRead("SFT");
            if (instr.SHFT0 & instr.SHFT1) {
                disasm.BitRange(11, 0);
            } else {
                disasm.BitRange(23, 11);
            }
        }

        disasm.Align(104);
        if (instr.EWT) {
            disasm.OperandWrite("EFREG")
                .Operator("[")
                .ImmHex(instr.EWA)
                .Operator("]")
                .Operator("<-")
                .OperandRead("SFT")
                .BitRange(23, 8);
        }

        disasm.Align(127);
        if (instr.TWT) {
            disasm.OperandWrite("TEMP")
                .Operator("[")
                .ImmHex<uint8, 2>(instr.TWA)
                .Operator("+")
                .OperandRead("MDEC_CT")
                .Operator("]")
                .Operator("<-")
                .OperandRead("SFT");
        }

        disasm.Align(152);
        if (instr.IWT) {
            disasm.OperandWrite("MEMS")
                .Operator("[")
                .ImmHex<uint8, 2>(instr.IWA)
                .Operator("]")
                .Operator("<-")
                .OperandRead("MEM");
        }

        disasm.Align(169);
        if (instr.MRD || instr.MWT) {
            disasm.OperandWrite("MADR").Operator("<-").Operator("(");
            if (!instr.TABLE && (instr.ADREB || instr.NXADR)) {
                disasm.Operator("(");
            }
            disasm.OperandRead("MADRS").Operator("[").ImmHex<uint8, 2>(instr.MASA).Operator("]");
            if (instr.ADREB) {
                disasm.Operator("+").OperandRead("ADRS_REG");
            }
            if (instr.NXADR) {
                disasm.Operator("+").ImmDec(1);
            }
            if (!instr.TABLE) {
                if (instr.ADREB || instr.NXADR) {
                    disasm.Operator(")");
                }
                disasm.Operator("&").OperandRead("RBL");
            }
            disasm.Operator(")").Operator("+").OperandRead("RBP");
        }

        if (instr.MRD) {
            disasm.Align(211);
            disasm.OperandWrite("MEM").Operator("<-").OperandRead("WRAM").Operator("[").OperandRead("MADR").Operator(
                "]");
        }

        if (instr.MWT) {
            disasm.Align(228);
            disasm.OperandWrite("WRAM").Operator("[").OperandRead("MADR").Operator("]").Operator("<-").OperandRead(
                "SFT");
        }

        if ((instr.MRD || instr.MWT) && instr.NOFL) {
            disasm.Align(245);
            disasm.Mnemonic("NOFL");
        }

        if (instr.ADRL) {
            disasm.Align(251);
            disasm.OperandWrite("ADRS_REG").Operator("<-");
            if (instr.SHFT0 & instr.SHFT1) {
                disasm.OperandRead("SFT").BitRange(23, 12);
            } else {
                disasm.OperandRead("INPUTS").BitRange(27, 16);
            }
        }

        return *this;
    }

    SCSPDSPDisassembler &Disassemble(uint64 opcode, bool raw) {
        const ymir::scsp::DSPInstr instr{.u64 = opcode};

        Address().Opcode(opcode);
        if (raw) {
            InstructionRaw(instr);
        } else {
            Instruction(instr);
        }

        disasm.NewLine();

        ++address;

        return *this;
    }

    Disassembler &disasm;
    uint8 address = 0;
};

using SCSPDSPOpcode = uint64;
using SCSPDSPOpcodeFetcher = IOpcodeFetcher<SCSPDSPOpcode>;
using CommandLineSCSPDSPOpcodeFetcher = CommandLineOpcodeFetcher<SCSPDSPOpcode>;
using StreamSCSPDSPOpcodeFetcher = StreamOpcodeFetcher<SCSPDSPOpcode>;

bool DisassembleSCSPDSP(Disassembler &disasm, std::string_view origin, const std::vector<std::string> &args,
                        const std::string &inputFile, bool raw) {
    auto maybeAddress = ParseHex<uint8>(origin);
    if (!maybeAddress) {
        fmt::println("Invalid origin address: {}", origin);
        return false;
    }

    SCSPDSPDisassembler scspDspDisasm{disasm};
    scspDspDisasm.address = *maybeAddress;
    if (scspDspDisasm.address >= 128) {
        fmt::println("Origin address out of range: {} > 7F", origin);
        return false;
    }

    auto fetcher =
        MakeFetcher<SCSPDSPOpcodeFetcher, CommandLineSCSPDSPOpcodeFetcher, StreamSCSPDSPOpcodeFetcher>(args, inputFile);
    if (!fetcher) {
        return false;
    }

    bool running = true;
    const auto visitor = [&](auto &value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, SCSPDSPOpcode>) {
            scspDspDisasm.Disassemble(value, raw);
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
