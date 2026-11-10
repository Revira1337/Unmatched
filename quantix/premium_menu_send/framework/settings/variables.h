#pragma once
#include "../headers/includes.h"
#include "../headers/flags.h"
#include "../headers/widgets.h"

enum class pages {
	AIM_ASSISTANCE,
	PLAYER,
	VEHICLE,
	WEAPON,
	ESP,
	RADAR,
	WORLD_MANIPULATION,
	GLOBAL_SETTINGS
};

class c_variables
{
public:
	struct
	{
		pages current_page = pages::AIM_ASSISTANCE;
		pages target_page = pages::AIM_ASSISTANCE;

		int current_subtab = 0;
		int target_subtab = 0;
		
		bool subtab_switched = false;
		bool window_moving = false;
		bool last_window_scrolling = false;

		float alpha = 0.f;
		ImVec2 screen_size{ 0, 0 };
		ImVec2 menu_pos{ 0, 0 };  // Track main menu position for bottom bar sync
	} window;

	struct
	{
		float dpi = 1.f;
		float stored_dpi = 1.f;
		float dpi_target = 1.f;
		bool dpi_changed = true;
		bool wanna_close = false;
		bool wanna_minimize = false;
	} gui;

	config cfg{ "#", true, {
		// Aimbot - ranges: FOV 0-300, Distance 0-1000, Smooth 0-100
		{"enable_aimbot", false},
		{"aimbot_key_enabled", false},
		{"show_aimbot_fov", false},
		{"aimbot_visible_check", false},
		{"aimbot_fov", 70},
		{"aimbot_distance", 100},
		{"aimbot_x_smooth", 0.0f},
		{"aimbot_y_smooth", 0.0f},
		{"aimbot_bone", 3},
		{"aimbot_ignore_dead", true},
		{"aimbot_ignore_invisible", true},
		{"aimbot_ignore_friends", true},
		{"aimbot_fov_color", ImVec4(1.0f, 1.0f, 1.0f, 1.0f)},
		// Aimbot bone selection (interactive skeleton)
		{"aimbot_bone_head", true},
		{"aimbot_bone_neck", true},
		{"aimbot_bone_chest", true},
		{"aimbot_bone_stomach", true},
		{"aimbot_bone_left_hip", true},
		{"aimbot_bone_right_hip", true},
		{"aimbot_bone_left_shoulder", true},
		{"aimbot_bone_right_shoulder", true},
		{"aimbot_bone_left_hand", true},
		{"aimbot_bone_right_hand", true},
		{"aimbot_bone_left_foot", true},
		{"aimbot_bone_right_foot", true},

		{"ignore_team", true},
		{"fov", 70},
		{"smoothness", 5.0f},
		{"visible_check", true},
		{"allow_human_errors", false},
		{"enable_triggerbot", false},
		{"trigger_key_enabled", false},
		{"trigger_delay", 50},
		{"trigger_bone", 3},
		{"trigger_fov", 5},
		{"trigger_distance", 200},
		{"trigger_visible_check", true},
		{"trigger_ignore_dead", true},
		{"trigger_ignore_invisible", true},
		{"trigger_ignore_friends", true},
		{"trigger_auto_shoot", true},

		// Magic
		{"enable_magic", false},
		{"magic_key_enabled", false},

		// Silent Aim - ranges: FOV 0-300, Distance 0-1000, Miss 0-100
		{"enable_silent", false},
		{"silent_key_enabled", false},
		{"show_silent_fov", false},
		{"silent_visible_check", false},
		{"silent_fov", 70},
		{"silent_distance", 300},
		{"silent_miss_percentage", 0},
		{"silent_bone", 3},
		{"silent_ignore_dead", true},
		{"silent_ignore_invisible", true},
		{"silent_ignore_friends", true},
		{"silent_fov_color", ImVec4(1.0f, 1.0f, 1.0f, 1.0f)},
		{"silent_show_locked_bone", true},
		// Silent bone selection (interactive skeleton)
		{"silent_bone_head", true},
		{"silent_bone_neck", true},
		{"silent_bone_chest", true},
		{"silent_bone_stomach", true},
		{"silent_bone_left_hip", true},
		{"silent_bone_right_hip", true},
		{"silent_bone_left_shoulder", true},
		{"silent_bone_right_shoulder", true},
		{"silent_bone_left_hand", true},
		{"silent_bone_right_hand", true},
		{"silent_bone_left_foot", true},
		{"silent_bone_right_foot", true},

		{"enable_esp", false},
		{"esp_name", false},
		{"esp_distance", false},
		{"esp_weapon_name", false},
		{"esp_line", false},
		{"esp_health_bar", false},
		{"esp_armor_bar", false},
		{"esp_box", false},
		{"esp_box_type", 0},
		{"esp_skeleton", false},
		{"esp_visible_check", false},
		{"esp_max_distance", 500},
		{"esp_bar_position", 0},
		{"esp_health_bar_pos", 3},  // BOTTOM = 3
		{"esp_armor_bar_pos", 3},   // BOTTOM = 3
		{"esp_name_pos", 0},        // TOP = 0
		{"esp_distance_pos", 3},    // BOTTOM = 3
		{"esp_weapon_pos", 0},      // TOP = 0

		{"esp_box_color_enabled", false},
		{"esp_box_color", ImVec4(1.0f, 1.0f, 1.0f, 1.0f)},
		{"esp_skeleton_color_enabled", false},
		{"esp_skeleton_color", ImVec4(1.0f, 1.0f, 1.0f, 1.0f)},
		{"esp_name_color_enabled", false},
		{"esp_name_color", ImVec4(1.0f, 1.0f, 1.0f, 1.0f)},
		{"esp_weapon_color_enabled", false},
		{"esp_weapon_color", ImVec4(1.0f, 1.0f, 1.0f, 1.0f)},
		{"esp_distance_color_enabled", false},
		{"esp_distance_color", ImVec4(1.0f, 1.0f, 1.0f, 1.0f)},
		{"esp_snapline_color_enabled", false},
		{"esp_snapline_color", ImVec4(1.0f, 1.0f, 1.0f, 1.0f)},
		{"esp_health_color_enabled", false},
		{"esp_health_color", ImVec4(0.0f, 1.0f, 0.0f, 1.0f)},
		{"esp_armor_color_enabled", false},
		{"esp_armor_color", ImVec4(0.0f, 0.0f, 1.0f, 1.0f)},
		{"esp_friend_color_enabled", false},
		{"esp_friend_color", ImVec4(0.0f, 1.0f, 1.0f, 1.0f)},
		{"esp_invisible_color_enabled", false},
		{"esp_invisible_color", ImVec4(1.0f, 0.5f, 0.5f, 1.0f)},
		{"esp_arrow_color_enabled", false},
		{"esp_arrow_color", ImVec4(1.0f, 1.0f, 0.0f, 1.0f)},
		{"esp_head_dot", false},
		{"esp_head_circle_color_enabled", false},
		{"esp_head_circle_color", ImVec4(1.0f, 0.0f, 0.0f, 1.0f)},

		{"esp_show_friends", false},
		{"esp_friend_list", false},
		{"esp_friend_color_override", false},
		{"esp_add_friend_key_enabled", false},

		{"watermark", true},
		{"keybinds", true},
		{"send_debug_info", true},
		{"window_name", "cs2.exe"},

		{"enable_anti_aim", true},
		{"aa_pitch", 0},
		{"aa_yaw", 0},
		{"aa_yaw_jitter", 12},
		{"desync_mode", 0},
		{"desync_amount", 51},
		{"fakelag_type", 0},

		{"raw_input_bypass", true},
		{"mouse_sensivity", 3.2f},
		{"mouse_smoothing", 5.f},
		{"mouse_acceleration", 0},
		{"humanize_movement", true},
		{"mouse_recoil_compensation", true},

		{"enable_charms", true},
		{"visible_charms_only", false},
		{"distance_fade", 22},
		{"chams_style", 0},
		{"chams_brightness", 134},

		{"enable_world_perception", true},
		{"map_boundaries", false},
		{"grenade_trajectory", true},
		{"trajectory_type", 0},
		{"grenade_show_distance", true},

		// Player özellikleri
		{"player_godmode", false},
		{"player_godmode_key_enabled", false},
		{"player_semi_godmode", false},
		{"player_semi_godmode_min_health", 70},
		{"player_steal_car", false},
		{"player_invisible", false},
		{"player_invisible_key_enabled", false},
		{"player_no_collision", false},
		{"player_no_fall_damage", false},
		{"player_shrink", false},
		{"player_shrink_amount", 0.7f},
		{"player_custom_fov", false},
		{"player_custom_fov_value", 90},
		{"player_health", false},
		{"player_health_key_enabled", false},
		{"player_health_value", 200},
		{"player_armor", false},
		{"player_armor_key_enabled", false},
		{"player_armor_value", 100},
		{"player_absorption", false},
		{"player_absorption_percent", 50},
		{"player_speed", false},
		{"player_speed_value", 1.0f},
		{"player_strafe_macro", false},
		{"player_strafe_key_enabled", false},
		{"player_strafe_delay", 100},
		{"player_noclip", false},
		{"player_noclip_key_enabled", false},
		{"player_noclip_speed", 100.0f},

		// Freecam özellikleri
		{"freecam_enabled", false},
		{"freecam_toggle_key_enabled", false},
		{"freecam_speed", 5.0f},
		{"freecam_teleport_waypoint", false},
		{"freecam_teleport_waypoint_key_enabled", false},
		{"freecam_teleport_saved", false},
		{"freecam_teleport_saved_key_enabled", false},
		{"freecam_teleport", false},
		{"freecam_teleport_key_enabled", false},

		// Vehicle özellikleri
		{"vehicle_godmode", false},
		{"vehicle_speed_modifier", false},
		{"vehicle_speed_value", 1.0f},
		{"vehicle_super_handling", false},
		{"vehicle_bug", false},
		{"vehicle_bug_test_value", 1.0f},
		{"vehicle_rocket_boost", false},
		{"vehicle_jump", false},
		{"vehicle_parachute", false},
		{"vehicle_gravity", false},
		{"vehicle_gravity_value", 1.0f},
		{"vehicle_fix_key_enabled", false},

		// Weapon özellikleri
		{"weapon_recoil_modifier", false},
		{"weapon_recoil_amount", 0.0f},
		{"weapon_reload_modifier", false},
		{"weapon_reload_speed", 1.0f},
		{"weapon_spread_modifier", false},
		{"weapon_spread_amount", 0.0f},
		{"weapon_range_modifier", false},
		{"weapon_range_value", 500.0f},
		{"weapon_damage_boost", false},
		{"weapon_damage_amount", 1.0f},
		{"weapon_safe_damage_boost", false},
		{"weapon_safe_damage_key_enabled", false},
		{"weapon_safe_damage_amount", 1.0f},
		{"weapon_infinite_ammo", false},

		// Radar özellikleri
		{"radar_enabled", false},
		{"radar_size", 200},
		{"radar_range", 100},
		{"radar_position", 0},
		{"radar_show_waves", true},
		{"radar_animate_waves", true},
		{"radar_show_players", true},
		{"radar_show_vehicles", true},
		{"radar_show_names", false},
		{"radar_circle_style", false},
		{"radar_bg_color_enabled", true},
		{"radar_bg_color", ImVec4(0.2f, 0.2f, 0.2f, 0.6f)},
		{"radar_border_color_enabled", true},
		{"radar_border_color", ImVec4(0.5f, 0.5f, 0.5f, 0.0f)},
		{"radar_enemy_color_enabled", true},
		{"radar_enemy_color", ImVec4(1.0f, 0.0f, 0.0f, 1.0f)},
		{"radar_friend_color_enabled", true},
		{"radar_friend_color", ImVec4(0.0f, 1.0f, 0.0f, 1.0f)},
		{"radar_vehicle_color_enabled", true},
		{"radar_vehicle_color", ImVec4(0.5f, 0.8f, 1.0f, 1.0f)},

		// Players Around özellikleri
		{"players_around_enabled", false},
		{"players_around_distance", 100},
		{"players_around_show_distance", true},

		// Menu/Settings özellikleri
		{"menu_streamproof", true},
		{"menu_hide_alt_tab", true},
		{"menu_gui_key_enabled", false},
		{"menu_overlay_delay", 0},
		{"menu_enable_animations", true},
		{"menu_watermark", true},
		{"menu_keybinds", true},
	}};

	esp_preview_state esp_preview;

	gui_style style;
};

inline std::unique_ptr<c_variables> var = std::make_unique<c_variables>();

inline void set_page(pages new_page) {
	var->window.target_page = new_page;
	var->window.target_subtab = 0;
}

inline void set_subtab(uint8_t new_subtab) {
	var->window.target_subtab = new_subtab;
	var->window.subtab_switched = true;
}