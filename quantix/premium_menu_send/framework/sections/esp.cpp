#include "../headers/sections.h"
#include "../../cheat/Main/Options.hpp"

namespace {
	void render_header(const char* icon, const char* name, const char* desc);
	void render_esp_tab();
	void render_esp_color_tab();
	void render_esp_friend_tab();
	void render_preview();
	
	// Helper functions to convert between displacement enum and ESP position values
	// displacement: TOP=0, LEFT=1, RIGHT=2, BOTTOM=3
	// ESP text positions: 0=Top, 1=Bottom (only 2 options)
	// ESP bar positions: 0=Top, 1=Left, 2=Right, 3=Bottom (matches displacement)
	
	displacement textPosToDisplacement(int pos) {
		// 0=Top -> TOP, 1=Bottom -> BOTTOM
		return (pos == 0) ? TOP : BOTTOM;
	}
	
	int displacementToTextPos(displacement d) {
		// TOP -> 0, BOTTOM -> 1, anything else -> 1 (bottom)
		return (d == TOP) ? 0 : 1;
	}
	
	displacement barPosToDisplacement(int pos) {
		// Direct mapping: 0=Top, 1=Left, 2=Right, 3=Bottom
		switch (pos) {
			case 0: return TOP;
			case 1: return LEFT;
			case 2: return RIGHT;
			case 3: return BOTTOM;
			default: return RIGHT;
		}
	}
	
	int displacementToBarPos(displacement d) {
		// Direct mapping
		switch (d) {
			case TOP: return 0;
			case LEFT: return 1;
			case RIGHT: return 2;
			case BOTTOM: return 3;
			default: return 2;
		}
	}
}

void sections::render_esp() {
	ImDrawList* drawlist = gui->window_drawlist();
	const ImVec2 pos = gui->window_pos();
	const ImVec2 size = gui->window_size();

	const ImVec2 padding = scale(elements->window.content_padding);
	const float header_height = scale(elements->content.header_height_no_subtabs);

	const float old_alpha = var->window.alpha;
	if (var->window.subtab_switched) {
		var->window.alpha = 1.f;
	}
	gui->push_var(style_var_alpha, var->window.alpha);

	render_header(ICON_EYE, "Visuals", "Visual settings and customization");

	gui->pop_var();
	var->window.alpha = old_alpha;

	gui->set_pos(ImVec2(0, header_height), pos_all);

	// Render content based on selected subtab
	switch (var->window.current_subtab) {
	case 0:
		render_esp_tab();
		render_preview();
		break;
	case 1:
		render_esp_color_tab();
		render_preview();
		break;
	case 2:
		render_esp_friend_tab();
		break;
	}
}

namespace {
	void render_header(const char* icon, const char* name, const char* desc) {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const ImVec2 padding = scale(elements->window.content_padding);
		const float header_icon_radius = scale(elements->content.header_icon_radius);
		const float header_desc_height = scale(elements->content.header_desc_height);
		const float header_height = scale(elements->content.header_height_no_subtabs);
		
		const float content_width = size.x - padding.x * 2.f;
		const float circle_text_spacing_x = scale(12);

		const ImVec2 top_animation_pos = ImVec2(0.f, header_height * (1.f - var->style.alpha));

		const ImVec2 content_start_pos = pos + padding;
		const ImRect header_rect(content_start_pos - top_animation_pos, content_start_pos - ImVec2(0.f, top_animation_pos.y) + ImVec2(content_width, header_desc_height));

		{
			const ImVec2 circle_center_pos = header_rect.Min + ImVec2(header_icon_radius, header_desc_height * 0.5f);
			draw->circle_filled(drawlist, circle_center_pos, header_icon_radius, gui->u32(col->gray_3));
			draw->circle(drawlist, circle_center_pos, header_icon_radius, gui->u32(col->gray_5), 0, var->style.child_border_size);

			draw->text_clipped(drawlist, font->get("icons16"), circle_center_pos - ImVec2(header_icon_radius, header_icon_radius), circle_center_pos + ImVec2(header_icon_radius, header_icon_radius), gui->u32(col->text), icon, nullptr, nullptr, { 0.5f, 0.5f });

			draw->text(drawlist, font->get("medium13"), header_rect.Min + ImVec2(header_icon_radius * 2.f + circle_text_spacing_x, 0), gui->u32(col->text), name);
			draw->text_clipped(drawlist, font->get("medium12"), header_rect.Min + ImVec2(header_icon_radius * 2.f + circle_text_spacing_x, 0), header_rect.Max, gui->u32(col->text_dimmed), desc, nullptr, nullptr, { 0.f, 1.f });
		}

		draw->line(drawlist, pos + ImVec2(0, header_height) - top_animation_pos, pos + ImVec2(size.x, header_height) - top_animation_pos, gui->u32(var->style.colors[style_col_border]), var->style.child_border_size);
	}

