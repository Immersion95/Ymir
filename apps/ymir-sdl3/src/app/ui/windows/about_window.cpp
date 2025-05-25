#include "about_window.hpp"

#include <ymir/util/compiler_info.hpp>

// Includes for versions only
#include <SDL3/SDL.h>
#include <cxxopts.hpp>
#include <fmt/format.h>
#include <lz4.h>
#include <toml++/toml.hpp>
#include <xxhash.h>

#define _STR_IMPL(x) #x
#define _STR(x) _STR_IMPL(x)
#define CEREAL_VERSION "1.3.2" // Not exported
#define CMRC_VERSION "2.0.0"   // Not exported
#define CXXOPTS_VERSION _STR(CXXOPTS__VERSION_MAJOR) "." _STR(CXXOPTS__VERSION_MINOR) "." _STR(CXXOPTS__VERSION_PATCH)
#define IMGUI_VERSION_FULL IMGUI_VERSION " (" _STR(IMGUI_VERSION_NUM) ")"
#define LZMA_VERSION "24.05" // Private dependency of libchdr
#define MIO_VERSION "1.1.0"  // Not exported
#define SDL_VERSION_STR _STR(SDL_MAJOR_VERSION) "." _STR(SDL_MINOR_VERSION) "." _STR(SDL_MICRO_VERSION)
#define STB_IMAGE_VERSION "2.30"     // Not exported
#define MC_CONCQUEUE_VERSION "1.0.4" // Not exported
#define TOMLPP_VERSION _STR(TOML_LIB_MAJOR) "." _STR(TOML_LIB_MINOR) "." _STR(TOML_LIB_PATCH)
#define XXHASH_VERSION _STR(XXH_VERSION_MAJOR) "." _STR(XXH_VERSION_MINOR) "." _STR(XXH_VERSION_RELEASE)
#define ZLIB_VERSION "1.3.1" // Private dependency of libchdr
#define ZSTD_VERSION "1.5.6" // Private dependency of libchdr

static const std::string fmtVersion = std::to_string(FMT_VERSION / 10000) + "." +
                                      std::to_string(FMT_VERSION / 100 % 100) + "." + std::to_string(FMT_VERSION % 100);

namespace app::ui {

struct License {
    const char *name;
    const char *url;
};

struct FontDesc {
    using FontFn = ImFont *(*)(SharedContext &ctx);

