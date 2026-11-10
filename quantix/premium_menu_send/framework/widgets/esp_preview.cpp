#include "../headers/widgets.h"
#include "../headers/includes.h"
#include "../headers/functions.h"
#include "../headers/animations.h"
#include "../settings/colors.h"
#include "../settings/elements.h"
#include "../settings/variables.h"
#include "../headers/draw.h"
#include "../headers/font.h"
#include "../image_loaders/image_dx11.h"
#include "../data/icons.h"

void init_esp_preview(esp_preview_state& state)
{
    const float widget_w = scale(230);
    const float widget_h = scale(420);
    const float char_w = scale(180);
    const float char_h = scale(306);
    const float char_x = (widget_w - char_w) * 0.5f;
    const float char_y = scale(35);
    const float head_top = char_y;
    const float feet_bottom = char_y + char_h;
    const float char_left = char_x;
    const float char_right = char_x + char_w;
    
    if (state.bar_places[0].empty())
    {
        state.bar_places = {
            std::vector<esp_preview_state::bar_place>{},
            std::vector<esp_preview_state::bar_place>{
                {.rect = ImRect(ImVec2(char_left - scale(10), head_top), ImVec2(char_left - scale(4), feet_bottom)) },
                {.rect = ImRect(ImVec2(char_left - scale(18), head_top), ImVec2(char_left - scale(12), feet_bottom)) }
            },
            std::vector<esp_preview_state::bar_place>{
                {.rect = ImRect(ImVec2(char_right + scale(4), head_top), ImVec2(char_right + scale(10), feet_bottom)) },
                {.rect = ImRect(ImVec2(char_right + scale(12), head_top), ImVec2(char_right + scale(18), feet_bottom)) }
            },
            std::vector<esp_preview_state::bar_place>{
                {.rect = ImRect(ImVec2(char_left, feet_bottom + scale(4)), ImVec2(char_right, feet_bottom + scale(10))), .occupied = true },
                {.rect = ImRect(ImVec2(char_left, feet_bottom + scale(12)), ImVec2(char_right, feet_bottom + scale(18))) }
            }
        };
    }
    
    if (state.text_places[0].empty())
    {
        state.text_places = {
            std::vector<esp_preview_state::text_place>{{.rect = ImRect(ImVec2(char_left, head_top - scale(22)), ImVec2(char_right, head_top - scale(6))), .occupied = true }},
            std::vector<esp_preview_state::text_place>{{.rect = ImRect(ImVec2(0, 0), ImVec2(0, 0)) }},
            std::vector<esp_preview_state::text_place>{{.rect = ImRect(ImVec2(0, 0), ImVec2(0, 0)) }},
            std::vector<esp_preview_state::text_place>{{.rect = ImRect(ImVec2(char_left, feet_bottom + scale(22)), ImVec2(char_right, feet_bottom + scale(38))), .occupied = true }}
        };
    }
    
    if (state.text_items.empty())
    {
        state.text_items = {
            esp_preview_state::text_item{.data = "Username", .font_id = "medium12", .rect = state.text_places[TOP][0].rect, .color = ImVec4(1, 1, 1, 1), .displacement_type = TOP },
            esp_preview_state::text_item{.data = "[100m]", .font_id = "medium12", .rect = state.text_places[BOTTOM][0].rect, .color = ImVec4(1, 1, 1, 1), .displacement_type = BOTTOM },
            esp_preview_state::text_item{.data = "Weapon", .font_id = "medium12", .rect = state.text_places[TOP][0].rect, .color = ImVec4(1, 1, 1, 1), .displacement_type = TOP }
        };
    }
    
    if (state.bar_items.empty())
    {
        state.bar_items = {
            {.size = state.bar_places[BOTTOM][0].rect.GetSize(), .pos = state.bar_places[BOTTOM][0].rect.Min, .color = state.health_bar_color, .enabled = state.health_bar, .displacement_type = BOTTOM},
            {.size = state.bar_places[BOTTOM][1].rect.GetSize(), .pos = state.bar_places[BOTTOM][1].rect.Min, .color = state.armor_bar_color, .enabled = state.armor_bar, .displacement_type = BOTTOM}
        };
    }
    
    state.bounding_box = var->cfg.get<bool>("esp_box");
    state.health_bar = var->cfg.get<bool>("esp_health_bar");
    state.armor_bar = var->cfg.get<bool>("esp_armor_bar");
    
    if (state.bar_items.size() >= 2) { state.bar_items[0].enabled = state.health_bar; state.bar_items[1].enabled = state.armor_bar; }
    if (state.text_items.size() >= 3) { state.text_items[0].enabled = var->cfg.get<bool>("esp_name"); state.text_items[1].enabled = var->cfg.get<bool>("esp_distance"); state.text_items[2].enabled = var->cfg.get<bool>("esp_weapon_name"); }
}


