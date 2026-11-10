#include "../headers/functions.h"
#include "../headers/widgets.h"
#include "../headers/animations.h"
#include "../settings/colors.h"
#include "../headers/font.h"
#include "../headers/draw.h"
#include "../settings/elements.h"

c_notify::notification::notification(uint16_t id, const std::string& text, notify_type type) : id{ id }, text{ text }, type{ type },
    alpha_anim_id{ "notify_alpha", id },
    move_anim_id{ "notify_move", id }
    {};

void c_notify::add_notify(const std::string& text, notify_type type)
{
    const float notify_spacing = scale(NOTIFY_SPACING);
    const float notify_height = scale(NOTIFY_HEIGHT);

    for (auto& notify : notifications) {
        notify.pos.y += notify_height + notify_spacing;
    }

    notifications.emplace_back(current_max_id++, text, type);
}

void c_notify::process_notifies(const ImRect& work_rect, notify_anchor anchor)
{
    const ImVec2 work_rect_padding = scale(WORK_RECT_PADDING);
    const ImVec2 notify_padding = scale(NOTIFY_PADDING);
    const float notify_height = scale(NOTIFY_HEIGHT);

    ImVec2 current_notify_anchor_pos{ 0,0 };

    switch (anchor) {
        using enum notify_anchor;
    case topright:
        current_notify_anchor_pos = work_rect.GetTR() + ImVec2(-work_rect_padding.x, work_rect_padding.y);
        break;
    }

    const float delta_time = GImGui->IO.DeltaTime;

    uint16_t rendered_notifies_count = 0;

    for (auto notify_it = notifications.begin(); (notify_it != notifications.end()) && (rendered_notifies_count != MAX_VISIBLE_NOTIFIES);)
    {
        if (notify_it->destroy_next_frame)
        {
            reset_tween<ImVec2>(notify_it->get_move_anim_id());
            reset_tween<float>(notify_it->get_alpha_anim_id());
            notify_it = notifications.erase(notify_it);
            continue;
        }

        const float text_width = gui->text_size(font->get("medium13"), notify_it->text.data()).x;

        const float calculated_notify_width = text_width + notify_padding.x * 2.f;
        const ImVec2 notify_size = ImVec2(calculated_notify_width, notify_height);

        const float animated_pos_x = calculated_notify_width + work_rect_padding.x;

        if (!notify_it->expired)
        {
            notify_it->duration += static_cast<std::chrono::duration<float>>(delta_time);
        }
        if ((notify_it->duration >= NOTIFY_DURATION) || var->gui.dpi_changed)
        {
            notify_it->expired = true;
            notify_it->pos.x += animated_pos_x;
        }
        if (current_max_id >= MAX_VISIBLE_NOTIFIES) {
            current_max_id = 0;
        }

        if (notify_it->pos == ImVec2(0, 0))
        {
            notify_it->pos = current_notify_anchor_pos + ImVec2(-calculated_notify_width, 0);
        }

        const ImVec2 notify_pos = tween(notify_it->get_move_anim_id(), notify_it->pos + ImVec2(animated_pos_x, 0), notify_it->pos, easing::QuadEaseInOut, 300ms);

        const float notify_alpha = tween(notify_it->get_alpha_anim_id(), 0.f, 1.f, easing::QuadEaseInOut, 300ms, !notify_it->expired);
        if (notify_it->expired && notify_alpha == 0.f) {
            notify_it->destroy_next_frame = true;
        }

        const bool is_last_renderer_notify = ((rendered_notifies_count + 1) == MAX_VISIBLE_NOTIFIES) || ((notify_it + 1) == notifications.end());

        gui->push_var(style_var_alpha, notify_alpha);

        notify_it->render(notify_pos, notify_size, is_last_renderer_notify);
        
        gui->pop_var();

        ++notify_it;
        rendered_notifies_count++;
    }
}

void c_notify::notification::render(const ImVec2& pos, const ImVec2& size, bool focused_notify)
{
    const ImVec2 notify_padding = scale(NOTIFY_PADDING);
    const float progress = duration.count() / NOTIFY_DURATION.count();

    gui->push_color(style_col_popup_bg, gui->u32(col->black));

    gui->set_next_window_pos(pos);
    gui->set_next_window_size(size);
    gui->begin("notify_" + std::to_string(id), nullptr, window_flags_no_decoration | window_flags_no_nav | window_flags_no_saved_settings | window_flags_tooltip | window_flags_no_inputs);
    {
        ImDrawList* drawlist = gui->window_drawlist();
        
        std::string type_text;
        ImVec4 active_color;
        switch (type) {
            using enum notify_type;
        case success:
            active_color = col->green;
            type_text = "Success";
            break;
        case warning:
            active_color = col->orange;
            type_text = "Warning";
            break;
        case error:
            active_color = col->red;
            type_text = "Error";
            break;
        }

        const ImVec2 type_rect_padding = scale(6, 6);
        const ImVec2 type_text_size = gui->text_size(font->get("medium12"), type_text.c_str());
        const ImVec2 type_rect_size = type_rect_padding * 2.f + type_text_size;
        
        draw->rect_filled(drawlist, pos + notify_padding, pos + notify_padding + type_rect_size, gui->u32(active_color, 0.12f), 100.f);
        draw->rect(drawlist, pos + notify_padding, pos + notify_padding + type_rect_size, gui->u32(active_color, 0.5f), 100.f, 0, 0.5f);

        draw->text_clipped(drawlist, font->get("medium12"), pos + notify_padding, pos + notify_padding + type_rect_size, gui->u32(active_color), type_text.c_str(), nullptr, nullptr, { 0.5f, 0.5f });

        draw->text_clipped(drawlist, font->get("medium13"), pos + notify_padding, pos + size - notify_padding, gui->u32(col->text), text.c_str(), nullptr, nullptr, { 0.f, 1.f });
    }
    gui->end();
    gui->pop_color();
}