#include "framework/headers/includes.h"
#include "framework/headers/functions.h"
#include "framework/headers/sections.h"
#include "framework/settings/variables.h"
#include "framework/settings/elements.h"
#include "framework/settings/colors.h"
#include "framework/headers/font.h"
#include "framework/image_loaders/image_dx11.h"
#include "framework/headers/binds.h"
#include "cheat/Utils/Render/Overlay.hpp"
#include "cheat/Utils/Render/Interface.hpp"
#include "cheat/Main/Cheat.hpp"
#include "cheat/Main/Options.hpp"
#include "cheat/Main/FivemSDK/Fivem.hpp"
#include "cheat/Main/Features/playernames.hpp"
#include "cheat/Main/Features/player.hpp"
#include "cheat/Main/Features/silent.hpp"
#include "cheat/Main/Features/aim.hpp"
#include "cheat/Main/Features/esp.hpp"
#include "cheat/Main/Features/radar.hpp"

#include <d3d11.h>
#include <dwmapi.h>
#include <thread>
#include <fstream>
#include <ctime>
#include <chrono>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")

// Global variable for ImGui text alpha animation
float alpha_text = 1.0f;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static std::ofstream g_LogFile;

void DebugLog(const char* format, ...)
{
    if (!g_LogFile.is_open())
        g_LogFile.open("premium_menu_debug.log", std::ios::out | std::ios::trunc);
    if (g_LogFile.is_open())
    {
        time_t now = time(nullptr);
        tm* ltm = localtime(&now);
        char timeStr[64];
        strftime(timeStr, sizeof(timeStr), "[%H:%M:%S] ", ltm);
        g_LogFile << timeStr;
        char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        g_LogFile << buffer << std::endl;
        g_LogFile.flush();
    }
}

int ImGuiKeyToVK(ImGuiKey key)
{
    switch (key)
    {
        case ImGuiKey_Tab: return VK_TAB;
        case ImGuiKey_LeftArrow: return VK_LEFT;
        case ImGuiKey_RightArrow: return VK_RIGHT;
        case ImGuiKey_UpArrow: return VK_UP;
        case ImGuiKey_DownArrow: return VK_DOWN;
        case ImGuiKey_PageUp: return VK_PRIOR;
        case ImGuiKey_PageDown: return VK_NEXT;
        case ImGuiKey_Home: return VK_HOME;
        case ImGuiKey_End: return VK_END;
        case ImGuiKey_Insert: return VK_INSERT;
        case ImGuiKey_Delete: return VK_DELETE;
        case ImGuiKey_Backspace: return VK_BACK;
        case ImGuiKey_Space: return VK_SPACE;
        case ImGuiKey_Enter: return VK_RETURN;
        case ImGuiKey_Escape: return VK_ESCAPE;
        case ImGuiKey_LeftCtrl: return VK_LCONTROL;
        case ImGuiKey_LeftShift: return VK_LSHIFT;
        case ImGuiKey_LeftAlt: return VK_LMENU;
        case ImGuiKey_LeftSuper: return VK_LWIN;
        case ImGuiKey_RightCtrl: return VK_RCONTROL;
        case ImGuiKey_RightShift: return VK_RSHIFT;
        case ImGuiKey_RightAlt: return VK_RMENU;
        case ImGuiKey_RightSuper: return VK_RWIN;
        case ImGuiKey_0: return 0x30;
        case ImGuiKey_1: return 0x31;
        case ImGuiKey_2: return 0x32;
        case ImGuiKey_3: return 0x33;
        case ImGuiKey_4: return 0x34;
        case ImGuiKey_5: return 0x35;
        case ImGuiKey_6: return 0x36;
        case ImGuiKey_7: return 0x37;
        case ImGuiKey_8: return 0x38;
        case ImGuiKey_9: return 0x39;
        case ImGuiKey_A: return 0x41;
        case ImGuiKey_B: return 0x42;
        case ImGuiKey_C: return 0x43;
        case ImGuiKey_D: return 0x44;
        case ImGuiKey_E: return 0x45;
        case ImGuiKey_F: return 0x46;
        case ImGuiKey_G: return 0x47;
        case ImGuiKey_H: return 0x48;
        case ImGuiKey_I: return 0x49;
        case ImGuiKey_J: return 0x4A;
        case ImGuiKey_K: return 0x4B;
        case ImGuiKey_L: return 0x4C;
        case ImGuiKey_M: return 0x4D;
        case ImGuiKey_N: return 0x4E;
        case ImGuiKey_O: return 0x4F;
        case ImGuiKey_P: return 0x50;
        case ImGuiKey_Q: return 0x51;
        case ImGuiKey_R: return 0x52;
        case ImGuiKey_S: return 0x53;
        case ImGuiKey_T: return 0x54;
        case ImGuiKey_U: return 0x55;
        case ImGuiKey_V: return 0x56;
        case ImGuiKey_W: return 0x57;
        case ImGuiKey_X: return 0x58;
        case ImGuiKey_Y: return 0x59;
        case ImGuiKey_Z: return 0x5A;
        case ImGuiKey_F1: return VK_F1;
        case ImGuiKey_F2: return VK_F2;
        case ImGuiKey_F3: return VK_F3;
        case ImGuiKey_F4: return VK_F4;
        case ImGuiKey_F5: return VK_F5;
        case ImGuiKey_F6: return VK_F6;
        case ImGuiKey_F7: return VK_F7;
        case ImGuiKey_F8: return VK_F8;
        case ImGuiKey_F9: return VK_F9;
        case ImGuiKey_F10: return VK_F10;
        case ImGuiKey_F11: return VK_F11;
        case ImGuiKey_F12: return VK_F12;
        case ImGuiKey_MouseLeft: return VK_LBUTTON;
        case ImGuiKey_MouseRight: return VK_RBUTTON;
        case ImGuiKey_MouseMiddle: return VK_MBUTTON;
        case ImGuiKey_MouseX1: return VK_XBUTTON1;
        case ImGuiKey_MouseX2: return VK_XBUTTON2;
        default: return 0;
    }
}