bool c_widgets::esp_preview(const char* str_id, esp_preview_state& state)
{
    ImGuiWindow* window = gui->get_window();
    ImDrawList* drawlist = window->DrawList;
    if (window->SkipItems) return false;

    const ImVec2 size = scale(230, 420);
    bool value_changed = false;
    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(str_id);
    const ImVec2 pos = window->DC.CursorPos;

    static std::unordered_map<ImGuiID, ImVec2> config_entries_states;
    auto [it, inserted] = config_entries_states.try_emplace(id, window->Pos);
    ImVec2& last_window_pos = it->second;

    const ImRect bb(pos, pos + size);
    gui->item_size(bb);

    const float char_w = scale(180);
    const float char_h = scale(306);
    const float char_x = (size.x - char_w) * 0.5f;
    const float char_y = scale(35);
    
    const ImVec2 char_pos = ImVec2(char_x, char_y);
    const ImVec2 char_size = ImVec2(char_w, char_h);
    
    const float head_top = char_y;
    const float feet_bottom = char_y + char_h;
    const float char_left = char_x;
    const float char_right = char_x + char_w;
    
    // Draw character image
    if (image->get("character"))
    {
        const ImVec2 img_min = pos + char_pos;
        const ImVec2 img_max = img_min + char_size;
        drawlist->AddImage(image->get("character"), img_min, img_max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, (int)(230 * ImGui::GetStyle().Alpha)));
    }
    
    // Draw skeleton image if enabled - centered on character with proper size
    if (state.skeleton && image->get("skeleton"))
    {
        // Skeleton original size is 144x243, scale it proportionally to fit character
        const float skel_scale = char_h / 243.f * 0.75f;  // 85% of character height
        const float skel_w = 144.f * skel_scale;
        const float skel_h = 243.f * skel_scale;
        // Center skeleton on character
        const float skel_x = char_x + (char_w - skel_w) * 0.5f;
        const float skel_y = char_y + (char_h - skel_h) * 0.5f + scale(12);
        const ImVec2 skel_min = pos + ImVec2(skel_x, skel_y);
        const ImVec2 skel_max = skel_min + ImVec2(skel_w, skel_h);
        // Get skeleton color from config and apply as tint
        const ImVec4 skeleton_color = var->cfg.get<ImVec4>("esp_skeleton_color");
        const ImU32 skel_tint = IM_COL32(
            (int)(skeleton_color.x * 255),
            (int)(skeleton_color.y * 255),
            (int)(skeleton_color.z * 255),
            (int)(skeleton_color.w * 255 * ImGui::GetStyle().Alpha)
        );
        drawlist->AddImage(image->get("skeleton"), skel_min, skel_max, ImVec2(0, 0), ImVec2(1, 1), skel_tint);
    }
    
    // Draw bounding box around character
    if (state.bounding_box)
    {
        // Expand box by 5% on each side for better visibility
        const float expand = scale(8);
        const ImVec2 box_min = pos + char_pos - ImVec2(expand, expand);
        const ImVec2 box_max = pos + char_pos + char_size + ImVec2(expand, expand);
        const ImU32 box_col = gui->u32(state.bounding_box_color);
        const float thickness = scale(1.5f);
        
        if (state.box_type == 0)
        {
            // Normal box - full rectangle
            draw->rect(drawlist, box_min, box_max, box_col, 0.f, thickness);
        }
        else
        {
            // Dashed box - corners only
            const float corner_len = char_w * 0.2f;  // 20% of width for corner length
            
            // Top-left corner
            drawlist->AddLine(box_min, ImVec2(box_min.x + corner_len, box_min.y), box_col, thickness);
            drawlist->AddLine(box_min, ImVec2(box_min.x, box_min.y + corner_len), box_col, thickness);
            
            // Top-right corner
            drawlist->AddLine(ImVec2(box_max.x, box_min.y), ImVec2(box_max.x - corner_len, box_min.y), box_col, thickness);
            drawlist->AddLine(ImVec2(box_max.x, box_min.y), ImVec2(box_max.x, box_min.y + corner_len), box_col, thickness);
            
            // Bottom-left corner
            drawlist->AddLine(ImVec2(box_min.x, box_max.y), ImVec2(box_min.x + corner_len, box_max.y), box_col, thickness);
            drawlist->AddLine(ImVec2(box_min.x, box_max.y), ImVec2(box_min.x, box_max.y - corner_len), box_col, thickness);
            
            // Bottom-right corner
            drawlist->AddLine(box_max, ImVec2(box_max.x - corner_len, box_max.y), box_col, thickness);
            drawlist->AddLine(box_max, ImVec2(box_max.x, box_max.y - corner_len), box_col, thickness);
        }
    }

    // Draw head dot preview (hollow circle on the character's head)
    if (var->cfg.get<bool>("esp_head_dot"))
    {
        const ImVec2 head_center = pos + ImVec2(char_x + char_w * 0.5f, head_top + scale(24));
        const float hd_r = scale(11);
        const ImU32 hd_col = gui->u32(var->cfg.get<ImVec4>("esp_head_circle_color"));
        drawlist->AddCircle(head_center, hd_r + 1.f, IM_COL32(0, 0, 0, 160), 28, scale(2.2f));
        drawlist->AddCircle(head_center, hd_r, hd_col, 28, scale(1.6f));
    }

    constexpr auto MAX_AVAILABLE_PLACES = 4;
    const ImVec2 mouse_pos = g.IO.MousePos;
    const bool mouse_clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    const bool mouse_released = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
    const ImVec2 bar_padding = scale(1, 1);

    const float places_appear_anim = tween({ id, "places_appear" }, 0.f, 1.f, easing::QuadEaseOut, 200ms, state.dragged_item.type != esp_preview_state::dragged_item_info::dragged_type::none);
    const float pulse = 0.25f + 0.25f * ImMin(1.f, static_cast<float>(std::sin(GImGui->Time * 7.f) * GImGui->IO.DeltaTime * 75.f));
    const ImVec4 places_color = ImVec4(col->primary.x, col->primary.y, col->primary.z, places_appear_anim * pulse);

    const bool is_dragged_bar = state.dragged_item.type == esp_preview_state::dragged_item_info::dragged_type::bar;
    const bool is_dragged_text = state.dragged_item.type == esp_preview_state::dragged_item_info::dragged_type::text;

    // Count bars per direction first for proper stacking
    int top_bar_total = 0, bottom_bar_total = 0, left_bar_total = 0, right_bar_total = 0;
    for (auto& bar : state.bar_items)
    {
        if (!bar.enabled) continue;
        if (bar.displacement_type == TOP) top_bar_total++;
        else if (bar.displacement_type == BOTTOM) bottom_bar_total++;
        else if (bar.displacement_type == LEFT) left_bar_total++;
        else if (bar.displacement_type == RIGHT) right_bar_total++;
    }

    int top_bars_drawn = 0, bottom_bars_drawn = 0, left_bars_drawn = 0, right_bars_drawn = 0;
    const float bar_h = scale(6);
    const float bar_gap = scale(2);

    // Draw bars
    size_t bar_idx = 0;
    for (auto& bar : state.bar_items)
    {
        if (!bar.enabled) { bar_idx++; continue; }
        ImVec2 bar_pos = bar.pos;
        ImVec2 bar_size = bar.size;
        
        // Calculate position based on displacement_type, not stored pos
        if (bar.displacement_type == TOP) { bar_pos.x = char_left; bar_pos.y = head_top - scale(4) - bar_h - (top_bars_drawn * (bar_h + bar_gap)); bar_size.x = char_w; bar_size.y = bar_h; top_bars_drawn++; }
        else if (bar.displacement_type == BOTTOM) { bar_pos.x = char_left; bar_pos.y = feet_bottom + scale(4) + (bottom_bars_drawn * (bar_h + bar_gap)); bar_size.x = char_w; bar_size.y = bar_h; bottom_bars_drawn++; }
        else if (bar.displacement_type == LEFT) { bar_pos.x = char_left - scale(4) - bar_h - (left_bars_drawn * (bar_h + bar_gap)); bar_pos.y = head_top; bar_size.x = bar_h; bar_size.y = char_h; left_bars_drawn++; }
        else if (bar.displacement_type == RIGHT) { bar_pos.x = char_right + scale(4) + (right_bars_drawn * (bar_h + bar_gap)); bar_pos.y = head_top; bar_size.x = bar_h; bar_size.y = char_h; right_bars_drawn++; }

        const ImRect bar_bb(pos + bar_pos, pos + bar_pos + bar_size);
        draw->rect_filled(drawlist, bar_bb.Min, bar_bb.Max, gui->u32(col->bar_bg), 100.f);
        draw->rect_filled(drawlist, bar_bb.Min + bar_padding, bar_bb.Max - bar_padding, gui->u32(bar.color), 100.f);

        if (mouse_clicked && bar_bb.Contains(mouse_pos)) { state.start_dragging_place = ImRect(bar.pos, bar.pos + bar.size); state.dragged_item = { .type = esp_preview_state::dragged_item_info::bar, .index = bar_idx }; }
        bar_idx++;
    }

    // Count texts per direction for proper stacking
    int top_text_count = 0, bottom_text_count = 0;
    const float text_line_height = scale(16);
    for (auto& text : state.text_items) { if (!text.enabled) continue; if (text.displacement_type == TOP) top_text_count++; else if (text.displacement_type == BOTTOM) bottom_text_count++; }
    
    // Calculate bar offset for text positioning (bars take space)
    float top_bar_offset = top_bar_total > 0 ? (top_bar_total * (bar_h + bar_gap)) : 0;
    float bottom_bar_offset = bottom_bar_total > 0 ? (bottom_bar_total * (bar_h + bar_gap)) : 0;
    
    // Preview offset: when dragging a bar, immediately add offset for potential placement
    // This makes text move down as soon as user starts dragging, showing preview of final result
    if (is_dragged_bar)
    {
        auto& dragged_bar = state.bar_items[state.dragged_item.index];
        
        // If dragged bar is NOT in BOTTOM, add preview offset to bottom (assuming it might go there)
        if (dragged_bar.displacement_type != BOTTOM)
        {
            bottom_bar_offset += (bar_h + bar_gap) + scale(4);
        }
        
        // If dragged bar is NOT in TOP, add preview offset to top (assuming it might go there)
        if (dragged_bar.displacement_type != TOP)
        {
            top_bar_offset += (bar_h + bar_gap);
        }
    }
    
    int top_drawn = 0, bottom_drawn = 0;
    size_t text_idx = 0;
    for (auto& text : state.text_items)
    {
        if (!text.enabled) { text_idx++; continue; }
        ImVec2 text_pos = text.rect.Min;
        ImVec2 text_size = ImVec2(char_w, scale(16));
        
        // Calculate position based on displacement_type with proper stacking
        if (text.displacement_type == TOP) { 
            text_pos.x = char_left; 
            text_pos.y = head_top - scale(14) - top_bar_offset - (top_drawn * text_line_height) - scale(8); 
            text_size.x = char_w; 
            top_drawn++; 
        }
        else if (text.displacement_type == BOTTOM) { 
            text_pos.x = char_left; 
            text_pos.y = feet_bottom + scale(8) + bottom_bar_offset + (bottom_drawn * text_line_height); 
            text_size.x = char_w; 
            bottom_drawn++; 
        }
        
        const ImRect text_bb(pos + text_pos, pos + text_pos + text_size);
        draw->text_clipped(drawlist, font->get(text.font_id.data()), text_bb.Min - text_bb.GetSize(), text_bb.Max + text_bb.GetSize(), gui->u32(text.color), text.data.c_str(), nullptr, nullptr, {0.5f, 0.5f}, nullptr);

        if (mouse_clicked && text_bb.Contains(mouse_pos)) { state.start_dragging_place = text.rect; state.dragged_item = { .type = esp_preview_state::dragged_item_info::text, .index = text_idx }; }
        text_idx++;
    }

    // Bar dragging logic
    if (is_dragged_bar)
    {
        auto& dragged_item = state.bar_items[state.dragged_item.index];
        bool is_any_place_hovered = false;
        std::optional<std::reference_wrapper<esp_preview_state::bar_place>> hovered_place_ref;
        int hovered_place_direction = -1;

        for (size_t i = 0; i < MAX_AVAILABLE_PLACES; i++)
        {
            if (i == TOP) continue;
            for (size_t j = 0; j < state.bar_places[i].size(); j++)
            {
                auto& bar_place = state.bar_places[i][j];
                const ImRect rect_real(pos + bar_place.rect.Min, pos + bar_place.rect.Max);
                const bool mouse_hovered = rect_real.Contains(mouse_pos);

                if (mouse_hovered) { is_any_place_hovered = true; hovered_place_ref = bar_place; hovered_place_direction = static_cast<int>(i); }

                // Always show available places - multiple bars can be in same direction
                {
                    ImRect rect_bb;
                    switch (i)
                    {
                    case BOTTOM: rect_bb = ImRect(ImVec2(rect_real.GetCenter().x - rect_real.GetWidth() * 0.5f * places_appear_anim, rect_real.Min.y), ImVec2(rect_real.GetCenter().x + rect_real.GetWidth() * 0.5f * places_appear_anim, rect_real.Max.y)); break;
                    case LEFT: case RIGHT: rect_bb = ImRect(ImVec2(rect_real.Min.x, rect_real.GetCenter().y - rect_real.GetHeight() * 0.5f * places_appear_anim), ImVec2(rect_real.Max.x, rect_real.GetCenter().y + rect_real.GetHeight() * 0.5f * places_appear_anim)); break;
                    }
                    draw->rect_filled(drawlist, rect_bb.Min, rect_bb.Max, gui->u32(places_color), 100.f);
                }
            }
        }

        if (mouse_released)
        {
            if (hovered_place_ref && hovered_place_direction >= 0 && hovered_place_direction != TOP)
            {
                auto& bar_place = hovered_place_ref.value().get();
                bool is_current_slot = (bar_place.rect.Min.x == dragged_item.pos.x && bar_place.rect.Min.y == dragged_item.pos.y);
                
                // Allow moving to any direction (multiple bars can stack in same direction)
                {
                    dragged_item.displacement_type = static_cast<displacement>(hovered_place_direction);
                    dragged_item.pos = bar_place.rect.Min;
                    dragged_item.size = bar_place.rect.GetSize();
                    value_changed = true;
                }
            }
            state.dragged_item.type = esp_preview_state::dragged_item_info::dragged_type::none;
        }
    }

    // Text dragging logic
    if (is_dragged_text)
    {
        auto& dragged_item = state.text_items[state.dragged_item.index];
        bool is_any_place_hovered = false;
        std::optional<std::reference_wrapper<esp_preview_state::text_place>> hovered_place_ref;
        int hovered_place_direction = -1;

        // Top slot: texts are drawn at head_top - scale(22) - top_bar_offset, height is scale(16)
        const float top_text_y = head_top - scale(22) - top_bar_offset;
        const ImRect top_slot_rect(ImVec2(pos.x + char_left, pos.y + top_text_y), ImVec2(pos.x + char_right, pos.y + top_text_y + scale(16)));
        
        // Bottom slot: texts are drawn at feet_bottom + scale(8) + bottom_bar_offset, height is scale(16)
        const float bottom_text_y = feet_bottom + scale(8) + bottom_bar_offset;
        const ImRect bottom_slot_rect(ImVec2(pos.x + char_left, pos.y + bottom_text_y), ImVec2(pos.x + char_right, pos.y + bottom_text_y + scale(16)));

        if (top_slot_rect.Contains(mouse_pos)) { is_any_place_hovered = true; hovered_place_ref = state.text_places[TOP][0]; hovered_place_direction = TOP; }
        else if (bottom_slot_rect.Contains(mouse_pos)) { is_any_place_hovered = true; hovered_place_ref = state.text_places[BOTTOM][0]; hovered_place_direction = BOTTOM; }

        { ImRect rect_bb = ImRect(ImVec2(top_slot_rect.GetCenter().x - top_slot_rect.GetWidth() * 0.5f * places_appear_anim, top_slot_rect.Min.y), ImVec2(top_slot_rect.GetCenter().x + top_slot_rect.GetWidth() * 0.5f * places_appear_anim, top_slot_rect.Max.y)); draw->rect_filled(drawlist, rect_bb.Min, rect_bb.Max, gui->u32(places_color)); }
        { ImRect rect_bb = ImRect(ImVec2(bottom_slot_rect.GetCenter().x - bottom_slot_rect.GetWidth() * 0.5f * places_appear_anim, bottom_slot_rect.Min.y), ImVec2(bottom_slot_rect.GetCenter().x + bottom_slot_rect.GetWidth() * 0.5f * places_appear_anim, bottom_slot_rect.Max.y)); draw->rect_filled(drawlist, rect_bb.Min, rect_bb.Max, gui->u32(places_color)); }

        if (state.swapped)
        {
            bool cursor_is_still_on_swapped_place = false;
            if (hovered_place_ref) { if (hovered_place_ref.value().get().rect.Min == dragged_item.rect.Min) { cursor_is_still_on_swapped_place = true; } }
            if (!cursor_is_still_on_swapped_place) { auto& swapped_item = state.text_items[state.swapped_item.index]; std::swap(swapped_item.rect, dragged_item.rect); state.swapped = false; state.swapped_item.type = esp_preview_state::dragged_item_info::none; }
        }

        if (hovered_place_ref && hovered_place_direction >= 0)
        {
            auto& text_place = hovered_place_ref.value().get();
            if (hovered_place_direction == TOP || hovered_place_direction == BOTTOM) { dragged_item.displacement_type = static_cast<displacement>(hovered_place_direction); dragged_item.rect = text_place.rect; }
        }

        if (mouse_released) { value_changed = true; state.swapped_item.type = esp_preview_state::dragged_item_info::dragged_type::none; state.dragged_item.type = esp_preview_state::dragged_item_info::dragged_type::none; state.swapped = false; }
    }

    if (state.dragged_item.type != esp_preview_state::dragged_item_info::dragged_type::none) { ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); }
    last_window_pos = window->Pos;
    return value_changed;
}
