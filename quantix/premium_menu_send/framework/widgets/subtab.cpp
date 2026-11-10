#include "../headers/widgets.h"
#include "../headers/includes.h"
#include "../headers/functions.h"
#include "../headers/animations.h"
#include "../settings/colors.h"
#include "../headers/draw.h"
#include "../headers/font.h"
#include "../data/icons.h"

bool c_widgets::subtab(const char* label, bool is_active)
{
    ImGuiWindow* window = gui->get_window();
    ImDrawList* drawlist = window->DrawList;
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(label);

    // Base padding and sizing
    const ImVec2 base_padding = ImVec2(scale(12), scale(8));
    const ImVec2 label_size = gui->text_size(font->get("medium12"), label, nullptr, true);
    
    // Animate size growth for active tab (grows slightly larger)
    const float size_scale = tween({ id, "subtab_size_scale" }, 1.0f, 1.08f, easing::QuadEaseOut, 200ms, is_active);
    const ImVec2 padding = base_padding * size_scale;
    const ImVec2 size = label_size + padding * 2.f;

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, pos + size);
    gui->item_size(bb, 0.f);
    if (!gui->item_add(bb, id))
        return false;

    bool hovered, held;
    bool pressed = gui->button_behavior(bb, id, &hovered, &held);

    // Smooth alpha animation for background
    const float alpha = tween({ id, "subtab_alpha" }, 0.f, 1.f, easing::QuadEaseInOut, 200ms, is_active);
    
    // Text color animation
    const ImVec4 text_color = tween({ id, "subtab_text_color" }, col->text_dimmed, col->text, easing::QuadEaseInOut, 200ms, is_active);
    
    // Font scale animation (subtle size change for text)
    const float font_scale = tween({ id, "subtab_font_scale" }, 1.0f, 1.05f, easing::QuadEaseOut, 200ms, is_active);
    
    // Hover effect (subtle highlight when not active)
    const float hover_alpha = tween({ id, "subtab_hover" }, 0.f, 0.03f, easing::QuadEaseInOut, 150ms, hovered && !is_active);

    // Draw background - grows from center with animation
    const ImVec2 bg_size = bb.GetSize() * alpha;
    draw->rect_filled(drawlist, bb.GetCenter() - bg_size * 0.5f, bb.GetCenter() + bg_size * 0.5f, gui->u32(col->text, 0.05f * alpha + hover_alpha), var->style.frame_rounding);

    // Draw text with animated font scale
    ImFont* text_font = font->get("medium12");
    if (text_font) {
        const float original_scale = text_font->Scale;
        text_font->Scale = font_scale;
        draw->text_clipped(drawlist, text_font, bb.Min, bb.Max, gui->u32(text_color), gui->hide_text_hint(label), nullptr, nullptr, { 0.5f, 0.5f });
        text_font->Scale = original_scale;
    } else {
        draw->text_clipped(drawlist, font->get("medium12"), bb.Min, bb.Max, gui->u32(text_color), gui->hide_text_hint(label), nullptr, nullptr, { 0.5f, 0.5f });
    }

    return pressed;
}