// Sync keybinds from menu to g_Options
// Also handles toggle states for features that use keybinds
void SyncKeybindsToOptions()
{
    try {
        // Aimbot key (hold mode)
        if (auto* bind = keybinds->get_bind("Aimbot key")) {
            g_Options.Aimbot.key = ImGuiKeyToVK(bind->key);
        }
        
        // Silent key (hold mode)
        if (auto* bind = keybinds->get_bind("Silent Key")) {
            g_Options.Silent.key = ImGuiKeyToVK(bind->key);
        }
        
        // Magic key (hold mode)
        if (auto* bind = keybinds->get_bind("Magic Key")) {
            g_Options.Silent.Magic_key = ImGuiKeyToVK(bind->key);
        }
        
        // Triggerbot key (hold mode)
        if (auto* bind = keybinds->get_bind("Trigger key")) {
            g_Options.Triggerbot.key = ImGuiKeyToVK(bind->key);
        }
        
        // Godmode (toggle mode) - sync both key and toggle state
        if (auto* bind = keybinds->get_bind("Godmode")) {
            g_Options.Player.GodKey = ImGuiKeyToVK(bind->key);
            if (bind->key != ImGuiKey_None && bind->type == c_keybinds::switch_type::toggle) {
                g_Options.Player.God = bind->value;
            }
        }
        
        // Invisible (toggle mode)
        if (auto* bind = keybinds->get_bind("Invisible")) {
            g_Options.Exploit.KediciShadowKey = ImGuiKeyToVK(bind->key);
            if (bind->key != ImGuiKey_None && bind->type == c_keybinds::switch_type::toggle) {
                g_Options.Exploit.KediciShadow = bind->value;
            }
        }
        
        // Health (toggle mode)
        if (auto* bind = keybinds->get_bind("Health")) {
            g_Options.Player.HealthKey = ImGuiKeyToVK(bind->key);
            if (bind->key != ImGuiKey_None && bind->type == c_keybinds::switch_type::toggle) {
                g_Options.Player.Health = bind->value;
            }
        }
        
        // Armor (toggle mode)
        if (auto* bind = keybinds->get_bind("Armor")) {
            g_Options.Player.ArmorKey = ImGuiKeyToVK(bind->key);
            if (bind->key != ImGuiKey_None && bind->type == c_keybinds::switch_type::toggle) {
                g_Options.Player.Armor = bind->value;
            }
        }
        
        // Strafe Macro (toggle mode) - key is used for activation while held
        if (auto* bind = keybinds->get_bind("Strafe Macro")) {
            g_Options.Exploit.StrafeMacroBind = ImGuiKeyToVK(bind->key);
            if (bind->key != ImGuiKey_None && bind->type == c_keybinds::switch_type::toggle) {
                g_Options.Exploit.StrafeMacro = bind->value;
            }
        }
        
        // Ghost Peek Mark Key (hold mode)
        if (auto* bind = keybinds->get_bind("Ghost Peek Save")) {
            g_Options.Player.GhostPeekMarkKey = ImGuiKeyToVK(bind->key);
        }
        
        // Ghost Peek Return Key (hold mode)
        if (auto* bind = keybinds->get_bind("Ghost Peek TP")) {
            g_Options.Player.GhostPeekReturnKey = ImGuiKeyToVK(bind->key);
        }
        
        // Freecam (toggle mode)
        if (auto* bind = keybinds->get_bind("Freecam")) {
            g_Options.Player.FreecamKey = ImGuiKeyToVK(bind->key);
            if (bind->key != ImGuiKey_None && bind->type == c_keybinds::switch_type::toggle) {
                g_Options.Player.Freecam = bind->value;
            }
        }
        
        // Teleport to Waypoint (toggle mode)
        if (auto* bind = keybinds->get_bind("Teleport to Waypoint")) {
            g_Options.Player.TeleportToWaypointKey = ImGuiKeyToVK(bind->key);
            if (bind->key != ImGuiKey_None && bind->type == c_keybinds::switch_type::toggle) {
                g_Options.Player.TeleportToWaypoint = bind->value;
            }
        }
        
        // Teleport to Saved Location (toggle mode)
        if (auto* bind = keybinds->get_bind("Teleport to Saved Location")) {
            g_Options.Player.TeleportToSavedLocationKey = ImGuiKeyToVK(bind->key);
            if (bind->key != ImGuiKey_None && bind->type == c_keybinds::switch_type::toggle) {
                g_Options.Player.TeleportToSavedLocation = bind->value;
            }
        }
        
        // Freecam Teleport (toggle mode)
        if (auto* bind = keybinds->get_bind("Freecam Teleport")) {
            g_Options.Player.FreecamTeleportKey = ImGuiKeyToVK(bind->key);
            if (bind->key != ImGuiKey_None && bind->type == c_keybinds::switch_type::toggle) {
                g_Options.Player.FreecamTeleport = bind->value;
            }
        }
    } catch (...) {}
}