	void render_esp_tab() {
		static bool init_esp = false;
		if (!init_esp)
		{
			init_esp_preview(var->esp_preview);
			
			// Load bar positions from individual g_Options.ESP positions
			if (var->esp_preview.bar_items.size() >= 2)
			{
				var->esp_preview.bar_items[0].displacement_type = barPosToDisplacement(g_Options.ESP.HealthBarPosition);
				var->esp_preview.bar_items[1].displacement_type = barPosToDisplacement(g_Options.ESP.ArmorBarPosition);
			}
			
			// Load text positions from g_Options.ESP (convert 0=Top/1=Bottom to displacement)
			if (var->esp_preview.text_items.size() >= 3)
			{
				var->esp_preview.text_items[0].displacement_type = textPosToDisplacement(g_Options.ESP.NamePosition);
				var->esp_preview.text_items[1].displacement_type = textPosToDisplacement(g_Options.ESP.DistancePosition);
				var->esp_preview.text_items[2].displacement_type = textPosToDisplacement(g_Options.ESP.WeaponPosition);
			}
			
			init_esp = true;
		}
		
		// Update preview state based on current config values
		var->esp_preview.bounding_box = var->cfg.get<bool>("esp_box");
		var->esp_preview.box_type = var->cfg.get<int>("esp_box_type");
		var->esp_preview.health_bar = var->cfg.get<bool>("esp_health_bar");
		var->esp_preview.armor_bar = var->cfg.get<bool>("esp_armor_bar");
		var->esp_preview.skeleton = var->cfg.get<bool>("esp_skeleton");
		
		if (var->esp_preview.bar_items.size() >= 2)
		{
			var->esp_preview.bar_items[0].enabled = var->esp_preview.health_bar;
			var->esp_preview.bar_items[1].enabled = var->esp_preview.armor_bar;
		}
		
		if (var->esp_preview.text_items.size() >= 3)
		{
			var->esp_preview.text_items[0].enabled = var->cfg.get<bool>("esp_name");
			var->esp_preview.text_items[1].enabled = var->cfg.get<bool>("esp_distance");
			var->esp_preview.text_items[2].enabled = var->cfg.get<bool>("esp_weapon_name");
		}
		
		// Sync colors from config to preview (always sync)
		var->esp_preview.bounding_box_color = var->cfg.get<ImVec4>("esp_box_color");
		if (var->esp_preview.bar_items.size() >= 2)
		{
			var->esp_preview.bar_items[0].color = var->cfg.get<ImVec4>("esp_health_color");
			var->esp_preview.bar_items[1].color = var->cfg.get<ImVec4>("esp_armor_color");
		}
		if (var->esp_preview.text_items.size() >= 3)
		{
			var->esp_preview.text_items[0].color = var->cfg.get<ImVec4>("esp_name_color");
			var->esp_preview.text_items[1].color = var->cfg.get<ImVec4>("esp_distance_color");
			var->esp_preview.text_items[2].color = var->cfg.get<ImVec4>("esp_weapon_color");
		}
		
		// Sync bar positions from preview to g_Options.ESP individual positions
		if (var->esp_preview.bar_items.size() >= 2)
		{
			g_Options.ESP.HealthBarPosition = displacementToBarPos(var->esp_preview.bar_items[0].displacement_type);
			g_Options.ESP.ArmorBarPosition = displacementToBarPos(var->esp_preview.bar_items[1].displacement_type);
			
			// Also update legacy baropt for compatibility
			displacement health_pos = var->esp_preview.bar_items[0].displacement_type;
			displacement armor_pos = var->esp_preview.bar_items[1].displacement_type;
			if (health_pos == RIGHT && armor_pos == RIGHT) {
				g_Options.ESP.baropt = 0;
			} else if (health_pos == LEFT && armor_pos == LEFT) {
				g_Options.ESP.baropt = 1;
			} else if (health_pos == RIGHT && armor_pos == LEFT) {
				g_Options.ESP.baropt = 2;
			} else if (health_pos == BOTTOM && armor_pos == BOTTOM) {
				g_Options.ESP.baropt = 3;
			}
		}
		
		// Sync text positions from preview to g_Options.ESP (convert displacement to 0=Top/1=Bottom)
		if (var->esp_preview.text_items.size() >= 3)
		{
			g_Options.ESP.NamePosition = displacementToTextPos(var->esp_preview.text_items[0].displacement_type);
			g_Options.ESP.DistancePosition = displacementToTextPos(var->esp_preview.text_items[1].displacement_type);
			g_Options.ESP.WeaponPosition = displacementToTextPos(var->esp_preview.text_items[2].displacement_type);
		}

		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const float header_height = scale(elements->content.header_height_no_subtabs);
		const ImVec2 content_size = ImVec2(size.x, size.y - header_height);
		const ImVec2 padding = scale(elements->window.content_padding);

		const ImVec2 content_animation_pos = ImVec2(0.f, content_size.y * (1.f - var->style.alpha));
		gui->set_pos(gui->get_pos() + content_animation_pos, pos_all);

		gui->begin_content("Content##EspTab", content_size, padding, padding, window_flags_no_background | window_flags_no_scrollbar | window_flags_no_collapse);
		{
			widgets->begin_content_group("ESP ELEMENTS");
			widgets->checkbox_setting("Enable ESP", "Enables ESP functionality", var->cfg.get<bool>("enable_esp"));
			gui->separator();
			widgets->checkbox("Name", "Show player names", var->cfg.get<bool>("esp_name"));
			gui->separator();
			widgets->checkbox("Distance", "Show distance to players", var->cfg.get<bool>("esp_distance"));
			gui->separator();
			widgets->checkbox("Weapon Name", "Show weapon names", var->cfg.get<bool>("esp_weapon_name"));
			gui->separator();
			widgets->checkbox("Line", "Draw lines to players", var->cfg.get<bool>("esp_line"));
			gui->separator();
			widgets->checkbox("Health Bar", "Show health bars", var->cfg.get<bool>("esp_health_bar"));
			gui->separator();
			widgets->checkbox("Armor Bar", "Show armor bars", var->cfg.get<bool>("esp_armor_bar"));
			gui->separator();
			widgets->checkbox("Box", "Draw boxes around players", var->cfg.get<bool>("esp_box"));
			if (var->cfg.get<bool>("esp_box"))
			{
				gui->separator();
				static const char* box_types[] = { "Normal", "Dashed" };
				widgets->combo("Box Type", "Select box style", box_types, var->cfg.get<int>("esp_box_type"));
			}
			gui->separator();
			widgets->checkbox("Skeleton", "Show player skeletons", var->cfg.get<bool>("esp_skeleton"));
			gui->separator();
			widgets->checkbox("Head Dot", "Draw a hollow circle on players' heads", var->cfg.get<bool>("esp_head_dot"));
			gui->separator();
			widgets->checkbox("Visible Check", "Only show visible players", var->cfg.get<bool>("esp_visible_check"));
			gui->separator();
			widgets->slider_int("Max Distance", var->cfg.get<int>("esp_max_distance"), 0, 500, "%dm");
			widgets->end_content_group("ESP ELEMENTS");
		}
		gui->end_content("Content##EspTab", true);
	}

