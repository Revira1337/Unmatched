#include "../headers/sections.h"
#include "../../cheat/Main/Options.hpp"
#include "../../cheat/Main/FivemSDK/Fivem.hpp"


namespace {
	void render_header(const char* icon, const char* name, const char* desc);
	void render_modifiers_tab();
	void render_weapon_tab();
}

void sections::render_weapon() {
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

	render_header(ICON_ARROW_MERGED, "Weapon", "Weapon modifications, recoil, damage and ammo settings");

	gui->pop_var();
	var->window.alpha = old_alpha;

	gui->set_pos(ImVec2(0, header_height), pos_all);

	switch (var->window.current_subtab) {
	case 0:
		render_modifiers_tab();
		break;
	case 1:
		render_weapon_tab();
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

	void render_modifiers_tab() {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const float header_height = scale(elements->content.header_height_no_subtabs);
		const ImVec2 content_size = ImVec2(size.x, size.y - header_height);
		const ImVec2 padding = scale(elements->window.content_padding);

		const ImVec2 content_animation_pos = ImVec2(0.f, content_size.y * (1.f - var->style.alpha));
		gui->set_pos(gui->get_pos() + content_animation_pos, pos_all);

		gui->begin_content("Content##WeaponModifiers", content_size, padding, padding, window_flags_no_background | window_flags_no_scrollbar | window_flags_no_collapse);
		{
			// RECOIL content group
			widgets->begin_content_group("RECOIL");
			widgets->checkbox("Recoil Modifier", "Reduce weapon recoil", var->cfg.get<bool>("weapon_recoil_modifier"));
			widgets->slider_float("Recoil Amount##WeaponRecoil", var->cfg.get<float>("weapon_recoil_amount"), 0.0f, 10.0f);
			widgets->end_content_group("RECOIL");

			// RELOAD content group
			widgets->begin_content_group("RELOAD");
			widgets->checkbox("Reload Modifier", "Modify reload speed", var->cfg.get<bool>("weapon_reload_modifier"));
			widgets->slider_float("Reload Speed##WeaponReload", var->cfg.get<float>("weapon_reload_speed"), 0, 1000);
			widgets->end_content_group("RELOAD");

			// SPREAD content group
			widgets->begin_content_group("SPREAD");
			widgets->checkbox("Spread Modifier", "Reduce weapon spread", var->cfg.get<bool>("weapon_spread_modifier"));
			widgets->slider_float("Spread Amount##WeaponSpread", var->cfg.get<float>("weapon_spread_amount"), 0.0f, 10.0f);
			widgets->end_content_group("SPREAD");

			// RANGE content group
			widgets->begin_content_group("RANGE");
			widgets->checkbox("Range Modifier", "Modify weapon range", var->cfg.get<bool>("weapon_range_modifier"));
			widgets->slider_float("Range Value##WeaponRange", var->cfg.get<float>("weapon_range_value"), 0.0f, 1000.0f);
			widgets->end_content_group("RANGE");
		}
		gui->end_content("Content##WeaponModifiers", true);
	}

	void render_weapon_tab() {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const float header_height = scale(elements->content.header_height_no_subtabs);
		const ImVec2 content_size = ImVec2(size.x, size.y - header_height);
		const ImVec2 padding = scale(elements->window.content_padding);

		const ImVec2 content_animation_pos = ImVec2(0.f, content_size.y * (1.f - var->style.alpha));
		gui->set_pos(gui->get_pos() + content_animation_pos, pos_all);

		gui->begin_content("Content##WeaponDamageAmmo", content_size, padding, padding, window_flags_no_background | window_flags_no_scrollbar | window_flags_no_collapse);
		{
			// DAMAGE content group
			widgets->begin_content_group("DAMAGE");
			widgets->checkbox("Damage Boost", "Increase weapon damage", var->cfg.get<bool>("weapon_damage_boost"));
			widgets->slider_float("Damage Amount##WeaponDamage", var->cfg.get<float>("weapon_damage_amount"), 0.0f, 200.0f);
			gui->separator();
			widgets->checkbox("Safe Damage Boost", "Safe damage increase", var->cfg.get<bool>("weapon_safe_damage_boost"));
			widgets->slider_float("Safe Damage##WeaponSafeDamage", var->cfg.get<float>("weapon_safe_damage_amount"), 1.0f, 10.0f);
			widgets->end_content_group("DAMAGE");

			// AMMO content group
			widgets->begin_content_group("AMMO");
			widgets->checkbox("Infinite Ammo", "Never run out of ammo", var->cfg.get<bool>("weapon_infinite_ammo"));
			gui->separator();
			if (widgets->button("Set Ammo to 200", ImVec2(0, scale(38))))
			{
				try {
					auto lplayer = Cheat::C_Five.GetLocalPlayerInfo();
					if (lplayer.Ped != nullptr) {
						uint64_t ped_ptr = reinterpret_cast<uint64_t>(lplayer.Ped);
						uint64_t weapon_manager = M_base::mem::Read<uint64_t>(ped_ptr + Cheat::WeaponManager);
						if (weapon_manager) {
							uint64_t weapon_info = M_base::mem::Read<uint64_t>(weapon_manager + 0x20);
							if (weapon_info) {
								uint64_t ammo_info = M_base::mem::Read<uint64_t>(weapon_info + 0x60);
								if (ammo_info) {
									uint64_t ammo_count = M_base::mem::Read<uint64_t>(ammo_info + 0x8);
									if (ammo_count) {
										uint64_t ammo_count2 = M_base::mem::Read<uint64_t>(ammo_count + 0x0);
										if (ammo_count2) {
											M_base::mem::WriteMemory<int>(ammo_count2 + 0x18, 200);
										}
									}
								}
							}
						}
					}
					notify->add_notify("Ammo set to 200!", notify_type::success);
				} catch (...) {
					notify->add_notify("Failed to set ammo!", notify_type::error);
				}
			}
			widgets->end_content_group("AMMO");
		}
		gui->end_content("Content##WeaponDamageAmmo", true);
	}
}