// Sync config values to g_Options
void SyncConfigToOptions()
{
    try {
        auto& opts = var->cfg.get_options();
        
        // ==================== GENERAL/SETTINGS ====================
        if (auto* val = std::get_if<bool>(&opts["menu_streamproof"])) g_Options.General.StreamProof = *val;
        if (auto* val = std::get_if<bool>(&opts["menu_hide_alt_tab"])) g_Options.General.HideOnAltTab = *val;
        if (auto* val = std::get_if<int>(&opts["menu_overlay_delay"])) g_Options.General.ThreadDelay = (float)*val;
        
        // ==================== AIMBOT ====================
        if (auto* val = std::get_if<bool>(&opts["enable_aimbot"])) g_Options.Aimbot.enable = *val;
        if (auto* val = std::get_if<int>(&opts["aimbot_fov"])) g_Options.Aimbot.fov = *val;
        if (auto* val = std::get_if<int>(&opts["aimbot_distance"])) g_Options.Aimbot.distance = *val;
        if (auto* val = std::get_if<float>(&opts["aimbot_x_smooth"])) g_Options.Aimbot.xsmooth = *val;
        if (auto* val = std::get_if<float>(&opts["aimbot_y_smooth"])) g_Options.Aimbot.ysmooth = *val;
        if (auto* val = std::get_if<int>(&opts["aimbot_bone"])) g_Options.Aimbot.bone = *val;
        if (auto* val = std::get_if<bool>(&opts["aimbot_visible_check"])) g_Options.Aimbot.cVisible = *val;
        if (auto* val = std::get_if<bool>(&opts["aimbot_ignore_dead"])) g_Options.Aimbot.IgnoreDead = *val;
        if (auto* val = std::get_if<bool>(&opts["aimbot_ignore_invisible"])) g_Options.Aimbot.IgnoreInvisible = *val;
        if (auto* val = std::get_if<bool>(&opts["aimbot_ignore_friends"])) g_Options.Aimbot.IgnoreFriends = *val;
        if (auto* val = std::get_if<bool>(&opts["show_aimbot_fov"])) g_Options.Aimbot.drawFov = *val;
        if (auto* val = std::get_if<ImVec4>(&opts["aimbot_fov_color"])) {
            g_Options.Aimbot.AimbotFovColor[0] = val->x;
            g_Options.Aimbot.AimbotFovColor[1] = val->y;
            g_Options.Aimbot.AimbotFovColor[2] = val->z;
            g_Options.Aimbot.AimbotFovColor[3] = val->w;
        }
        
        // ==================== TRIGGERBOT ====================
        if (auto* val = std::get_if<bool>(&opts["enable_triggerbot"])) g_Options.Triggerbot.enable = *val;
        if (auto* val = std::get_if<int>(&opts["trigger_fov"])) g_Options.Triggerbot.fov = *val;
        if (auto* val = std::get_if<int>(&opts["trigger_distance"])) g_Options.Triggerbot.distance = *val;
        if (auto* val = std::get_if<int>(&opts["trigger_delay"])) g_Options.Triggerbot.delay = *val;
        if (auto* val = std::get_if<int>(&opts["trigger_bone"])) g_Options.Triggerbot.bone = *val;
        if (auto* val = std::get_if<bool>(&opts["trigger_visible_check"])) g_Options.Triggerbot.cVisible = *val;
        if (auto* val = std::get_if<bool>(&opts["trigger_ignore_dead"])) g_Options.Triggerbot.IgnoreDead = *val;
        if (auto* val = std::get_if<bool>(&opts["trigger_ignore_invisible"])) g_Options.Triggerbot.IgnoreInvisible = *val;
        if (auto* val = std::get_if<bool>(&opts["trigger_ignore_friends"])) g_Options.Triggerbot.IgnoreFriends = *val;
        if (auto* val = std::get_if<bool>(&opts["trigger_auto_shoot"])) g_Options.Triggerbot.autoShoot = *val;
        
        // ==================== SILENT ====================
        if (auto* val = std::get_if<bool>(&opts["enable_silent"])) g_Options.Silent.enable = *val;
        if (auto* val = std::get_if<int>(&opts["silent_fov"])) g_Options.Silent.fov = *val;
        if (auto* val = std::get_if<int>(&opts["silent_distance"])) g_Options.Silent.distance = *val;
        if (auto* val = std::get_if<int>(&opts["silent_bone"])) g_Options.Silent.bone = *val;
        if (auto* val = std::get_if<bool>(&opts["silent_visible_check"])) g_Options.Silent.cVisible = *val;
        if (auto* val = std::get_if<int>(&opts["silent_miss_percentage"])) g_Options.Silent.misschance = *val;
        if (auto* val = std::get_if<bool>(&opts["silent_ignore_dead"])) g_Options.Silent.IgnoreDead = *val;
        if (auto* val = std::get_if<bool>(&opts["silent_ignore_invisible"])) g_Options.Silent.IgnoreInvisible = *val;
        if (auto* val = std::get_if<bool>(&opts["silent_ignore_friends"])) g_Options.Silent.IgnoreFriends = *val;
        if (auto* val = std::get_if<bool>(&opts["show_silent_fov"])) g_Options.Silent.drawFov = *val;
        if (auto* val = std::get_if<bool>(&opts["silent_show_locked_bone"])) g_Options.Silent.showLockedBone = *val;
        if (auto* val = std::get_if<bool>(&opts["enable_magic"])) g_Options.Silent.Magic = *val;
        if (auto* val = std::get_if<bool>(&opts["silent_waveshield_bypass"])) g_Options.Silent.waveShieldBypass = *val;
        if (auto* val = std::get_if<ImVec4>(&opts["silent_fov_color"])) {
            g_Options.Silent.SilentFovColor[0] = val->x;
            g_Options.Silent.SilentFovColor[1] = val->y;
            g_Options.Silent.SilentFovColor[2] = val->z;
            g_Options.Silent.SilentFovColor[3] = val->w;
        }
        
        // ==================== ESP ====================
        if (auto* val = std::get_if<bool>(&opts["enable_esp"])) g_Options.ESP.enable = *val;
        if (auto* val = std::get_if<bool>(&opts["esp_box"])) { g_Options.ESP.Box = *val; g_Options.ESP.DrawBox.Enabled = *val; }
        if (auto* val = std::get_if<int>(&opts["esp_box_type"])) g_Options.ESP.DrawBox.SelectedType = *val;
        if (auto* val = std::get_if<bool>(&opts["esp_skeleton"])) g_Options.ESP.Skeleton = *val;
        if (auto* val = std::get_if<bool>(&opts["esp_name"])) g_Options.ESP.Name = *val;
        if (auto* val = std::get_if<bool>(&opts["esp_distance"])) g_Options.ESP.DistanceESP = *val;
        if (auto* val = std::get_if<bool>(&opts["esp_weapon_name"])) g_Options.ESP.WeaponName = *val;
        if (auto* val = std::get_if<bool>(&opts["esp_line"])) g_Options.ESP.SnapLines = *val;
        if (auto* val = std::get_if<bool>(&opts["esp_health_bar"])) g_Options.ESP.HealthBar = *val;
        if (auto* val = std::get_if<bool>(&opts["esp_armor_bar"])) g_Options.ESP.ArmorBar = *val;
        if (auto* val = std::get_if<bool>(&opts["esp_visible_check"])) g_Options.ESP.visible = *val;
        if (auto* val = std::get_if<int>(&opts["esp_max_distance"])) g_Options.ESP.MaxDistance = *val;
        if (auto* val = std::get_if<int>(&opts["esp_bar_position"])) g_Options.ESP.baropt = *val;
        if (auto* val = std::get_if<bool>(&opts["esp_show_friends"])) g_Options.ESP.ShowFriends = *val;
        // ESP Colors
        if (auto* val = std::get_if<ImVec4>(&opts["esp_box_color"])) {
            g_Options.ESP.BoxColor[0] = val->x; g_Options.ESP.BoxColor[1] = val->y;
            g_Options.ESP.BoxColor[2] = val->z; g_Options.ESP.BoxColor[3] = val->w;
        }
        if (auto* val = std::get_if<ImVec4>(&opts["esp_skeleton_color"])) {
            g_Options.ESP.SkeletonColor[0] = val->x; g_Options.ESP.SkeletonColor[1] = val->y;
            g_Options.ESP.SkeletonColor[2] = val->z; g_Options.ESP.SkeletonColor[3] = val->w;
        }
        if (auto* val = std::get_if<ImVec4>(&opts["esp_name_color"])) {
            g_Options.ESP.NameColor[0] = val->x; g_Options.ESP.NameColor[1] = val->y;
            g_Options.ESP.NameColor[2] = val->z; g_Options.ESP.NameColor[3] = val->w;
        }
        if (auto* val = std::get_if<ImVec4>(&opts["esp_snapline_color"])) {
            g_Options.ESP.SnaplinesColor[0] = val->x; g_Options.ESP.SnaplinesColor[1] = val->y;
            g_Options.ESP.SnaplinesColor[2] = val->z; g_Options.ESP.SnaplinesColor[3] = val->w;
        }
        if (auto* val = std::get_if<ImVec4>(&opts["esp_friend_color"])) {
            g_Options.ESP.FriendColor[0] = val->x; g_Options.ESP.FriendColor[1] = val->y;
            g_Options.ESP.FriendColor[2] = val->z; g_Options.ESP.FriendColor[3] = val->w;
        }
        
        // ==================== RADAR ====================
        if (auto* val = std::get_if<bool>(&opts["radar_enabled"])) g_Options.Radar.enable = *val;
        if (auto* val = std::get_if<int>(&opts["radar_size"])) g_Options.Radar.size = *val;
        if (auto* val = std::get_if<int>(&opts["radar_range"])) g_Options.Radar.range = *val;
        if (auto* val = std::get_if<int>(&opts["radar_position"])) g_Options.Radar.position = *val;
        if (auto* val = std::get_if<bool>(&opts["radar_show_waves"])) g_Options.Radar.showWaves = *val;
        if (auto* val = std::get_if<bool>(&opts["radar_animate_waves"])) g_Options.Radar.animateWaves = *val;
        if (auto* val = std::get_if<bool>(&opts["radar_show_players"])) g_Options.Radar.showPlayers = *val;
        if (auto* val = std::get_if<bool>(&opts["radar_show_vehicles"])) g_Options.Radar.showVehicles = *val;
        if (auto* val = std::get_if<bool>(&opts["radar_show_names"])) g_Options.Radar.showNames = *val;
        if (auto* val = std::get_if<bool>(&opts["radar_circle_style"])) g_Options.Radar.circleStyle = *val;
        // Radar Colors
        if (auto* val = std::get_if<ImVec4>(&opts["radar_bg_color"])) {
            g_Options.Radar.backgroundColor[0] = val->x; g_Options.Radar.backgroundColor[1] = val->y;
            g_Options.Radar.backgroundColor[2] = val->z; g_Options.Radar.backgroundColor[3] = val->w;
        }
        if (auto* val = std::get_if<ImVec4>(&opts["radar_enemy_color"])) {
            g_Options.Radar.enemyColor[0] = val->x; g_Options.Radar.enemyColor[1] = val->y;
            g_Options.Radar.enemyColor[2] = val->z; g_Options.Radar.enemyColor[3] = val->w;
        }
        if (auto* val = std::get_if<ImVec4>(&opts["radar_friend_color"])) {
            g_Options.Radar.friendColor[0] = val->x; g_Options.Radar.friendColor[1] = val->y;
            g_Options.Radar.friendColor[2] = val->z; g_Options.Radar.friendColor[3] = val->w;
        }
        if (auto* val = std::get_if<ImVec4>(&opts["radar_vehicle_color"])) {
            g_Options.Radar.vehicleColor[0] = val->x; g_Options.Radar.vehicleColor[1] = val->y;
            g_Options.Radar.vehicleColor[2] = val->z; g_Options.Radar.vehicleColor[3] = val->w;
        }
        
        // ==================== PLAYERS AROUND ====================
        if (auto* val = std::get_if<bool>(&opts["players_around_enabled"])) g_Options.PlayersAround.enable = *val;
        if (auto* val = std::get_if<int>(&opts["players_around_distance"])) g_Options.PlayersAround.distance = *val;
        if (auto* val = std::get_if<bool>(&opts["players_around_show_distance"])) g_Options.PlayersAround.showDistance = *val;
        
        // ==================== PLAYER ====================
        { auto* godBind = keybinds->get_bind("Godmode"); if (!godBind || godBind->key == ImGuiKey_None) { if (auto* val = std::get_if<bool>(&opts["player_godmode"])) g_Options.Player.God = *val; } }
        if (auto* val = std::get_if<bool>(&opts["player_semi_godmode"])) g_Options.Player.SemiGod = *val;
        if (auto* val = std::get_if<int>(&opts["player_semi_godmode_min_health"])) g_Options.Player.SemiGodHealth = (float)*val;
        { auto* invisBind = keybinds->get_bind("Invisible"); if (!invisBind || invisBind->key == ImGuiKey_None) { if (auto* val = std::get_if<bool>(&opts["player_invisible"])) g_Options.Exploit.KediciShadow = *val; } }
        if (auto* val = std::get_if<bool>(&opts["player_no_collision"])) g_Options.Player.Nocollision = *val;
        if (auto* val = std::get_if<bool>(&opts["player_no_fall_damage"])) g_Options.Player.NoFallDamage = *val;
        if (auto* val = std::get_if<bool>(&opts["player_steal_car"])) g_Options.Exploit.stealcar = *val;
        if (auto* val = std::get_if<bool>(&opts["player_custom_fov"])) g_Options.Player.CamFovModify = *val;
        if (auto* val = std::get_if<int>(&opts["player_custom_fov_value"])) g_Options.Player.CamFovAmount = (float)*val;
        if (auto* val = std::get_if<bool>(&opts["player_health"])) g_Options.Player.Health = *val;
        if (auto* val = std::get_if<int>(&opts["player_health_value"])) g_Options.Player.HealthAmmount = (float)*val;
        if (auto* val = std::get_if<bool>(&opts["player_armor"])) g_Options.Player.Armor = *val;
        if (auto* val = std::get_if<int>(&opts["player_armor_value"])) g_Options.Player.ArmorAmmount = (float)*val;
        if (auto* val = std::get_if<bool>(&opts["player_absorption"])) g_Options.Player.Absorption = *val;
        if (auto* val = std::get_if<int>(&opts["player_absorption_percent"])) g_Options.Player.AbsorptionPercent = (float)*val;
        if (auto* val = std::get_if<bool>(&opts["player_speed"])) g_Options.Player.PSpeed = *val;
        if (auto* val = std::get_if<float>(&opts["player_speed_value"])) g_Options.Player.PSpeedAmount = *val;
        if (auto* val = std::get_if<bool>(&opts["player_noclip"])) g_Options.Player.noclip = *val;
        if (auto* val = std::get_if<float>(&opts["player_noclip_speed"])) g_Options.Player.noclipspeed = *val;
        if (auto* val = std::get_if<bool>(&opts["player_strafe_macro"])) g_Options.Exploit.StrafeMacro = *val;
        if (auto* val = std::get_if<int>(&opts["player_strafe_delay"])) g_Options.Exploit.StrafeMacroDelay = *val;
        
        // ==================== GHOST PEEK ====================
        if (auto* val = std::get_if<bool>(&opts["player_ghost_peek_enabled"])) g_Options.Player.GhostPeekEnabled = *val;
        if (auto* val = std::get_if<int>(&opts["player_ghost_peek_mark_key"])) g_Options.Player.GhostPeekMarkKey = *val;
        if (auto* val = std::get_if<int>(&opts["player_ghost_peek_return_key"])) g_Options.Player.GhostPeekReturnKey = *val;
        
        // ==================== FREECAM ====================
        // Only sync from config if no keybind is assigned (keybind controls toggle state)
        {
            auto* freecamBind = keybinds->get_bind("Freecam");
            if (!freecamBind || freecamBind->key == ImGuiKey_None) {
                if (auto* val = std::get_if<bool>(&opts["freecam_enabled"])) g_Options.Player.Freecam = *val;
            }
        }
        if (auto* val = std::get_if<float>(&opts["freecam_speed"])) g_Options.Player.FreecamSpeed = *val;
        // Only sync teleport options from config if no keybind is assigned
        {
            auto* waypointBind = keybinds->get_bind("Teleport to Waypoint");
            if (!waypointBind || waypointBind->key == ImGuiKey_None) {
                if (auto* val = std::get_if<bool>(&opts["freecam_teleport_waypoint"])) g_Options.Player.TeleportToWaypoint = *val;
            }
        }
        {
            auto* savedBind = keybinds->get_bind("Teleport to Saved Location");
            if (!savedBind || savedBind->key == ImGuiKey_None) {
                if (auto* val = std::get_if<bool>(&opts["freecam_teleport_saved"])) g_Options.Player.TeleportToSavedLocation = *val;
            }
        }
        {
            auto* freecamTpBind = keybinds->get_bind("Freecam Teleport");
            if (!freecamTpBind || freecamTpBind->key == ImGuiKey_None) {
                if (auto* val = std::get_if<bool>(&opts["freecam_teleport"])) g_Options.Player.FreecamTeleport = *val;
            }
        }
        
        // ==================== VEHICLE ====================
        if (auto* val = std::get_if<bool>(&opts["vehicle_godmode"])) g_Options.Player.VehicleGod = *val;
        if (auto* val = std::get_if<bool>(&opts["vehicle_speed_modifier"])) g_Options.Player.VehicleSpeed = *val;
        if (auto* val = std::get_if<float>(&opts["vehicle_speed_value"])) g_Options.Player.VehicleSpeedAmount = *val;
        if (auto* val = std::get_if<bool>(&opts["vehicle_super_handling"])) g_Options.Exploit.SuperHandling = *val;
        if (auto* val = std::get_if<bool>(&opts["vehicle_rocket_boost"])) g_Options.Exploit.rocket_boost = *val;
        if (auto* val = std::get_if<bool>(&opts["vehicle_jump"])) g_Options.Exploit.jump = *val;
        if (auto* val = std::get_if<bool>(&opts["vehicle_parachute"])) g_Options.Exploit.parachute = *val;
        if (auto* val = std::get_if<bool>(&opts["vehicle_gravity"])) g_Options.Exploit.Gravity = *val;
        if (auto* val = std::get_if<float>(&opts["vehicle_gravity_value"])) g_Options.Exploit.GravityCount = *val;
        if (auto* val = std::get_if<bool>(&opts["vehicle_bug"])) g_Options.Exploit.Test = *val;
        if (auto* val = std::get_if<float>(&opts["vehicle_bug_test_value"])) g_Options.Exploit.TestValue = *val;
        
        // ==================== WEAPON ====================
        if (auto* val = std::get_if<bool>(&opts["weapon_recoil_modifier"])) g_Options.Player.NoRecoil = *val;
        if (auto* val = std::get_if<float>(&opts["weapon_recoil_amount"])) g_Options.Player.NoRecoilValue = *val;
        if (auto* val = std::get_if<bool>(&opts["weapon_spread_modifier"])) g_Options.Player.NoSpread = *val;
        if (auto* val = std::get_if<float>(&opts["weapon_spread_amount"])) g_Options.Player.NoSpreadValue = *val;
        if (auto* val = std::get_if<bool>(&opts["weapon_reload_modifier"])) g_Options.Player.NoReload = *val;
        if (auto* val = std::get_if<bool>(&opts["weapon_range_modifier"])) g_Options.Player.RangeModify = *val;
        if (auto* val = std::get_if<float>(&opts["weapon_range_value"])) g_Options.Player.RangeModifyValue = *val;
        if (auto* val = std::get_if<bool>(&opts["weapon_damage_boost"])) g_Options.Player.DamageBoost = *val;
        if (auto* val = std::get_if<float>(&opts["weapon_damage_amount"])) g_Options.Player.DamageAmount = *val;
        if (auto* val = std::get_if<bool>(&opts["weapon_safe_damage_boost"])) g_Options.Player.SDamageBoost = *val;
        if (auto* val = std::get_if<float>(&opts["weapon_safe_damage_amount"])) g_Options.Player.SDamageBoostValue = *val;
        if (auto* val = std::get_if<bool>(&opts["weapon_infinite_ammo"])) g_Options.Player.InfiniteAmmo = *val;
        
    } catch (...) {}
}

