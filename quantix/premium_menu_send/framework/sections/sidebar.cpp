#include "../headers/sections.h"

// Flyout menu state
static float g_flyout_anim = 0.0f;
static float g_flyout_alpha = 0.0f;
static pages g_flyout_page = pages::AIM_ASSISTANCE;
static bool g_flyout_open = true;
static pages g_last_target_page = pages::AIM_ASSISTANCE;
static float g_slide_anim = 1.0f; // 0 = hidden behind menu, 1 = fully visible
static bool g_sliding_out = false; // true = going behind menu, false = coming back
static pages g_pending_page = pages::AIM_ASSISTANCE; // page to switch to after slide out
static std::map<std::string, float> g_flyout_item_anims;

// Get subtab names for each page
static std::vector<const char*> get_page_subtabs(pages page) {
    switch (page) {
    case pages::AIM_ASSISTANCE: return {"Aimbot", "Silent", "Trigger"};
    case pages::PLAYER: return {"General", "Freecam", "Players"};
    case pages::VEHICLE: return {"Vehicle", "Repair", "Vehicles"};
    case pages::ESP: return {"Visuals", "Color", "Friend"};
    case pages::RADAR: return {"Radar", "Colors", "Around"};
    case pages::WEAPON: return {"Modifiers", "Weapon"};
    case pages::GLOBAL_SETTINGS: return {"Settings", "Config"};
    default: return {};
    }
}

static bool page_has_subtabs(pages page) {
    return !get_page_subtabs(page).empty();
}

