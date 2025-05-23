#include "cdblock_cmd_trace_view.hpp"

namespace app::ui {

CDBlockCommandTraceView::CDBlockCommandTraceView(SharedContext &context)
    : m_context(context)
    , m_tracer(context.tracers.CDBlock) {}

void CDBlockCommandTraceView::Display() {
    const float paddingWidth = ImGui::GetStyle().FramePadding.x;
    ImGui::PushFont(m_context.fonts.monospace.medium.regular);
    const float hexCharWidth = ImGui::CalcTextSize("F").x;
    ImGui::PopFont();

    ImGui::BeginGroup();

    ImGui::Checkbox("Enable", &m_tracer.traceCommands);
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::TextUnformatted("You must also enable tracing in Debug > Enable tracing (F11)");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        m_tracer.ClearCommands();
    }

    if (ImGui::BeginTable("cdblock_cmd_trace", 3,
                          ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable)) {
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_PreferSortDescending);
        ImGui::TableSetupColumn("Request", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
                                paddingWidth * 2 + hexCharWidth * (4 + 1 + 4 + 1 + 4 + 1 + 4));
        ImGui::TableSetupColumn("Response", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
                                paddingWidth * 2 + hexCharWidth * (4 + 1 + 4 + 1 + 4 + 1 + 4));
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableHeadersRow();

        const size_t count = m_tracer.commands.Count();
        for (size_t i = 0; i < count; i++) {
            auto *sort = ImGui::TableGetSortSpecs();
            bool reverse = false;
            if (sort != nullptr && sort->SpecsCount == 1) {
                reverse = sort->Specs[0].SortDirection == ImGuiSortDirection_Descending;
            }

            auto trace = reverse ? m_tracer.commands.ReadReverse(i) : m_tracer.commands.Read(i);

            ImGui::TableNextRow();
            if (ImGui::TableNextColumn()) {
                ImGui::PushFont(m_context.fonts.monospace.medium.regular);
                ImGui::Text("%u", trace.index);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                ImGui::PushFont(m_context.fonts.monospace.medium.regular);
                ImGui::Text("%04X %04X %04X %04X", trace.request[0], trace.request[1], trace.request[2],
                            trace.request[3]);
                ImGui::PopFont();
            }
            if (ImGui::TableNextColumn()) {
                if (trace.processed) {
                    ImGui::PushFont(m_context.fonts.monospace.medium.regular);
                    ImGui::Text("%04X %04X %04X %04X", trace.response[0], trace.response[1], trace.response[2],
                                trace.response[3]);
                    ImGui::PopFont();
                }
            }
        }

        ImGui::EndTable();
    }

    ImGui::EndGroup();
}

} // namespace app::ui