// Strafe Macro state
static bool g_StrafeMacroActive = false;
static std::chrono::steady_clock::time_point g_LastStrafeTime;
static int g_CurrentStrafeKey = 0;
static bool g_StrafeKeyDown = false;

// Process strafe macro (called from render loop)
void ProcessStrafeMacro()
{
    if (g_Options.Exploit.StrafeMacroBind == 0 || !g_Options.Exploit.StrafeMacro)
    {
        if (g_StrafeMacroActive && g_StrafeKeyDown)
        {
            WORD scanCode = 0;
            switch (g_CurrentStrafeKey) {
                case 0: scanCode = 0x20; break;
                case 1: scanCode = 0x11; break;
                case 2: scanCode = 0x1E; break;
                case 3: scanCode = 0x1F; break;
            }
            INPUT input = { 0 };
            input.type = INPUT_KEYBOARD;
            input.ki.wScan = scanCode;
            input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));
            g_StrafeKeyDown = false;
        }
        g_StrafeMacroActive = false;
        return;
    }

    bool strafeMacroPressed = GetAsyncKeyState(g_Options.Exploit.StrafeMacroBind) & 0x8000;

    if (strafeMacroPressed)
    {
        if (!g_StrafeMacroActive)
        {
            g_StrafeMacroActive = true;
            g_LastStrafeTime = std::chrono::steady_clock::now();
            g_CurrentStrafeKey = 3;
            g_StrafeKeyDown = false;
        }

        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - g_LastStrafeTime).count();

        if (elapsed >= g_Options.Exploit.StrafeMacroDelay)
        {
            if (g_StrafeKeyDown)
            {
                WORD scanCode = 0;
                switch (g_CurrentStrafeKey) {
                    case 0: scanCode = 0x20; break;
                    case 1: scanCode = 0x11; break;
                    case 2: scanCode = 0x1E; break;
                    case 3: scanCode = 0x1F; break;
                }
                INPUT input = { 0 };
                input.type = INPUT_KEYBOARD;
                input.ki.wScan = scanCode;
                input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
                SendInput(1, &input, sizeof(INPUT));
                g_StrafeKeyDown = false;
            }

            g_CurrentStrafeKey = (g_CurrentStrafeKey + 1) % 4;

            WORD scanCode = 0;
            switch (g_CurrentStrafeKey) {
                case 0: scanCode = 0x20; break;
                case 1: scanCode = 0x11; break;
                case 2: scanCode = 0x1E; break;
                case 3: scanCode = 0x1F; break;
            }
            INPUT input = { 0 };
            input.type = INPUT_KEYBOARD;
            input.ki.wScan = scanCode;
            input.ki.dwFlags = KEYEVENTF_SCANCODE;
            SendInput(1, &input, sizeof(INPUT));
            g_StrafeKeyDown = true;

            g_LastStrafeTime = currentTime;
        }
    }
    else
    {
        if (g_StrafeMacroActive && g_StrafeKeyDown)
        {
            WORD scanCode = 0;
            switch (g_CurrentStrafeKey) {
                case 0: scanCode = 0x20; break;
                case 1: scanCode = 0x11; break;
                case 2: scanCode = 0x1E; break;
                case 3: scanCode = 0x1F; break;
            }
            INPUT input = { 0 };
            input.type = INPUT_KEYBOARD;
            input.ki.wScan = scanCode;
            input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));
            g_StrafeKeyDown = false;
        }
        g_StrafeMacroActive = false;
    }
}

