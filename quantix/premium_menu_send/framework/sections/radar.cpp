#include "../headers/sections.h"

namespace {
	void render_header(const char* icon, const char* name, const char* desc);
	void render_radar_tab();
	void render_colors_tab();
	void render_friends_around_tab();
}

void sections::render_radar() {
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

	render_header(ICON_PLANET, "Radar", "Radar display, colors and player tracking settings");

	gui->pop_var();
	var->window.alpha = old_alpha;

	gui->set_pos(ImVec2(0, header_height), pos_all);

	switch (var->window.current_subtab) {
	case 0:
		render_radar_tab();
		break;
	case 1:
		render_colors_tab();
		break;
	case 2:
		render_friends_around_tab();
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

	void render_radar_tab() {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const float header_height = scale(elements->content.header_height_no_subtabs);
		const ImVec2 content_size = ImVec2(size.x, size.y - header_height);
		const ImVec2 padding = scale(elements->window.content_padding);

		const ImVec2 content_animation_pos = ImVec2(0.f, content_size.y * (1.f - var->style.alpha));
		gui->set_pos(gui->get_pos() + content_animation_pos, pos_all);

		gui->begin_content("Content##RadarMain", content_size, padding, padding, window_flags_no_background | window_flags_no_scrollbar | window_flags_no_collapse);
		{
			widgets->begin_content_group("GENERAL");
			widgets->checkbox("Enable Radar", "Show radar on screen", var->cfg.get<bool>("radar_enabled"));
			gui->separator();
			widgets->slider_int("Radar Size##RadarSize", var->cfg.get<int>("radar_size"), 50, 400);
			gui->separator();
			widgets->slider_int("Radar Range##RadarRange", var->cfg.get<int>("radar_range"), 100, 2000);
			widgets->end_content_group("GENERAL");

			widgets->begin_content_group("DISPLAY");
			widgets->checkbox("Show Waves", "Display radar waves", var->cfg.get<bool>("radar_show_waves"));
			gui->separator();
			widgets->checkbox("Animate Waves", "Animate radar waves", var->cfg.get<bool>("radar_animate_waves"));
			gui->separator();
			widgets->checkbox("Show Players", "Display players on radar", var->cfg.get<bool>("radar_show_players"));
			gui->separator();
			widgets->checkbox("Show Names", "Display player names on radar", var->cfg.get<bool>("radar_show_names"));
			widgets->end_content_group("DISPLAY");
		}
		gui->end_content("Content##RadarMain", true);
	}

	void render_colors_tab() {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const float header_height = scale(elements->content.header_height_no_subtabs);
		const ImVec2 content_size = ImVec2(size.x, size.y - header_height);
		const ImVec2 padding = scale(elements->window.content_padding);

		const ImVec2 content_animation_pos = ImVec2(0.f, content_size.y * (1.f - var->style.alpha));
		gui->set_pos(gui->get_pos() + content_animation_pos, pos_all);

		gui->begin_content("Content##RadarColors", content_size, padding, padding, window_flags_no_background | window_flags_no_scrollbar | window_flags_no_collapse);
		{
			widgets->begin_content_group("RADAR COLORS");
			widgets->checkbox_color("Background Color", "Radar background color", var->cfg.get<bool>("radar_bg_color_enabled"), var->cfg.get<ImVec4>("radar_bg_color"));
			gui->separator();
			widgets->checkbox_color("Border Color", "Radar border color", var->cfg.get<bool>("radar_border_color_enabled"), var->cfg.get<ImVec4>("radar_border_color"));
			gui->separator();
			widgets->checkbox_color("Enemy Color", "Enemy player color", var->cfg.get<bool>("radar_enemy_color_enabled"), var->cfg.get<ImVec4>("radar_enemy_color"));
			gui->separator();
			widgets->checkbox_color("Friend Color", "Friendly player color", var->cfg.get<bool>("radar_friend_color_enabled"), var->cfg.get<ImVec4>("radar_friend_color"));
			widgets->end_content_group("RADAR COLORS");
		}
		gui->end_content("Content##RadarColors", true);
	}

	void render_friends_around_tab() {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const float header_height = scale(elements->content.header_height_no_subtabs);
		const ImVec2 content_size = ImVec2(size.x, size.y - header_height);
		const ImVec2 padding = scale(elements->window.content_padding);

		const ImVec2 content_animation_pos = ImVec2(0.f, content_size.y * (1.f - var->style.alpha));
		gui->set_pos(gui->get_pos() + content_animation_pos, pos_all);

		gui->begin_content("Content##PlayersAround", content_size, padding, padding, window_flags_no_background | window_flags_no_scrollbar | window_flags_no_collapse);
		{
			widgets->begin_content_group("PLAYERS AROUND");
			widgets->checkbox("Enable", "Show players around you", var->cfg.get<bool>("players_around_enabled"));
			gui->separator();
			widgets->slider_int("Distance (m)##PlayersDistance", var->cfg.get<int>("players_around_distance"), 10, 500);
			gui->separator();
			widgets->checkbox("Show Distance", "Display distance to players", var->cfg.get<bool>("players_around_show_distance"));
			widgets->end_content_group("PLAYERS AROUND");
		}
		gui->end_content("Content##PlayersAround", true);
	}
}