	void render_esp_color_tab() {
		// Sync colors from config to preview (always sync, no switch needed)
		var->esp_preview.bounding_box_color = var->cfg.get<ImVec4>("esp_box_color");
		if (var->esp_preview.bar_items.size() >= 2)
		{
			var->esp_preview.bar_items[0].color = var->cfg.get<ImVec4>("esp_health_color");
			var->esp_preview.bar_items[1].color = var->cfg.get<ImVec4>("esp_armor_color");
		}
		if (var->esp_preview.text_items.size() >= 3)
		{
			var->esp_preview.text_items[0].color = var->cfg.get<ImVec4>("esp_name_color");
			var->esp_preview.text_items[1].color = var->cfg.get<ImVec4>("esp_distance_color");
			var->esp_preview.text_items[2].color = var->cfg.get<ImVec4>("esp_weapon_color");
		}
		
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const float header_height = scale(elements->content.header_height_no_subtabs);
		const ImVec2 content_size = ImVec2(size.x, size.y - header_height);
		const ImVec2 padding = scale(elements->window.content_padding);

		const ImVec2 content_animation_pos = ImVec2(0.f, content_size.y * (1.f - var->style.alpha));
		gui->set_pos(gui->get_pos() + content_animation_pos, pos_all);

		// Dummy bool for checkbox_color (always true, switch hidden)
		static bool dummy_true = true;

		gui->begin_content("Content##EspColor", content_size, padding, padding, window_flags_no_background | window_flags_no_scrollbar | window_flags_no_collapse);
		{
			widgets->begin_content_group("ESP COLOR");
			widgets->color_only("Box", "Color for ESP boxes", var->cfg.get<ImVec4>("esp_box_color"));
			gui->separator();
			widgets->color_only("Skeleton", "Color for skeleton", var->cfg.get<ImVec4>("esp_skeleton_color"));
			gui->separator();
			widgets->color_only("Head Dot", "Color for the head dot", var->cfg.get<ImVec4>("esp_head_circle_color"));
			gui->separator();
			widgets->color_only("Name", "Color for names", var->cfg.get<ImVec4>("esp_name_color"));
			gui->separator();
			widgets->color_only("Weapon", "Color for weapons", var->cfg.get<ImVec4>("esp_weapon_color"));
			gui->separator();
			widgets->color_only("Distance", "Color for distance", var->cfg.get<ImVec4>("esp_distance_color"));
			gui->separator();
			widgets->color_only("Snapline", "Color for snaplines", var->cfg.get<ImVec4>("esp_snapline_color"));
			gui->separator();
			widgets->color_only("Health", "Color for health bar", var->cfg.get<ImVec4>("esp_health_color"));
			gui->separator();
			widgets->color_only("Armor", "Color for armor bar", var->cfg.get<ImVec4>("esp_armor_color"));
			gui->separator();
			widgets->color_only("Friend", "Color for friends", var->cfg.get<ImVec4>("esp_friend_color"));
			gui->separator();
			widgets->color_only("Invisible", "Color for invisible players", var->cfg.get<ImVec4>("esp_invisible_color"));
			widgets->end_content_group("ESP COLOR");
		}
		gui->end_content("Content##EspColor", true);
	}