// Render sidebar content
void sections::render_sidebar_content(ImDrawList* drawlist, const ImVec2& pos, const ImVec2& size) {
	const float sidebar_width = size.x;
	const float sidebar_height = size.y;
	const float icon_size = scale(26.0f);
	const float icon_spacing = scale(6.0f);
	const float padding_top = scale(8.0f);
	const float padding_x = (sidebar_width - icon_size) * 0.5f;

	// Draw logo at top
	const float logo_size = scale(50.0f);
	const float logo_x = pos.x + (sidebar_width - logo_size) * 0.5f;
	const float logo_y = pos.y + padding_top;
	if (image && image->get("logo"))
	{
		drawlist->AddImage(
			image->get("logo"), 
			ImVec2(logo_x, logo_y), 
			ImVec2(logo_x + logo_size, logo_y + logo_size), 
			ImVec2(0, 0), ImVec2(1, 1), 
			IM_COL32(255, 255, 255, 255)
		);
	}

	// Separator line after logo
	const float sep_y = logo_y + logo_size + scale(2.0f);
	drawlist->AddLine(
		ImVec2(pos.x + scale(10.0f), sep_y),
		ImVec2(pos.x + sidebar_width - scale(10.0f), sep_y),
		gui->u32(col->gray_3, 0.3f),
		1.0f
	);

	// Icon button helper lambda
	const float icons_start_y = sep_y + scale(12.0f);
	int icon_index = 0;
	
	static std::map<pages, float> icon_y_positions;
	
	auto icon_button = [&](const char* icon, pages page, const char* tooltip = nullptr) -> bool {
		const float icon_y = icons_start_y + icon_index * (icon_size + icon_spacing);
		const ImVec2 icon_pos = ImVec2(pos.x + padding_x, icon_y);
		const ImRect icon_bb(icon_pos, icon_pos + ImVec2(icon_size, icon_size));
		
		bool hovered = icon_bb.Contains(ImGui::GetIO().MousePos);
		bool pressed = hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
		bool is_active = var->window.current_page == page;
		
		static std::map<pages, float> hover_anims;
		static std::map<pages, float> active_anims;
		
		float& hover_anim = hover_anims[page];
		float& active_anim = active_anims[page];
		
		float anim_speed = 10.0f * ImGui::GetIO().DeltaTime;
		
		if (hovered && !is_active) {
			hover_anim = ImMin(hover_anim + anim_speed, 1.0f);
		} else {
			hover_anim = ImMax(hover_anim - anim_speed, 0.0f);
		}
		
		if (is_active) {
			active_anim = ImMin(active_anim + anim_speed, 1.0f);
		} else {
			active_anim = ImMax(active_anim - anim_speed, 0.0f);
		}
		
		const ImVec2 center = icon_pos + ImVec2(icon_size * 0.5f, icon_size * 0.5f);
		const float radius = icon_size * 0.5f;
		
		// Active indicator - left accent bar
		if (active_anim > 0.01f) {
			float bar_height = icon_size * 0.6f * active_anim;
			float bar_y = center.y - bar_height * 0.5f;
			drawlist->AddRectFilled(
				ImVec2(pos.x + scale(2.0f), bar_y),
				ImVec2(pos.x + scale(4.0f), bar_y + bar_height),
				gui->u32(col->primary, active_anim),
				scale(2.0f)
			);
		}
		
		// Background circle
		if (active_anim > 0.01f) {
			draw->circle_filled(drawlist, center, radius * (0.9f + 0.1f * active_anim), gui->u32(col->primary, 0.2f * active_anim));
		}
		if (hover_anim > 0.01f && active_anim < 0.5f) {
			draw->circle_filled(drawlist, center, radius * 0.9f, gui->u32(col->gray_3, 0.3f * hover_anim));
		}
		
		float alpha = 0.5f + 0.3f * hover_anim + 0.5f * active_anim;
		alpha = ImMin(alpha, 1.0f);
		
		draw->text_clipped(
			drawlist, 
			font->get("icons20"), 
			icon_pos, 
			icon_pos + ImVec2(icon_size, icon_size), 
			gui->u32(col->text, alpha), 
			icon, 
			nullptr, nullptr, 
			{0.5f, 0.5f}
		);
		
		icon_y_positions[page] = icon_y;
		
		// Tooltip
		if (hovered && tooltip && g_flyout_page != page) {
			ImDrawList* fg_drawlist = ImGui::GetForegroundDrawList();
			ImVec2 tooltip_pos = ImVec2(pos.x + sidebar_width + scale(5.0f), icon_pos.y + icon_size * 0.5f - scale(10.0f));
			ImVec2 text_size = ImGui::CalcTextSize(tooltip);
			ImVec2 tooltip_size = text_size + ImVec2(scale(14.0f), scale(8.0f));
			
			fg_drawlist->AddRectFilled(tooltip_pos, tooltip_pos + tooltip_size, IM_COL32(25, 25, 25, 245), scale(4.0f));
			fg_drawlist->AddRect(tooltip_pos, tooltip_pos + tooltip_size, IM_COL32(50, 50, 50, 200), scale(4.0f));
			fg_drawlist->AddText(tooltip_pos + ImVec2(scale(7.0f), scale(4.0f)), IM_COL32(255, 255, 255, 255), tooltip);
		}
		
		icon_index++;
		return pressed;
	};

	// Navigation icons
	if (icon_button(ICON_OCTAGON_PLUS, pages::AIM_ASSISTANCE, "Aim Assist")) {
		var->window.target_page = pages::AIM_ASSISTANCE;
	}
	
	if (icon_button(ICON_EYE, pages::ESP, "ESP")) {
		var->window.target_page = pages::ESP;
	}
	
	if (icon_button(ICON_USER, pages::PLAYER, "Player")) {
		var->window.target_page = pages::PLAYER;
	}
	
	if (icon_button(ICON_ARROW_MERGED, pages::VEHICLE, "Vehicle")) {
		var->window.target_page = pages::VEHICLE;
	}
	
	if (icon_button(ICON_LINE_HEIGHT, pages::WEAPON, "Weapon")) {
		var->window.target_page = pages::WEAPON;
	}
	
	if (icon_button(ICON_PLANET, pages::RADAR, "Radar")) {
		var->window.target_page = pages::RADAR;
	}
	
	// Detect page change - start slide out animation
	if (var->window.target_page != g_last_target_page && !g_sliding_out) {
		if (page_has_subtabs(var->window.target_page)) {
			g_sliding_out = true;
			g_pending_page = var->window.target_page;
		}
		g_last_target_page = var->window.target_page;
	}
	
	// Settings at bottom
	const float settings_y = pos.y + sidebar_height - icon_size - scale(15.0f);
	icon_y_positions[pages::GLOBAL_SETTINGS] = settings_y;
	
	// Separator before settings
	const float sep2_y = settings_y - scale(10.0f);
	drawlist->AddLine(
		ImVec2(pos.x + scale(10.0f), sep2_y),
		ImVec2(pos.x + sidebar_width - scale(10.0f), sep2_y),
		gui->u32(col->gray_3, 0.3f),
		1.0f
	);
	
	// Settings button
	{
		const ImVec2 icon_pos = ImVec2(pos.x + padding_x, settings_y);
		const ImRect icon_bb(icon_pos, icon_pos + ImVec2(icon_size, icon_size));
		
		bool hovered = icon_bb.Contains(ImGui::GetIO().MousePos);
		bool pressed = hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
		bool is_active = var->window.current_page == pages::GLOBAL_SETTINGS;
		
		static float settings_hover_anim = 0.0f;
		static float settings_active_anim = 0.0f;
		
		float anim_speed = 10.0f * ImGui::GetIO().DeltaTime;
		
		if (hovered && !is_active) {
			settings_hover_anim = ImMin(settings_hover_anim + anim_speed, 1.0f);
		} else {
			settings_hover_anim = ImMax(settings_hover_anim - anim_speed, 0.0f);
		}
		
		if (is_active) {
			settings_active_anim = ImMin(settings_active_anim + anim_speed, 1.0f);
		} else {
			settings_active_anim = ImMax(settings_active_anim - anim_speed, 0.0f);
		}
		
		const ImVec2 center = icon_pos + ImVec2(icon_size * 0.5f, icon_size * 0.5f);
		const float radius = icon_size * 0.5f;
		
		if (settings_active_anim > 0.01f) {
			float bar_height = icon_size * 0.6f * settings_active_anim;
			float bar_y = center.y - bar_height * 0.5f;
			drawlist->AddRectFilled(
				ImVec2(pos.x + scale(2.0f), bar_y),
				ImVec2(pos.x + scale(4.0f), bar_y + bar_height),
				gui->u32(col->primary, settings_active_anim),
				scale(2.0f)
			);
		}
		
		if (settings_active_anim > 0.01f) {
			draw->circle_filled(drawlist, center, radius * (0.9f + 0.1f * settings_active_anim), gui->u32(col->primary, 0.2f * settings_active_anim));
		}
		if (settings_hover_anim > 0.01f && settings_active_anim < 0.5f) {
			draw->circle_filled(drawlist, center, radius * 0.9f, gui->u32(col->gray_3, 0.3f * settings_hover_anim));
		}
		
		float alpha = 0.5f + 0.3f * settings_hover_anim + 0.5f * settings_active_anim;
		alpha = ImMin(alpha, 1.0f);
		
		draw->text_clipped(
			drawlist, 
			font->get("icons20"), 
			icon_pos, 
			icon_pos + ImVec2(icon_size, icon_size), 
			gui->u32(col->text, alpha), 
			ICON_SETTINGS, 
			nullptr, nullptr, 
			{0.5f, 0.5f}
		);
		
		if (hovered && g_flyout_page != pages::GLOBAL_SETTINGS) {
			ImDrawList* fg_drawlist = ImGui::GetForegroundDrawList();
			const char* tooltip = "Settings";
			ImVec2 tooltip_pos = ImVec2(pos.x + sidebar_width + scale(5.0f), icon_pos.y + icon_size * 0.5f - scale(10.0f));
			ImVec2 text_size = ImGui::CalcTextSize(tooltip);
			ImVec2 tooltip_size = text_size + ImVec2(scale(14.0f), scale(8.0f));
			
			fg_drawlist->AddRectFilled(tooltip_pos, tooltip_pos + tooltip_size, IM_COL32(25, 25, 25, 245), scale(4.0f));
			fg_drawlist->AddRect(tooltip_pos, tooltip_pos + tooltip_size, IM_COL32(50, 50, 50, 200), scale(4.0f));
			fg_drawlist->AddText(tooltip_pos + ImVec2(scale(7.0f), scale(4.0f)), IM_COL32(255, 255, 255, 255), tooltip);
		}
		
		if (pressed) {
			var->window.target_page = pages::GLOBAL_SETTINGS;
		}
	}
	
	// ============== FLYOUT MENU ==============
	float delta = ImGui::GetIO().DeltaTime;
	float anim_speed = 12.0f * delta;
	
	// Slide animation logic
	if (g_sliding_out) {
		// Sliding out (going behind menu)
		g_slide_anim -= delta * 6.0f;
		if (g_slide_anim <= 0.0f) {
			g_slide_anim = 0.0f;
			g_sliding_out = false;
			// Switch to new page
			g_flyout_page = g_pending_page;
			g_flyout_item_anims.clear();
		}
	} else {
		// Sliding in (coming back)
		if (g_slide_anim < 1.0f) {
			g_slide_anim += delta * 6.0f;
			if (g_slide_anim > 1.0f) g_slide_anim = 1.0f;
		}
	}
	
	// Flyout dimensions
	const float flyout_width = scale(105.0f);
	const float flyout_item_height = scale(30.0f);
	const float flyout_padding = scale(8.0f);
	const float flyout_rounding = scale(8.0f);
	const float flyout_gap = scale(6.0f);
	
	// Get flyout position based on current page icon
	float flyout_icon_y = icons_start_y;
	if (icon_y_positions.count(g_flyout_page)) {
		flyout_icon_y = icon_y_positions[g_flyout_page];
	}
	
	auto subtabs = get_page_subtabs(g_flyout_page);
	float flyout_height = subtabs.size() * flyout_item_height + flyout_padding * 2;
	
	// Flyout position - LEFT of sidebar
	float flyout_x_right = pos.x - flyout_gap;
	float flyout_x_left = flyout_x_right - flyout_width;
	float flyout_y = flyout_icon_y + (icon_size - flyout_height) * 0.5f;
	
	// Clamp to menu bounds
	flyout_y = ImMax(flyout_y, pos.y + scale(5.0f));
	flyout_y = ImMin(flyout_y, pos.y + sidebar_height - flyout_height - scale(5.0f));
	
	// Apply slide animation - move flyout behind menu (to the right)
	float slide_offset = (1.0f - g_slide_anim) * (flyout_width + flyout_gap + scale(10.0f));
	float animated_x_left = flyout_x_left + slide_offset;
	float animated_x_right = flyout_x_right + slide_offset;
	
	// Flyout alpha animation
	g_flyout_anim = ImMin(g_flyout_anim + anim_speed * 1.5f, 1.0f);
	g_flyout_alpha = ImMin(g_flyout_alpha + anim_speed * 2.0f, 1.0f);
	
	// Draw flyout
	if (g_flyout_anim > 0.01f && g_slide_anim > 0.01f) {
		ImDrawList* fg_draw = ImGui::GetForegroundDrawList();
		ImVec2 mouse_pos = ImGui::GetIO().MousePos;
		
		ImVec2 draw_min = ImVec2(animated_x_left, flyout_y);
		ImVec2 draw_max = ImVec2(animated_x_right, flyout_y + flyout_height);
		
		// Clip to not draw over sidebar
		fg_draw->PushClipRect(ImVec2(0, 0), ImVec2(pos.x - flyout_gap * 0.5f, var->window.screen_size.y), true);
		
		// Shadow
		float shadow_alpha = g_flyout_alpha * g_slide_anim;
		for (int s = 1; s <= 4; s++) {
			int shadow_a = (int)((20 - s * 4) * shadow_alpha);
			fg_draw->AddRectFilled(
				ImVec2(draw_min.x - s * 2, draw_min.y + s),
				ImVec2(draw_max.x - s, draw_max.y + s * 2),
				IM_COL32(0, 0, 0, shadow_a),
				flyout_rounding
			);
		}
		
		// Background - same as menu
		int bg_alpha = (int)(255 * g_flyout_alpha * g_slide_anim);
		fg_draw->AddRectFilled(draw_min, draw_max, IM_COL32(13, 13, 13, bg_alpha), flyout_rounding);
		
		// Border
		int border_alpha = (int)(80 * g_flyout_alpha * g_slide_anim);
		fg_draw->AddRect(draw_min, draw_max, IM_COL32(60, 60, 65, border_alpha), flyout_rounding, 0, 1.0f);
		
		// Arrow
		if (g_flyout_alpha > 0.3f && g_slide_anim > 0.5f) {
			float arrow_alpha = ((g_flyout_alpha - 0.3f) / 0.7f) * g_slide_anim;
			float arrow_y = flyout_icon_y + icon_size * 0.5f;
			arrow_y = ImMax(arrow_y, flyout_y + scale(10.0f));
			arrow_y = ImMin(arrow_y, flyout_y + flyout_height - scale(10.0f));
			
			ImVec2 p1 = ImVec2(animated_x_right + scale(4.0f), arrow_y);
			ImVec2 p2 = ImVec2(animated_x_right - scale(2.0f), arrow_y - scale(6.0f));
			ImVec2 p3 = ImVec2(animated_x_right - scale(2.0f), arrow_y + scale(6.0f));
			
			fg_draw->AddTriangleFilled(p1, p2, p3, IM_COL32(13, 13, 13, (int)(255 * arrow_alpha)));
		}
		
		// Draw subtab items
		if (g_flyout_alpha > 0.1f && g_slide_anim > 0.1f) {
			ImFont* item_font = font->get("medium12");
			if (!item_font) item_font = ImGui::GetFont();
			
			float item_y = flyout_y + flyout_padding;
			float item_alpha = g_flyout_alpha * g_slide_anim;
			
			for (size_t i = 0; i < subtabs.size(); i++) {
				const char* label = subtabs[i];
				bool is_selected = (var->window.current_page == g_flyout_page && var->window.current_subtab == (int)i);
				
				float box_padding_y = scale(4.0f);
				
				ImVec2 item_min = ImVec2(animated_x_left + flyout_padding, item_y + box_padding_y);
				ImVec2 item_max = ImVec2(animated_x_right - flyout_padding, item_y + flyout_item_height - box_padding_y);
				
				ImVec2 item_hit_min = ImVec2(animated_x_left + flyout_padding, item_y);
				ImVec2 item_hit_max = ImVec2(animated_x_right - flyout_padding, item_y + flyout_item_height);
				
				bool item_hovered = g_slide_anim > 0.8f &&
				                    (mouse_pos.x >= item_hit_min.x && mouse_pos.x <= item_hit_max.x &&
				                     mouse_pos.y >= item_hit_min.y && mouse_pos.y <= item_hit_max.y);
				bool item_clicked = item_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
				
				// Hover animation
				std::string anim_key = "item_" + std::to_string((int)g_flyout_page) + "_" + std::to_string(i);
				float& item_anim = g_flyout_item_anims[anim_key];
				float item_target = (item_hovered || is_selected) ? 1.0f : 0.0f;
				item_anim += (item_target - item_anim) * anim_speed * 2.5f;
				
				// Item background
				if (item_anim > 0.01f) {
					int item_bg_alpha = (int)(40 * item_anim * item_alpha);
					fg_draw->AddRectFilled(item_min, item_max, IM_COL32(255, 255, 255, item_bg_alpha), scale(5.0f));
				}
				
				// Selected indicator bar
				if (is_selected) {
					float box_height = item_max.y - item_min.y;
					float bar_h = box_height * 0.35f;
					ImVec2 bar_min = ImVec2(item_max.x - scale(2.5f), item_min.y + (box_height - bar_h) * 0.5f);
					ImVec2 bar_max = ImVec2(item_max.x, bar_min.y + bar_h);
					fg_draw->AddRectFilled(bar_min, bar_max, IM_COL32(255, 255, 255, (int)(200 * item_alpha)), scale(1.0f));
				}
				
				// Text
				int text_gray = (int)(100 + 155 * item_anim);
				if (is_selected) text_gray = 255;
				int text_alpha = (int)(255 * item_alpha);
				
				ImVec2 text_size = item_font->CalcTextSizeA(item_font->FontSize, FLT_MAX, 0.0f, label);
				ImVec2 text_pos = ImVec2(item_hit_min.x + scale(10.0f), item_hit_min.y + (flyout_item_height - text_size.y) * 0.5f);
				
				fg_draw->AddText(item_font, item_font->FontSize, text_pos, IM_COL32(text_gray, text_gray, text_gray, text_alpha), label);
				
				// Handle click
				if (item_clicked) {
					var->window.target_subtab = (int)i;
					var->window.subtab_switched = true;
				}
				
				item_y += flyout_item_height;
			}
		}
		
		fg_draw->PopClipRect();
	}
}

// Keep old function for compatibility
void sections::render_sidebar() {
	ImDrawList* drawlist = gui->window_drawlist();
	const ImVec2 pos = gui->window_pos();
	const ImVec2 size = gui->window_size();
	render_sidebar_content(drawlist, pos, size);
}