// Process teleportation keybinds
void ProcessTeleportKeybinds()
{
    static bool waypointKeyPressed = false;
    static bool savedKeyPressed = false;
    static bool freecamTpKeyPressed = false;

    if (g_Options.Player.TeleportToWaypoint && g_Options.Player.TeleportToWaypointKey != 0)
    {
        bool pressed = GetAsyncKeyState(g_Options.Player.TeleportToWaypointKey) & 0x8000;
        if (pressed && !waypointKeyPressed)
        {
            Cheat::Player::TeleportToWaypoint();
        }
        waypointKeyPressed = pressed;
    }

    if (g_Options.Player.TeleportToSavedLocation && g_Options.Player.TeleportToSavedLocationKey != 0)
    {
        bool pressed = GetAsyncKeyState(g_Options.Player.TeleportToSavedLocationKey) & 0x8000;
        if (pressed && !savedKeyPressed)
        {
            Cheat::Player::TeleportToSavedLocation();
        }
        savedKeyPressed = pressed;
    }

    if (g_Options.Player.FreecamTeleport && g_Options.Player.FreecamTeleportKey != 0)
    {
        bool pressed = GetAsyncKeyState(g_Options.Player.FreecamTeleportKey) & 0x8000;
        if (pressed && !freecamTpKeyPressed)
        {
            Cheat::Player::FreecamTeleport();
        }
        freecamTpKeyPressed = pressed;
    }
}

