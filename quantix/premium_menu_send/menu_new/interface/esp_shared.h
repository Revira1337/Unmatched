#pragma once
#include <imgui.h>
#include "../../cheat/Main/Options.hpp"

// Shared ESP state written by the Visuals pages and read by the esp-preview.
// Now directly uses g_Options.ESP for real-time sync
namespace esps
{
    // Direct references to g_Options.ESP
    inline bool& box = g_Options.ESP.Box;
    inline bool& dashed = g_Options.ESP.BoxDashed;   // false = Normal (full), true = corners only
    inline bool& skeleton = g_Options.ESP.Skeleton;
    inline bool& head_dot = g_Options.ESP.HeadCircle;
    inline bool& name = g_Options.ESP.Name;
    inline bool& distance = g_Options.ESP.DistanceESP;
    inline bool& weapon = g_Options.ESP.WeaponName;
    inline bool& health = g_Options.ESP.HealthBar;
    inline bool& armor = g_Options.ESP.ArmorBar;
    inline bool& show_id = g_Options.ESP.ShowID;
    inline bool& filled = g_Options.ESP.FilledBox;

    // element slots (drag snaps to fixed positions) - now synced with g_Options
    // text slot: 0 head-top, 1 upper-left, 2 upper-right, 3 lower-left, 4 lower-right, 5 bottom
    // bar slot:  0 bottom (horizontal), 1 left (vertical), 2 right (vertical)
    inline int& slot_name = g_Options.ESP.NamePosition;
    inline int& slot_weapon = g_Options.ESP.WeaponPosition;
    inline int& slot_distance = g_Options.ESP.DistancePosition;
    inline int& slot_health = g_Options.ESP.HealthBarPosition;
    inline int& slot_armor = g_Options.ESP.ArmorBarPosition;
    inline int& slot_id = g_Options.ESP.IDPosition;

    // Colors - direct references to g_Options.ESP colors
    inline float* c_box = g_Options.ESP.BoxColor;
    inline float* c_skel = g_Options.ESP.SkeletonColor;
    inline float* c_head = g_Options.ESP.HeadCircleColor;
    inline float* c_name = g_Options.ESP.NameColor;
    inline float* c_wep = g_Options.ESP.WeaponNameColor;
    inline float* c_dist = g_Options.ESP.DistanceColor;
    inline float* c_health = g_Options.ESP.HealthBarColor;
    inline float* c_armor = g_Options.ESP.ArmorBarColor;
    inline float* c_id = g_Options.ESP.IDColor;
    inline float* c_filled = g_Options.ESP.FilledBoxColor;
    inline float c_snap[4]   = { 0.188f, 0.549f, 1.f, 1.f };  // UI color for snap indicators
    inline float* c_friend = g_Options.ESP.FriendColor;
    inline float* c_invis = g_Options.ESP.InvisibleColor;
}
