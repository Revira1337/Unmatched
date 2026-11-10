#include "../headers/widgets.h"
#include "../headers/includes.h"
#include "../headers/functions.h"
#include "../headers/animations.h"
#include "../settings/colors.h"
#include "../headers/draw.h"
#include "../headers/font.h"
#include "../data/icons.h"

bool c_widgets::tab(const char* label, const char* icon, const ImVec2& size_arg, bool is_active)
{
    ImGuiWindow* window = gui->get_window();
    ImDrawList* drawlist = window->DrawList;
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(label);

    const ImVec2 size = ImVec2(size_arg.x == 0.f ? gui->content_avail().x : size_arg.x, size_arg.y == 0.f ? gui->content_avail().y : size_arg.y);
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, pos + size);
    gui->item_size(bb, var->style.frame_padding.y);
    if (!gui->item_add(bb, id))
        return false;

    bool hovered, held;
    bool pressed = gui->button_behavior(bb, id, &hovered, &held);

    const float alpha = tween({ id, "tab_alpha"}, 0.55f, 1.f, easing::QuadEaseInOut, 200ms, is_active);

    const float circle_radius = size.y * 0.5f;
    draw->circle_filled(drawlist, pos + ImVec2(circle_radius, circle_radius), circle_radius, gui->u32(col->gray_3, alpha));
    draw->circle(drawlist, pos + ImVec2(circle_radius, circle_radius), circle_radius, gui->u32(col->gray_5, alpha), 0, var->style.child_border_size);

    draw->text_clipped(drawlist, font->get("icons12"), pos, pos + ImVec2(size.y, size.y), gui->u32(col->text, alpha), icon, nullptr, nullptr, {0.5f, 0.5f});

    draw->text_clipped(drawlist, font->get("medium12"), pos + ImVec2(size.y + var->style.item_inner_spacing.x, 0), pos + size, gui->u32(col->text, alpha), gui->hide_text_hint(label), nullptr, nullptr, { 0.f, 0.5f });

    return pressed;
}