// Main render callback - called every frame
void OnRender()
{
    try {
        // Sync keybinds and config to g_Options
        SyncKeybindsToOptions();
        SyncConfigToOptions();

        // Process strafe macro
        ProcessStrafeMacro();

        // Process teleport keybinds
        ProcessTeleportKeybinds();

        // Entity list update is handled in Cheat.cpp
    } catch (...) {}
}

// Draw ESP and overlays
void OnDraw()
{
    try {
        if (!Cheat::IsReady()) return;

        ImColor silentfovcolor(g_Options.Silent.SilentFovColor[0], g_Options.Silent.SilentFovColor[1], g_Options.Silent.SilentFovColor[2], g_Options.Silent.SilentFovColor[3]);
        ImColor aimfovcolor(g_Options.Aimbot.AimbotFovColor[0], g_Options.Aimbot.AimbotFovColor[1], g_Options.Aimbot.AimbotFovColor[2], g_Options.Aimbot.AimbotFovColor[3]);

        // Draw ESP
        Cheat::ESP::run();
        Cheat::ESP::DrawActiveFeatures();
        Cheat::Radar::run();

        // Draw FOV circles
        if (g_Options.Silent.drawFov && g_Options.Silent.enable)
            ImGui::GetBackgroundDrawList()->AddCircle(ImVec2((GetSystemMetrics(SM_CXSCREEN)) / 2, (GetSystemMetrics(SM_CYSCREEN)) / 2), (float)g_Options.Silent.fov, silentfovcolor, 500, 1.5f);

        if (g_Options.Aimbot.drawFov && g_Options.Aimbot.enable)
            ImGui::GetBackgroundDrawList()->AddCircle(ImVec2((GetSystemMetrics(SM_CXSCREEN)) / 2, (GetSystemMetrics(SM_CYSCREEN)) / 2), (float)g_Options.Aimbot.fov, aimfovcolor, 500, 1.5f);

        // Locked bone indicator is drawn in Cheat.cpp
    } catch (...) {}
}

// Entry point
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // Start cheat in background thread
    std::thread cheatThread(Cheat::C_Start);
    cheatThread.detach();

    // Keep application running
    while (!g_Options.General.ShutDown)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
