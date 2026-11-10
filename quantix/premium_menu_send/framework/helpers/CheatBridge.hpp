#pragma once

#include "../settings/variables.h"

// Forward declarations to avoid include issues
namespace Cheat
{
    void C_Start();
    void C_Stop();
    bool IsReady();
    
    namespace ESP
    {
        void run();
        void DrawActiveFeatures();
    }
    
    namespace Radar
    {
        void run();
    }
}

// Include Options after forward declarations
#include "../../cheat/Main/Options.hpp"

namespace CheatBridge
{
    // Initialize cheat system
    inline void Initialize()
    {
        Cheat::C_Start();
    }

    // Shutdown cheat system
    inline void Shutdown()
    {
        Cheat::C_Stop();
    }

    // Check if cheat is ready
    inline bool IsReady()
    {
        return Cheat::IsReady();
    }

    // Helper to convert ImVec4 to float[4]
    inline void ImVec4ToFloat4(const ImVec4& vec, float* arr)
    {
        arr[0] = vec.x;
        arr[1] = vec.y;
        arr[2] = vec.z;
        arr[3] = vec.w;
    }

    // Sync all menu settings to cheat options - call this every frame
    inline void SyncSettings()
    {
        auto& cfg = var->cfg;

        // ==================== GENERAL ====================
        g_Options.General.StreamProof = cfg.get<bool>("menu_streamproof");
        g_Options.General.HideOnAltTab = cfg.get<bool>("menu_hide_alt_tab");
        g_Options.General.ThreadDelay = static_cast<float>(cfg.get<int>("menu_overlay_delay"));

        // ==================== ESP ====================
        g_Options.ESP.enable = cfg.get<bool>("enable_esp");
        g_Options.ESP.Name = cfg.get<bool>("esp_name");
        g_Options.ESP.DistanceESP = cfg.get<bool>("esp_distance");
        g_Options.ESP.WeaponName = cfg.get<bool>("esp_weapon_name");
        g_Options.ESP.SnapLines = cfg.get<bool>("esp_line");
        g_Options.ESP.HealthBar = cfg.get<bool>("esp_health_bar");
        g_Options.ESP.ArmorBar = cfg.get<bool>("esp_armor_bar");
        g_Options.ESP.Box = cfg.get<bool>("esp_box");
        g_Options.ESP.Skeleton = cfg.get<bool>("esp_skeleton");
        g_Options.ESP.visible = cfg.get<bool>("esp_visible_check");
        g_Options.ESP.HeadCircle = cfg.get<bool>("esp_head_dot");
        g_Options.ESP.MaxDistance = cfg.get<int>("esp_max_distance");
        g_Options.ESP.ShowFriends = cfg.get<bool>("esp_show_friends");
        
        // ESP Box Type (0 = Normal/Full, 1 = Cornered)
        int boxType = cfg.get<int>("esp_box_type");
        g_Options.ESP.DrawBox.SelectedType = boxType;
        g_Options.ESP.CorneredBox = (boxType == 1);
        
        // ESP Bar Position
        g_Options.ESP.baropt = cfg.get<int>("esp_bar_position");

        // ESP Colors
        ImVec4ToFloat4(cfg.get<ImVec4>("esp_box_color"), g_Options.ESP.BoxColor);
        ImVec4ToFloat4(cfg.get<ImVec4>("esp_box_color"), g_Options.ESP.CorneredBoxColor);
        ImVec4ToFloat4(cfg.get<ImVec4>("esp_box_color"), g_Options.ESP.DrawBox.Color);
        ImVec4ToFloat4(cfg.get<ImVec4>("esp_skeleton_color"), g_Options.ESP.SkeletonColor);
        ImVec4ToFloat4(cfg.get<ImVec4>("esp_name_color"), g_Options.ESP.NameColor);
        ImVec4ToFloat4(cfg.get<ImVec4>("esp_weapon_color"), g_Options.ESP.WeaponNameColor);
        ImVec4ToFloat4(cfg.get<ImVec4>("esp_distance_color"), g_Options.ESP.DistanceColor);
        ImVec4ToFloat4(cfg.get<ImVec4>("esp_snapline_color"), g_Options.ESP.SnaplinesColor);
        ImVec4ToFloat4(cfg.get<ImVec4>("esp_health_color"), g_Options.ESP.HealthBarColor);
        ImVec4ToFloat4(cfg.get<ImVec4>("esp_armor_color"), g_Options.ESP.ArmorBarColor);
        ImVec4ToFloat4(cfg.get<ImVec4>("esp_friend_color"), g_Options.ESP.FriendColor);
        ImVec4ToFloat4(cfg.get<ImVec4>("esp_invisible_color"), g_Options.ESP.InvisibleColor);
        ImVec4ToFloat4(cfg.get<ImVec4>("esp_arrow_color"), g_Options.ESP.ArrowColor);
        ImVec4ToFloat4(cfg.get<ImVec4>("esp_head_circle_color"), g_Options.ESP.HeadCircleColor);

        // ==================== AIMBOT ====================
        g_Options.Aimbot.enable = cfg.get<bool>("enable_aimbot");
        g_Options.Aimbot.fov = cfg.get<int>("aimbot_fov");
        g_Options.Aimbot.distance = static_cast<int>(cfg.get<float>("aimbot_distance"));
        g_Options.Aimbot.xsmooth = cfg.get<float>("aimbot_x_smooth");
        g_Options.Aimbot.ysmooth = cfg.get<float>("aimbot_y_smooth");
        g_Options.Aimbot.bone = cfg.get<int>("aimbot_bone");
        g_Options.Aimbot.drawFov = cfg.get<bool>("show_aimbot_fov");
        g_Options.Aimbot.cVisible = cfg.get<bool>("aimbot_visible_check");
        g_Options.Aimbot.humanizedAim = cfg.get<bool>("allow_human_errors");
        
        // ==================== SILENT AIM ====================
        g_Options.Silent.enable = cfg.get<bool>("enable_silent");
        g_Options.Silent.fov = cfg.get<int>("silent_fov");
        g_Options.Silent.distance = static_cast<int>(cfg.get<float>("silent_distance"));
        g_Options.Silent.misschance = static_cast<int>(cfg.get<float>("silent_miss_percentage"));
        g_Options.Silent.bone = cfg.get<int>("silent_bone");
        g_Options.Silent.drawFov = cfg.get<bool>("show_silent_fov");
        g_Options.Silent.cVisible = cfg.get<bool>("silent_visible_check");
        g_Options.Silent.Magic = cfg.get<bool>("enable_magic");
        g_Options.Silent.IgnoreDead = cfg.get<bool>("silent_ignore_dead");
        g_Options.Silent.IgnoreInvisible = cfg.get<bool>("silent_ignore_invisible");
        g_Options.Silent.IgnoreFriends = cfg.get<bool>("silent_ignore_npc");

        // ==================== PLAYER ====================
        g_Options.Player.God = cfg.get<bool>("player_godmode");
        g_Options.Player.SemiGod = cfg.get<bool>("player_semi_godmode");
        g_Options.Player.SemiGodHealth = static_cast<float>(cfg.get<int>("player_semi_godmode_min_health"));
        g_Options.Player.Invisible = cfg.get<bool>("player_invisible");
        g_Options.Player.Nocollision = cfg.get<bool>("player_no_collision");
        g_Options.Player.NoFallDamage = cfg.get<bool>("player_no_fall_damage");
        g_Options.Player.Shrink = cfg.get<bool>("player_shrink");
        g_Options.Player.ShrinkAmount = cfg.get<float>("player_shrink_amount");
        g_Options.Player.CamFovModify = cfg.get<bool>("player_custom_fov");
        g_Options.Player.CamFovAmount = static_cast<float>(cfg.get<int>("player_custom_fov_value"));
        g_Options.Player.Health = cfg.get<bool>("player_health");
        g_Options.Player.HealthAmmount = static_cast<float>(cfg.get<int>("player_health_value"));
        g_Options.Player.Armor = cfg.get<bool>("player_armor");
        g_Options.Player.ArmorAmmount = static_cast<float>(cfg.get<int>("player_armor_value"));
        g_Options.Player.Absorption = cfg.get<bool>("player_absorption");
        g_Options.Player.AbsorptionPercent = static_cast<float>(cfg.get<int>("player_absorption_percent"));
        g_Options.Player.PSpeed = cfg.get<bool>("player_speed");
        g_Options.Player.PSpeedAmount = cfg.get<float>("player_speed_value");
        g_Options.Exploit.StrafeMacro = cfg.get<bool>("player_strafe_macro");
        g_Options.Exploit.StrafeMacroDelay = cfg.get<int>("player_strafe_delay");
        g_Options.Exploit.stealcar = cfg.get<bool>("player_steal_car");

        // Freecam
        g_Options.Player.Freecam = cfg.get<bool>("freecam_enabled");
        g_Options.Player.FreecamSpeed = cfg.get<float>("freecam_speed");
        g_Options.Player.TeleportToWaypoint = cfg.get<bool>("freecam_teleport_waypoint");
        g_Options.Player.TeleportToSavedLocation = cfg.get<bool>("freecam_teleport_saved");
        g_Options.Player.FreecamTeleport = cfg.get<bool>("freecam_teleport");

        // ==================== VEHICLE ====================
        g_Options.Player.VehicleGod = cfg.get<bool>("vehicle_godmode");
        g_Options.Player.VehicleSpeed = cfg.get<bool>("vehicle_speed_modifier");
        g_Options.Player.VehicleSpeedAmount = cfg.get<float>("vehicle_speed_value");
        g_Options.Exploit.SuperHandling = cfg.get<bool>("vehicle_super_handling");
        g_Options.Exploit.rocket_boost = cfg.get<bool>("vehicle_rocket_boost");
        g_Options.Exploit.jump = cfg.get<bool>("vehicle_jump");
        g_Options.Exploit.parachute = cfg.get<bool>("vehicle_parachute");
        g_Options.Exploit.Gravity = cfg.get<bool>("vehicle_gravity");
        g_Options.Exploit.GravityCount = cfg.get<float>("vehicle_gravity_value");
        g_Options.Exploit.Test = cfg.get<bool>("vehicle_bug");
        g_Options.Exploit.TestValue = static_cast<float>(cfg.get<int>("vehicle_bug_test_value"));

        // ==================== WEAPON ====================
        g_Options.Player.NoRecoil = cfg.get<bool>("weapon_recoil_modifier");
        g_Options.Player.NoRecoilValue = cfg.get<float>("weapon_recoil_amount");
        g_Options.Player.NoReload = cfg.get<bool>("weapon_reload_modifier");
        g_Options.Player.NoReloadValue = static_cast<int>(cfg.get<float>("weapon_reload_speed"));
        g_Options.Player.NoSpread = cfg.get<bool>("weapon_spread_modifier");
        g_Options.Player.NoSpreadValue = cfg.get<float>("weapon_spread_amount");
        g_Options.Player.RangeModify = cfg.get<bool>("weapon_range_modifier");
        g_Options.Player.RangeModifyValue = cfg.get<float>("weapon_range_value");
        g_Options.Player.DamageBoost = cfg.get<bool>("weapon_damage_boost");
        g_Options.Player.DamageAmount = cfg.get<float>("weapon_damage_amount");
        g_Options.Player.SDamageBoost = cfg.get<bool>("weapon_safe_damage_boost");
        g_Options.Player.SDamageBoostValue = cfg.get<float>("weapon_safe_damage_amount");
        g_Options.Player.InfiniteAmmo = cfg.get<bool>("weapon_infinite_ammo");

        // ==================== RADAR ====================
        g_Options.Radar.enable = cfg.get<bool>("radar_enabled");
        g_Options.Radar.size = cfg.get<int>("radar_size");
        g_Options.Radar.range = cfg.get<int>("radar_range");
        g_Options.Radar.showWaves = cfg.get<bool>("radar_show_waves");
        g_Options.Radar.animateWaves = cfg.get<bool>("radar_animate_waves");
        g_Options.Radar.showPlayers = cfg.get<bool>("radar_show_players");
        g_Options.Radar.showNames = cfg.get<bool>("radar_show_names");
        
        // Radar Colors
        ImVec4ToFloat4(cfg.get<ImVec4>("radar_bg_color"), g_Options.Radar.backgroundColor);
        ImVec4ToFloat4(cfg.get<ImVec4>("radar_border_color"), g_Options.Radar.borderColor);
        ImVec4ToFloat4(cfg.get<ImVec4>("radar_enemy_color"), g_Options.Radar.enemyColor);
        ImVec4ToFloat4(cfg.get<ImVec4>("radar_friend_color"), g_Options.Radar.friendColor);

        // ==================== PLAYERS AROUND ====================
        g_Options.PlayersAround.enable = cfg.get<bool>("players_around_enabled");
        g_Options.PlayersAround.distance = cfg.get<int>("players_around_distance");
        g_Options.PlayersAround.showDistance = cfg.get<bool>("players_around_show_distance");

        // ==================== OTHER ====================
        g_Options.Other.watermark = cfg.get<bool>("watermark");
        g_Options.Other.enableAnimations = cfg.get<bool>("menu_enable_animations");
    }

    // Run cheat features - call this every frame in render loop
    inline void RunFeatures()
    {
        if (!IsReady())
            return;

        // Sync settings from menu to cheat
        SyncSettings();

        // Run ESP
        Cheat::ESP::run();
        Cheat::ESP::DrawActiveFeatures();

        // Run Radar
        Cheat::Radar::run();
    }
}