    const char *name;
    const License &license;
    const char *url;
    FontFn fontFn;
};

// clang-format off
//inline constexpr License licenseApache2_0   {.name = "Apache-2.0",    .url = "https://opensource.org/licenses/Apache-2.0"};
inline constexpr License licenseBSD2        {.name = "BSD-2-Clause",  .url = "https://opensource.org/licenses/BSD-2-Clause"};
inline constexpr License licenseBSD3        {.name = "BSD-3-Clause",  .url = "https://opensource.org/licenses/BSD-3-Clause"};
//inline constexpr License licenseBSL         {.name = "BSL-1.0", .      url = "https://opensource.org/license/bsl-1-0"};
//inline constexpr License licenseISC         {.name = "ISC",           .url = "https://opensource.org/licenses/ISC"};
inline constexpr License licenseMIT         {.name = "MIT",           .url = "https://opensource.org/licenses/MIT"};
inline constexpr License licensePublicDomain{.name = "Public domain", .url = nullptr};
//inline constexpr License licenseUnlicense   {.name = "Unlicense",     .url = "https://opensource.org/licenses/unlicense"};
inline constexpr License licenseZlib        {.name = "Zlib",          .url = "https://opensource.org/licenses/Zlib"};
inline constexpr License licenseOFL         {.name = "OFL-1.1",       .url = "https://opensource.org/licenses/OFL-1.1"};

static const struct {
    const char *name;
    const char *version = nullptr;
    const License &license;
    const char *repoURL;
    const char *licenseURL = nullptr;
    const bool repoPrivate = false;
    const char *homeURL = nullptr;
} depsCode[] = {
    {.name = "cereal",                        .version = CEREAL_VERSION,             .license = licenseBSD3,          .repoURL = "https://github.com/USCiLab/cereal",              .licenseURL = "https://github.com/USCiLab/cereal/blob/master/LICENSE",                  .homeURL = "https://uscilab.github.io/cereal/index.html"},
    {.name = "CMakeRC",                       .version = CMRC_VERSION,               .license = licenseMIT,           .repoURL = "https://github.com/vector-of-bool/cmrc",         .licenseURL = "https://github.com/vector-of-bool/cmrc/blob/master/LICENSE.txt"},
    {.name = "cxxopts",                       .version = CXXOPTS_VERSION,            .license = licenseMIT,           .repoURL = "https://github.com/jarro2783/cxxopts",           .licenseURL = "https://github.com/jarro2783/cxxopts/blob/master/LICENSE"},
    {.name = "Dear ImGui",                    .version = IMGUI_VERSION_FULL,         .license = licenseMIT,           .repoURL = "https://github.com/ocornut/imgui",               .licenseURL = "https://github.com/ocornut/imgui/blob/master/LICENSE.txt"},
    {.name = "{fmt}",                         .version = fmtVersion.c_str(),         .license = licenseMIT,           .repoURL = "https://github.com/fmtlib/fmt",                  .licenseURL = "https://github.com/fmtlib/fmt/blob/master/LICENSE",                      .homeURL = "https://fmt.dev/latest/index.html"},
    {.name = "ImGui Club",                                                           .license = licenseMIT,           .repoURL = "https://github.com/ocornut/imgui_club",          .licenseURL = "https://github.com/ocornut/imgui_club/blob/main/LICENSE.txt"},
    {.name = "libchdr",                                                              .license = licenseBSD3,          .repoURL = "https://github.com/rtissera/libchdr",            .licenseURL = "https://github.com/rtissera/libchdr/blob/master/LICENSE.txt"},
    {.name = "lz4",                           .version = LZ4_VERSION_STRING,         .license = licenseBSD2,          .repoURL = "https://github.com/lz4/lz4",                     .licenseURL = "https://github.com/lz4/lz4/blob/dev/lib/LICENSE",                        .homeURL = "https://lz4.org/",},
    {.name = "lzma",                          .version = LZMA_VERSION,               .license = licensePublicDomain,                                                                                                                                                       .homeURL = "https://www.7-zip.org/sdk.html",},
    {.name = "mio",                           .version = MIO_VERSION,                .license = licenseMIT,           .repoURL = "https://github.com/StrikerX3/mio",               .licenseURL = "https://github.com/StrikerX3/mio/blob/master/LICENSE"},
    {.name = "moodycamel::\nConcurrentQueue", .version = "\n" MC_CONCQUEUE_VERSION,  .license = licenseBSD2,          .repoURL = "https://github.com/cameron314/concurrentqueue",  .licenseURL = "https://github.com/cameron314/concurrentqueue/blob/master/LICENSE.md"},
    {.name = "SDL3",                          .version = SDL_VERSION_STR,            .license = licenseZlib,          .repoURL = "https://github.com/libsdl-org/SDL",              .licenseURL = "https://github.com/libsdl-org/SDL/blob/main/LICENSE.txt"},
    {.name = "stb_image",                     .version = STB_IMAGE_VERSION,          .license = licenseMIT,           .repoURL = "https://github.com/nothings/stb",                .licenseURL = "https://github.com/nothings/stb/blob/master/LICENSE"},
    {.name = "toml++",                        .version = TOMLPP_VERSION,             .license = licenseMIT,           .repoURL = "https://github.com/marzer/tomlplusplus" ,        .licenseURL = "https://github.com/marzer/tomlplusplus/blob/master/LICENSE",             .homeURL = "https://marzer.github.io/tomlplusplus/"},
    {.name = "xxHash",                        .version = XXHASH_VERSION,             .license = licenseBSD2,          .repoURL = "https://github.com/Cyan4973/xxHash",             .licenseURL = "https://github.com/Cyan4973/xxHash/blob/dev/LICENSE",                    .homeURL = "https://xxhash.com/"},
    {.name = "zlib",                          .version = ZLIB_VERSION,               .license = licenseZlib,          .repoURL = "https://github.com/madler/zlib",                 .licenseURL = "https://github.com/madler/zlib/blob/develop/LICENSE",                    .homeURL = "https://zlib.net/"},
    {.name = "zstd",                          .version = ZSTD_VERSION,               .license = licenseBSD3,          .repoURL = "https://github.com/facebook/zstd",               .licenseURL = "https://github.com/facebook/zstd/blob/dev/LICENSE",                      .homeURL = "http://www.zstd.net/"},
};


static const FontDesc fontDescs[] = {
    { .name = "Spline Sans",      .license = licenseOFL, .url = "https://github.com/SorkinType/SplineSans",     .fontFn = [](SharedContext &ctx) -> ImFont * { return ctx.fonts.sansSerif.medium.regular; } },
    { .name = "Spline Sans Mono", .license = licenseOFL, .url = "https://github.com/SorkinType/SplineSansMono", .fontFn = [](SharedContext &ctx) -> ImFont * { return ctx.fonts.monospace.medium.regular; } },
    { .name = "Zen Dots",         .license = licenseOFL, .url = "https://github.com/googlefonts/zen-dots",      .fontFn = [](SharedContext &ctx) -> ImFont * { return ctx.fonts.display.small;            } },
};
// clang-format on

AboutWindow::AboutWindow(SharedContext &context)
    : WindowBase(context) {

    m_windowConfig.name = "About";
}

void AboutWindow::PrepareWindow() {
    auto *vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(vp->Pos.x + vp->Size.x * 0.5f, vp->Pos.y + vp->Size.y * 0.5f),
                            ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(660 * m_context.displayScale, 800 * m_context.displayScale),
                             ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(400 * m_context.displayScale, 240 * m_context.displayScale),
                                        ImVec2(FLT_MAX, FLT_MAX));
}