	void render_esp_friend_tab() {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const float header_height = scale(elements->content.header_height_no_subtabs);
		const ImVec2 content_size = ImVec2(size.x, size.y - header_height);
		const ImVec2 padding = scale(elements->window.content_padding);

		const ImVec2 content_animation_pos = ImVec2(0.f, content_size.y * (1.f - var->style.alpha));
		gui->set_pos(gui->get_pos() + content_animation_pos, pos_all);

		gui->begin_content("Content##EspFriend", content_size, padding, padding, window_flags_no_background | window_flags_no_scrollbar | window_flags_no_collapse);
		{
			widgets->begin_content_group("ESP FRIEND");
			widgets->checkbox("Show Friends", "Show ESP on friends", var->cfg.get<bool>("esp_show_friends"));
			gui->separator();
			widgets->checkbox("Friend List", "Manage friend list", var->cfg.get<bool>("esp_friend_list"));
			gui->separator();
			widgets->checkbox("Friend Color Override", "Use special colors for friends", var->cfg.get<bool>("esp_friend_color_override"));
			gui->separator();
			widgets->checkbox_key("Add Friend", "Add current player to friend list", var->cfg.get<bool>("esp_add_friend_key_enabled"));
			widgets->end_content_group("ESP FRIEND");
		}
		gui->end_content("Content##EspFriend", true);
	}

	void render_preview() {
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const ImVec2 preview_size = scale(260, 460);
		const ImVec2 preview_pos = ImVec2(pos.x + size.x + scale(20), pos.y + (size.y - preview_size.y) * 0.5f);
		const ImVec2 window_padding = scale(12, 12);

		// Apply fade animation to preview - sync with page/subtab transitions
		// Use style.alpha which handles both page and subtab transitions
		const float preview_alpha = var->style.alpha;

		gui->set_next_window_size(preview_size);
		gui->set_next_window_pos(preview_pos);
		gui->push_var(style_var_window_padding, window_padding);
		gui->push_var(style_var_alpha, preview_alpha);
		gui->begin("esp_preview", nullptr, window_flags_no_resize | window_flags_no_move | window_flags_no_title_bar | window_flags_no_scrollbar | window_flags_no_scroll_with_mouse);
		{
			ImDrawList* drawlist = gui->window_drawlist();
			const ImVec2 self_pos = gui->window_pos();
			const ImVec2 self_size = gui->window_size();

			// Draw 60% opaque background (40% transparent) - 153/255 = 60%
			drawlist->AddRectFilled(self_pos, self_pos + self_size, IM_COL32(13, 13, 13, (int)(153 * preview_alpha)), scale(8));

			draw->text(drawlist, font->get("medium12"), self_pos + window_padding, gui->u32(col->text_dimmed, preview_alpha), "PREVIEW");
		
			// Center the ESP preview widget in the window
			const ImVec2 widget_size = scale(230, 420);
			const float widget_x = (self_size.x - widget_size.x) * 0.5f;
			const float widget_y = scale(32);
			gui->set_pos(ImVec2(widget_x, widget_y), pos_all);
			
			// Apply alpha to the preview widget for character fade
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, preview_alpha);
			widgets->esp_preview("preview", var->esp_preview);
			ImGui::PopStyleVar();
		}
		gui->end();
		gui->pop_var(); // style_var_alpha
		gui->pop_var(); // style_var_window_padding
	}
}