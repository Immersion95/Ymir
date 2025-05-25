#pragma once

#include <ymir/hw/vdp/vdp_defs.hpp>

#include <ymir/core/types.hpp>

#include <array>

namespace ymir::state {

// Version history:
// v5:
// - New fields
//   - VDPRendererState::erase = false
// v4:
// - Changed fields
//   - NormBGLayerState::fracScrollX and fracScrollY no longer include the values of SC[XY][ID]N#. Therefore, they need
//     to be compensated for as follows:
//       normBGLayerStates[0].fracScrollX -= (regs2.SCXIN0 << 8u) | (regs2.SCXDN0 >> 8u);
//       normBGLayerStates[1].fracScrollX -= (regs2.SCXIN1 << 8u) | (regs2.SCXDN1 >> 8u);
//       normBGLayerStates[2].fracScrollX -= (regs2.SCXIN2 << 8u);
//       normBGLayerStates[3].fracScrollX -= (regs2.SCXIN3 << 8u);
//
//       normBGLayerStates[0].fracScrollY -= (regs2.SCYIN0 << 8u) | (regs2.SCYDN0 >> 8u);
//       normBGLayerStates[1].fracScrollY -= (regs2.SCYIN1 << 8u) | (regs2.SCYDN1 >> 8u);
//       normBGLayerStates[2].fracScrollY -= (regs2.SCYIN2 << 8u);
//       normBGLayerStates[3].fracScrollY -= (regs2.SCYIN3 << 8u);
// - New fields
//   - VDPRendererState::vertCellScrollInc = sizeof(uint32)
//   - NormBGLayerState::vertCellScrollOffset = 0

struct VDPState {
    alignas(16) std::array<uint8, vdp::kVDP1VRAMSize> VRAM1;
    alignas(16) std::array<uint8, vdp::kVDP2VRAMSize> VRAM2;
    alignas(16) std::array<uint8, vdp::kVDP2CRAMSize> CRAM;
    alignas(16) std::array<std::array<uint8, vdp::kVDP1FramebufferRAMSize>, 2> spriteFB;
    uint8 displayFB;

    struct VDP1RegsState {
        uint16 TVMR;
        uint16 FBCR;
        uint16 PTMR;
        uint16 EWDR;
        uint16 EWLR;
        uint16 EWRR;
        uint16 EDSR;
        uint16 LOPR;
        uint16 COPR;
        uint16 MODR;
        bool manualSwap;
        bool manualErase;
    } regs1;