void AboutWindow::DrawContents() {
    if (ImGui::BeginTabBar("##tabs")) {
        if (ImGui::BeginTabItem("About")) {
            if (ImGui::BeginChild("##about")) {
                DrawAboutTab();
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Dependencies")) {
            if (ImGui::BeginChild("##dependencies")) {
                DrawDependenciesTab();
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Acknowledgements")) {
            if (ImGui::BeginChild("##acknowledgements", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
                DrawAcknowledgementsTab();
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void AboutWindow::DrawAboutTab() {
    ImGui::PushTextWrapPos(ImGui::GetWindowContentRegionMax().x);

    ImGui::Image((ImTextureID)m_context.images.ymirLogo.texture,
                 ImVec2(m_context.images.ymirLogo.size.x * m_context.displayScale,
                        m_context.images.ymirLogo.size.y * m_context.displayScale));

    ImGui::PushFont(m_context.fonts.display.large);
    ImGui::TextUnformatted("Ymir");
    ImGui::PopFont();
    ImGui::PushFont(m_context.fonts.sansSerif.xlarge.bold);
    ImGui::TextUnformatted("Version " Ymir_VERSION);
    ImGui::PopFont();

    ImGui::PushFont(m_context.fonts.sansSerif.large.regular);
    ImGui::TextUnformatted("A Sega Saturn emulator");
    ImGui::PopFont();

    ImGui::NewLine();
    ImGui::Text("Compiled with %s %s.", compiler::name, compiler::version::string.c_str());
#ifdef Ymir_AVX2
    ImGui::Text("Using AVX2 instruction set.");
#else
    ImGui::Text("Using SSE2 instruction set.");
#endif

    ImGui::NewLine();
    ImGui::TextUnformatted("Licensed under ");
    ImGui::SameLine(0, 0);
    ImGui::TextLinkOpenURL("GPLv3", "https://www.gnu.org/licenses/gpl-3.0.en.html");

    ImGui::NewLine();
    ImGui::TextUnformatted("The source code can be found at ");
    ImGui::SameLine(0, 0);
    ImGui::TextLinkOpenURL("https://github.com/StrikerX3/Ymir");
}

void AboutWindow::DrawDependenciesTab() {
    static constexpr ImGuiTableFlags kTableFlags = ImGuiTableFlags_SizingFixedFit;

    // -----------------------------------------------------------------------------

    ImGui::PushFont(m_context.fonts.sansSerif.large.bold);
    ImGui::TextUnformatted("Libraries");
    ImGui::PopFont();

    if (ImGui::BeginTable("libraries", 3, kTableFlags)) {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("License");
        ImGui::TableSetupColumn("Links");
        ImGui::TableHeadersRow();

        for (auto &dep : depsCode) {
            ImGui::PushID(dep.name);
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::PushFont(m_context.fonts.sansSerif.medium.bold);
            ImGui::TextUnformatted(dep.name);
            ImGui::PopFont();
            if (dep.version != nullptr) {
                ImGui::SameLine();
                // TODO: don't hardcode colors!
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.77f, 0.8f, 1.0f));
                ImGui::TextUnformatted(dep.version);
                ImGui::PopStyleColor();
            }

            ImGui::TableSetColumnIndex(1);
            if (dep.licenseURL != nullptr) {
                ImGui::TextLinkOpenURL(dep.license.name, dep.licenseURL);
            } else if (dep.license.url != nullptr) {
                ImGui::TextLinkOpenURL(dep.license.name, dep.license.url);
            } else {
                ImGui::TextUnformatted(dep.license.name);
            }

            ImGui::TableSetColumnIndex(2);
            if (dep.repoURL != nullptr) {
                ImGui::TextLinkOpenURL(dep.repoURL);
            }
            if (dep.repoPrivate) {
                ImGui::SameLine();
                // TODO: don't hardcode colors!
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.77f, 0.8f, 1.0f));
                ImGui::TextUnformatted("(private)");
                ImGui::PopStyleColor();
            }
            if (dep.homeURL != nullptr) {
                ImGui::TextLinkOpenURL(dep.homeURL);
            }
            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    // -----------------------------------------------------------------------------

    ImGui::Separator();

    ImGui::PushFont(m_context.fonts.sansSerif.large.bold);
    ImGui::TextUnformatted("Fonts");
    ImGui::PopFont();

    if (ImGui::BeginTable("fonts", 3, kTableFlags)) {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("License");
        ImGui::TableSetupColumn("Link");
        ImGui::TableHeadersRow();

        for (auto &font : fontDescs) {
            ImGui::PushID(font.name);
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            auto cursor = ImGui::GetCursorPos();
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));
            ImGui::Selectable("", false, ImGuiSelectableFlags_SpanAllColumns);
            ImGui::PopStyleColor(2);
            if (ImGui::IsItemHovered() && (ImGui::TableGetColumnFlags(0) & ImGuiTableColumnFlags_IsHovered)) {
                ImGui::BeginTooltip();
                ImGui::PushFont(font.fontFn(m_context));
                ImGui::TextUnformatted("The quick brown fox jumps over the lazy dog\n"
                                       "0123456789 `~!@#$%^&*()_+-=[]{}<>,./?;:'\"\\|\n"
                                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ  \u00C0\u00C9\u00CE\u00D5\u00DA\u00D1\u00C7\u00DD\n"
                                       "abcdefghijklmnopqrstuvwxyz  \u00E0\u00E9\u00EE\u00F5\u00FA\u00F1\u00E7\u00FD");
                ImGui::PopFont();
                ImGui::EndTooltip();
            }
            ImGui::SetCursorPos(cursor);

            ImGui::PushFont(m_context.fonts.sansSerif.medium.bold);
            ImGui::TextUnformatted(font.name);
            ImGui::PopFont();

            ImGui::TableSetColumnIndex(1);
            ImGui::TextLinkOpenURL(font.license.name, font.license.url);

            ImGui::TableSetColumnIndex(2);
            ImGui::TextLinkOpenURL(font.url);
            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}

void AboutWindow::DrawAcknowledgementsTab() {
    {
        ImGui::PushFont(m_context.fonts.sansSerif.large.bold);
        ImGui::TextUnformatted("Ymir was made possible by");
        ImGui::PopFont();

        auto ack = [&](const char *name, const char *url) {
            ImGui::PushFont(m_context.fonts.sansSerif.medium.bold);
            ImGui::TextLinkOpenURL(name, url);
            ImGui::PopFont();
        };

        auto ackWithAuthor = [&](const char *name, const char *author, const char *url) {
            ImGui::PushFont(m_context.fonts.sansSerif.medium.bold);
            ImGui::TextLinkOpenURL(name, url);
            ImGui::PopFont();

            ImGui::SameLine();

            ImGui::PushFont(m_context.fonts.sansSerif.medium.regular);
            ImGui::Text("by %s", author);
            ImGui::PopFont();
        };

        ackWithAuthor("antime's feeble Sega Saturn page", "antime", "https://antime.kapsi.fi/sega/");
        ackWithAuthor("Hardware signal traces", "Sergiy Dvodnenko (srg320)", "https://github.com/srg320/Saturn_hw");
        ackWithAuthor("Original research", "Charles MacDonald",
                      "https://web.archive.org/web/20150119062930/http://cgfm2.emuviews.com/saturn.php");
        {
            ImGui::Indent();
            ack("Sega Saturn hardware notes (sattech.txt)",
                "https://web.archive.org/web/20140318183509/http://cgfm2.emuviews.com/txt/sattech.txt");
            ack("VDP1 hardware notes (vdp1tech.txt)",
                "https://web.archive.org/web/20150106171745/http://cgfm2.emuviews.com/sat/vdp1tech.txt");
            ack("Sega Saturn Cartridge Information (satcart.txt)",
                "https://web.archive.org/web/20140724061526/http://cgfm2.emuviews.com/sat/satcart.txt");
            ack("EMS Action Replay Plus notes (satar.txt)",
                "https://web.archive.org/web/20140724045721/http://cgfm2.emuviews.com/sat/satar.txt");
            ack("Comms Link hardware notes (comminfo.txt)",
                "https://web.archive.org/web/20140724035829/http://cgfm2.emuviews.com/sat/comminfo.txt");
            ImGui::Unindent();
        }
        ackWithAuthor("Collection of Dreamcast docs", "Senryoku",
                      "https://github.com/Senryoku/dreamcast-docs/tree/master/AICA/DOCS");
        {
            ImGui::Indent();
            ackWithAuthor("Original AICA research", "Neill Corlett",
                          "https://raw.githubusercontent.com/Senryoku/dreamcast-docs/refs/heads/master/"
                          "AICA/DOCS/myaica.txt");
            ImGui::Unindent();
        }
        ack("Yabause wiki", "http://wiki.yabause.org/");
        ack("SegaRetro on Sega Saturn", "https://segaretro.org/Sega_Saturn");

        // -----------------------------------------------------------------------------

        ImGui::NewLine();

        ImGui::PushFont(m_context.fonts.sansSerif.large.bold);
        ImGui::TextUnformatted("Helpful tools and test suites");
        ImGui::PopFont();

        ackWithAuthor("libyaul", "mrkotfw and contributors", "https://github.com/yaul-org/libyaul");
        ackWithAuthor("libyaul-examples", "mrkotfw and contributors", "https://github.com/yaul-org/libyaul-examples");
        ackWithAuthor("saturn-tests", "StrikerX3", "https://github.com/StrikerX3/saturn-tests");
        ackWithAuthor("SH-4 single step tests", "raddad772", "https://github.com/SingleStepTests/sh4");
        ackWithAuthor("M68000 single step tests", "raddad772", "https://github.com/SingleStepTests/m68000");
        ackWithAuthor("scsptest and scspadpcm", "celeriyacon", "https://github.com/celeriyacon");

        // -----------------------------------------------------------------------------

        ImGui::NewLine();

        ImGui::PushFont(m_context.fonts.sansSerif.large.bold);
        ImGui::TextUnformatted("Other emulators that inspired Ymir");
        ImGui::PopFont();

        ackWithAuthor("Mednafen", "various contributors", "https://mednafen.github.io/");
        ImGui::SameLine();
        ImGui::TextLinkOpenURL("(libretro git mirror)##mednafen", "https://github.com/libretro-mirrors/mednafen-git");
        ackWithAuthor("Yaba Sanshiro 2", "devmiyax", "https://github.com/devmiyax/yabause");
        ImGui::SameLine();
        ImGui::TextLinkOpenURL("(site)##yaba_sanshiro_2", "https://www.uoyabause.org/");
        ackWithAuthor("Yabause", "Guillaume Duhamel and contributors", "https://github.com/Yabause/yabause");
        ackWithAuthor("Mesen2", "Sour and contributors", "https://github.com/SourMesen/Mesen2");
        ImGui::SameLine();
        ImGui::TextLinkOpenURL("(site)##mesen", "https://www.mesen.ca/");
        ackWithAuthor("openMSX", "openMSX developers", "https://github.com/openMSX/openMSX");
        ImGui::SameLine();
        ImGui::TextLinkOpenURL("(site)##openmsx", "https://openmsx.org/");

        // -----------------------------------------------------------------------------

        ImGui::NewLine();

        ImGui::PushFont(m_context.fonts.sansSerif.large.bold);
        ImGui::TextUnformatted("Special thanks");
        ImGui::PopFont();

        ImGui::TextUnformatted("To the ");
        ImGui::SameLine(0, 0);
        ImGui::TextLinkOpenURL("/r/EmuDev community", "https://www.reddit.com/r/EmuDev/");
        ImGui::SameLine(0, 0);
        ImGui::TextUnformatted(" and their ");
        ImGui::SameLine(0, 0);
        ImGui::TextLinkOpenURL("Discord server", "https://discord.gg/dkmJAes");
        ImGui::SameLine(0, 0);
        ImGui::TextUnformatted(".");

        ImGui::TextUnformatted("To the ");
        ImGui::SameLine(0, 0);
        ImGui::TextLinkOpenURL("project contributors", "https://github.com/StrikerX3/Ymir/graphs/contributors");
        ImGui::SameLine(0, 0);
        ImGui::TextUnformatted(" and users ");
        ImGui::SameLine(0, 0);
        ImGui::TextLinkOpenURL("reporting issues and feature requests", "https://github.com/StrikerX3/Ymir/issues");
        ImGui::SameLine(0, 0);
        ImGui::TextUnformatted(".");

        ImGui::PushFont(m_context.fonts.sansSerif.large.bold);
        ImGui::TextUnformatted("And YOU!");
        ImGui::PopFont();
    }
}

} // namespace app::ui
