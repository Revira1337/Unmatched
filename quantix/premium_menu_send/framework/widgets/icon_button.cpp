#include "../headers/widgets.h"
#include "../headers/includes.h"
#include "../headers/functions.h"
#include "../headers/animations.h"
#include "../settings/colors.h"
#include "../headers/draw.h"
#include "../headers/font.h"
#include "../data/icons.h"

bool c_widgets::icon_button(const char* icon, const char* label, const ImVec2& size_arg, bool style_inversed, const char* str_id)
{
    const ImVec2 rect_padding = scale(6, 4);
    const float rect_rounding = scale(6);

    ImGuiWindow* window = gui->get_window();
    ImDrawList* drawlist = window->DrawList;
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(str_id == nullptr ? label : str_id);

    const ImVec2 icon_size = gui->text_size(font->get("icons14"), icon);
    const ImVec2 label_size = gui->text_size(font->get("medium13"), label, nullptr, true);

    const ImVec2 calculated_size = ImVec2(rect_padding.x * 3.f + icon_size.x + label_size.x, ImMax(icon_size.y, label_size.y) + rect_padding.y * 2.f);
    const ImVec2 size = ImVec2(size_arg.x == 0.f ? calculated_size.x : size_arg.x, size_arg.y == 0.f ? calculated_size.y : size_arg.y);
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, pos + size);
    gui->item_size(bb, var->style.frame_padding.y);
    if (!gui->item_add(bb, id))
        return false;

    bool hovered, held;
    bool pressed = gui->button_behavior(bb, id, &hovered, &held);

    struct button_state {
        bool anim_state;
        bool do_move_anim;
        bool last_inverse;
    };

    static std::unordered_map<ImGuiID, button_state> button_states;
    auto& [current_state, do_move, last_inverse] = button_states[id];
    if (pressed) {
        current_state = pressed;
        do_move = true;
        reset_tween<float>({ id, "icon_button_alpha" });
        reset_tween<float>({ id, "icon_button_anim" });
    }

    if (last_inverse != style_inversed)
    {
        current_state = false;
        reset_tween<float>({ id, "icon_button_alpha" });
    }
    last_inverse = style_inversed;

    const float alpha = tween({ id, "icon_button_alpha" }, 0.f, 1.f, easing::QuadEaseInOut, 250ms, current_state);
    const float anim = tween({ id, "icon_button_anim" }, 0.f, 1.f, easing::QuadEaseInOut, 250ms, do_move);
    const ImVec4 bg_color = tween({ id, "icon_button_bg_color" }, col->gray, col->primary, easing::QuadEaseInOut, 250ms, style_inversed);
    const ImVec4 active_color = tween({ id, "icon_button_active_color" }, col->text, col->black, easing::QuadEaseInOut, 250ms, style_inversed);
    const ImVec4 disabled_color = tween({ id, "icon_button_disabled_color" }, col->black, col->text, easing::QuadEaseInOut, 0ms, style_inversed);

    if (anim == 1.f) {
        current_state = false;
    }
    if (alpha == 0.f)
    {
        do_move = false;
    }
    
    draw->rect_filled(drawlist, bb.Min, bb.Max, gui->u32(bg_color), rect_rounding);
    draw->rect_filled(drawlist, ImVec2(bb.GetCenter().x - bb.GetWidth() * 0.5f * anim, bb.Min.y), ImVec2(bb.GetCenter().x + bb.GetWidth() * 0.5f * anim, bb.Max.y), gui->u32(bg_color, alpha), rect_rounding);

    draw->text_clipped(drawlist, font->get("icons14"), bb.Min + rect_padding, bb.Max - rect_padding, gui->u32(active_color, 1.f - alpha), icon, nullptr, nullptr, { 0.f, 0.5f });
    draw->text_clipped(drawlist, font->get("icons14"), bb.Min + rect_padding, bb.Max - rect_padding, gui->u32(disabled_color, alpha), icon, nullptr, nullptr, { 0.f, 0.5f });

    draw->text_clipped(drawlist, font->get("medium13"), bb.Min + rect_padding, bb.Max - rect_padding, gui->u32(active_color, 1.f - alpha), gui->hide_text_hint(label), nullptr, nullptr, { 1.f, 0.5f });
    draw->text_clipped(drawlist, font->get("medium13"), bb.Min + rect_padding, bb.Max - rect_padding, gui->u32(disabled_color, alpha), gui->hide_text_hint(label), nullptr, nullptr, { 1.f, 0.5f });

    return pressed;
}