    struct VDP2RegsState {
        uint16 TVMD;
        uint16 EXTEN;
        uint16 TVSTAT;
        uint16 VRSIZE;
        uint16 HCNT;
        uint16 VCNT;
        uint16 RAMCTL;
        uint16 CYCA0L;
        uint16 CYCA0U;
        uint16 CYCA1L;
        uint16 CYCA1U;
        uint16 CYCB0L;
        uint16 CYCB0U;
        uint16 CYCB1L;
        uint16 CYCB1U;
        uint16 BGON;
        uint16 MZCTL;
        uint16 SFSEL;
        uint16 SFCODE;
        uint16 CHCTLA;
        uint16 CHCTLB;
        uint16 BMPNA;
        uint16 BMPNB;
        uint16 PNCNA;
        uint16 PNCNB;
        uint16 PNCNC;
        uint16 PNCND;
        uint16 PNCR;
        uint16 PLSZ;
        uint16 MPOFN;
        uint16 MPOFR;
        uint16 MPABN0;
        uint16 MPCDN0;
        uint16 MPABN1;
        uint16 MPCDN1;
        uint16 MPABN2;
        uint16 MPCDN2;
        uint16 MPABN3;
        uint16 MPCDN3;
        uint16 MPABRA;
        uint16 MPCDRA;
        uint16 MPEFRA;
        uint16 MPGHRA;
        uint16 MPIJRA;
        uint16 MPKLRA;
        uint16 MPMNRA;
        uint16 MPOPRA;
        uint16 MPABRB;
        uint16 MPCDRB;
        uint16 MPEFRB;
        uint16 MPGHRB;
        uint16 MPIJRB;
        uint16 MPKLRB;
        uint16 MPMNRB;
        uint16 MPOPRB;
        uint16 SCXIN0;
        uint16 SCXDN0;
        uint16 SCYIN0;
        uint16 SCYDN0;
        uint16 ZMXIN0;
        uint16 ZMXDN0;
        uint16 ZMYIN0;
        uint16 ZMYDN0;
        uint16 SCXIN1;
        uint16 SCXDN1;
        uint16 SCYIN1;
        uint16 SCYDN1;
        uint16 ZMXIN1;
        uint16 ZMXDN1;
        uint16 ZMYIN1;
        uint16 ZMYDN1;
        uint16 SCXIN2;
        uint16 SCYIN2;
        uint16 SCXIN3;
        uint16 SCYIN3;
        uint16 ZMCTL;
        uint16 SCRCTL;
        uint16 VCSTAU;
        uint16 VCSTAL;
        uint16 LSTA0U;
        uint16 LSTA0L;
        uint16 LSTA1U;
        uint16 LSTA1L;
        uint16 LCTAU;
        uint16 LCTAL;
        uint16 BKTAU;
        uint16 BKTAL;
        uint16 RPMD;
        uint16 RPRCTL;
        uint16 KTCTL;
        uint16 KTAOF;
        uint16 OVPNRA;
        uint16 OVPNRB;
        uint16 RPTAU;
        uint16 RPTAL;
        uint16 WPSX0;
        uint16 WPSY0;
        uint16 WPEX0;
        uint16 WPEY0;
        uint16 WPSX1;
        uint16 WPSY1;
        uint16 WPEX1;
        uint16 WPEY1;
        uint16 WCTLA;
        uint16 WCTLB;
        uint16 WCTLC;
        uint16 WCTLD;
        uint16 LWTA0U;
        uint16 LWTA0L;
        uint16 LWTA1U;
        uint16 LWTA1L;
        uint16 SPCTL;
        uint16 SDCTL;
        uint16 CRAOFA;
        uint16 CRAOFB;
        uint16 LNCLEN;
        uint16 SFPRMD;
        uint16 CCCTL;
        uint16 SFCCMD;
        uint16 PRISA;
        uint16 PRISB;
        uint16 PRISC;
        uint16 PRISD;
        uint16 PRINA;
        uint16 PRINB;
        uint16 PRIR;
        uint16 CCRSA;
        uint16 CCRSB;
        uint16 CCRSC;
        uint16 CCRSD;
        uint16 CCRNA;
        uint16 CCRNB;
        uint16 CCRR;
        uint16 CCRLB;
        uint16 CLOFEN;
        uint16 CLOFSL;
        uint16 COAR;
        uint16 COAG;
        uint16 COAB;
        uint16 COBR;
        uint16 COBG;
        uint16 COBB;
    } regs2;

    enum class HorizontalPhase { Active, RightBorder, Sync, VBlankOut, LeftBorder, LastDot };
    HorizontalPhase HPhase; // Current horizontal display phase

    enum class VerticalPhase { Active, BottomBorder, BlankingAndSync, TopBorder, LastLine };
    VerticalPhase VPhase; // Current vertical display phase

    uint16 VCounter;

    struct VDPRendererState {
        struct VDP1RenderState {
            uint16 sysClipH;
            uint16 sysClipV;

            uint16 userClipX0;
            uint16 userClipY0;
            uint16 userClipX1;
            uint16 userClipY1;

            sint32 localCoordX;
            sint32 localCoordY;

            bool rendering;

            bool erase;

            uint64 cycleCount;
        };

        struct NormBGLayerState {
            uint32 fracScrollX;
            uint32 fracScrollY;
            uint32 scrollIncH;
            uint32 lineScrollTableAddress;
            uint32 vertCellScrollOffset;
            uint8 mosaicCounterY;
        };

        struct RotationParamState {
            std::array<uint32, 16> pageBaseAddresses;
            sint32 scrX, scrY;
            uint32 KA;
        };

        struct LineBackLayerState {
            uint32 lineColor;
            uint32 backColor;
        };

        VDP1RenderState vdp1State;
        std::array<NormBGLayerState, 4> normBGLayerStates;
        std::array<RotationParamState, 2> rotParamStates;
        LineBackLayerState lineBackLayerState;
        uint32 vertCellScrollInc;

        uint8 displayFB;
        bool vdp1Done;
    } renderer;
};

} // namespace ymir::state
