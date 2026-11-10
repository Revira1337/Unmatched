#include "../headers/includes.h"
#include "../../thirdparty/imgui/imgui_freetype.h"
#include "imgui_impl_dx11.h"
#include "../headers/font.h"
#include "../settings/variables.h"
#include "../headers/functions.h"

void c_font::update()
{
    if (var->gui.dpi_changed)
    {
        var->gui.dpi = var->gui.stored_dpi;

        ImFontConfig cfg;
        cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_Bitmap;
        cfg.FontDataOwnedByAtlas = false;

        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Clear();

        static ImVector<ImWchar> ranges;
        if (ranges.empty()) {
            ImFontGlyphRangesBuilder builder;
            builder.AddChar(0x00B0);
            builder.AddChar(0x201C);
            builder.AddChar(0x201D);
            builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
            builder.BuildRanges(&ranges);
        }

        for (auto& [name, font] : fonts_map) {
            cfg.GlyphMinAdvanceX = (font.is_mono ? scale(font.size) : 0.f);
            cfg.GlyphMaxAdvanceX = (font.is_mono ? scale(font.size) : FLT_MAX);

            font.out_font = io.Fonts->AddFontFromMemoryCompressedTTF(font.data, (int)font.data_size, scale(font.size), &cfg, font.ranges.data ? font.ranges.data : ranges.Data);
        }

        io.Fonts->Build();
        ImGui_ImplDX11_CreateDeviceObjects();

        var->gui.dpi_changed = false;
    }
}

void c_font::add(const std::string& name, const void* data, size_t data_size, float size, font_ranges ranges, bool mono)
{
    fonts_map.emplace(name, font_data{ data, data_size, size, nullptr, ranges, mono });
}

ImFont* c_font::get(const std::string& name) {
    return fonts_map.at(name).out_font;
}
