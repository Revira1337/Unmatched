#include "../headers/widgets.h"
#include "../headers/includes.h"
#include "../headers/functions.h"
#include "../headers/animations.h"
#include "../settings/colors.h"
#include "../headers/draw.h"
#include "../headers/font.h"
#include "../data/icons.h"

bool c_widgets::button(const char* label, const ImVec2& size_arg)
{
    const float rect_rounding = scale(10);

    ImGuiWindow* window = gui->get_window();
    ImDrawList* drawlist = window->DrawList;
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(label);

    const ImVec2 size = ImVec2(size_arg.x == 0.f ? gui->content_avail().x : size_arg.x, size_arg.y);
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
    };
    static std::unordered_map<ImGuiID, button_state> button_states;
    auto& [current_state, do_move] = button_states[id];
    if (pressed) {
        current_state = pressed;
        do_move = true;
        reset_tween<float>({ id, "button_alpha" });
        reset_tween<float>({ id, "button_anim" });
    }

    const float alpha = tween({ id, "button_alpha" }, 0.f, 1.f, easing::QuadEaseInOut, 250ms, current_state);
    const float anim = tween({ id, "button_anim" }, 0.f, 1.f, easing::QuadEaseInOut, 250ms, do_move);
    const float hover = tween({ id, "button_hover" }, 0.f, 1.f, easing::QuadEaseInOut, 150ms, hovered);
    if (anim == 1.f) {
        current_state = false;
    }
    if (alpha == 0.f)
    {
        do_move = false;
    }

    // base body — darker card that lifts slightly on hover
    draw->rect_filled(drawlist, bb.Min, bb.Max, gui->u32(col->gray_2, 0.40f + 0.20f * hover), rect_rounding);
    // subtle accent wash on hover
    draw->rect_filled(drawlist, bb.Min, bb.Max, gui->u32(col->primary, 0.04f + 0.06f * hover), rect_rounding);
    // click fill — primary sweeps in from the centre
    draw->rect_filled(drawlist, ImVec2(bb.GetCenter().x - bb.GetWidth() * 0.5f * anim, bb.Min.y), ImVec2(bb.GetCenter().x + bb.GetWidth() * 0.5f * anim, bb.Max.y), gui->u32(col->primary, alpha), rect_rounding);
    // border brightens on hover
    draw->rect(drawlist, bb.Min, bb.Max, gui->u32(col->primary, 0.10f + 0.22f * hover), rect_rounding, scale(1.f));
    // left accent bar grows on hover
    const float accent_h = bb.GetHeight() * 0.5f * hover;
    if (accent_h > 0.5f) {
        const ImVec2 bc = bb.GetCenter();
        draw->rect_filled(drawlist, ImVec2(bb.Min.x, bc.y - accent_h * 0.5f), ImVec2(bb.Min.x + scale(2.5f), bc.y + accent_h * 0.5f), gui->u32(col->primary, hover), scale(2.f));
    }

    draw->text_clipped(drawlist, font->get("semibold13"),bb.Min, bb.Max, gui->u32(col->text, 1.f - alpha), gui->hide_text_hint(label), nullptr, nullptr, { 0.5f, 0.5f });
    draw->text_clipped(drawlist, font->get("semibold13"), bb.Min, bb.Max, gui->u32(col->black, alpha), gui->hide_text_hint(label), nullptr, nullptr, { 0.5f, 0.5f });

    return pressed;
}