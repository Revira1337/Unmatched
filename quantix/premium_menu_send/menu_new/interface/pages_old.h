#pragma once
#include "../helpers/feats.h"
#include "elements_manager.h"
#include "esp_shared.h"
#include "settings_shared.h"

// Access to cheat options
#include "../../cheat/Main/Options.hpp"
extern Cheat::Options g_Options;

// ---------------------------------------------------------------------------
// pages.h — one function per subtab. Each is called twice per frame, once with
// col==0 (left column) and once with col==1 (right column). Every control keeps
// its own state but does nothing (visual mockup only).
// ---------------------------------------------------------------------------
namespace pages
{
    // ============================= AIM ASSIST =============================
    inline void aimbot(int col)
    {
        if (col == 0)
        {
            ft::card("General");
            ft::toggle("Preview", "Show the aim preview panel", &settings::aim_preview);
            ft::toggle("Enable aimbot", "Enables aimbot functionality", &g_Options.Aimbot.enable);
            ft::keybind("Aimbot key", "Key to activate aimbot", &g_Options.Aimbot.key);
            ft::toggle("Show fov", "Display field of view circle", &g_Options.Aimbot.drawFov);
            ft::toggle("Visible check", "Only aim when target is visible", &g_Options.Aimbot.cVisible);
            ft::endcard();
            ft::card("Target Filters");
            ft::check("Ignore Dead", "Don't aim at dead players", &g_Options.Aimbot.IgnoreDead);
            ft::check("Ignore Invisible", "Don't aim at invisible players", &g_Options.Aimbot.IgnoreInvisible);
            ft::check("Ignore Friends", "Don't aim at friends", &g_Options.Aimbot.IgnoreFriends);
            ft::endcard();
        }
        else
        {
            ft::card("Aimbot Settings");
            ft::slideri("Fov", &g_Options.Aimbot.fov, 0, 1000);
            ft::slideri("Distance", &g_Options.Aimbot.distance, 0, 500);
            int xs = (int)g_Options.Aimbot.xsmooth;
            int ys = (int)g_Options.Aimbot.ysmooth;
            ft::slideri("X Smooth", &xs, 0, 100);
            ft::slideri("Y Smooth", &ys, 0, 100);
            g_Options.Aimbot.xsmooth = (float)xs;
            g_Options.Aimbot.ysmooth = (float)ys;
            ft::endcard();
            ft::card("FOV Color");
            ft::color("Aimbot FOV Color", "Color for aimbot FOV circle", g_Options.Aimbot.AimbotFovColor);
            ft::endcard();
        }
    }

    inline void silent(int col)
    {
        static bool en, showfov, vis, lockbone, lineto, magic, wheel, wvis, wdraw, id_, ii, if_;
        static int key, mkey, wkey; static int fov = 30, dist = 150, miss = 0, wfov = 25, wdist = 100;
        if (col == 0)
        {
            ft::card("General");
            ft::toggle("Preview", "Show the aim preview panel", &settings::aim_preview);
            ft::toggle("Enable Silent", "Enables silent aim functionality", &en);
            ft::keybind("Silent Key", "Key to activate silent aim", &key);
            ft::toggle("Show FOV", "Display field of view circle", &showfov);
            ft::toggle("Visible Check", "Only aim when target is visible", &vis);
            ft::endcard();
            ft::card("Silent Settings");
            ft::slideri("Fov", &fov, 0, 1000);
            ft::slideri("Distance", &dist, 0, 500);
            ft::slideri("Miss Chance", &miss, 0, 100, "%");
            ft::check("Show Locked Bone", "Display locked bone indicator", &lockbone);
            ft::check("Show Line to Target", "Draw line from crosshair to locked target", &lineto);
            ft::endcard();
            ft::card("Target Filters");
            ft::check("Ignore Dead", "Don't aim at dead players", &id_);
            ft::check("Ignore Invisible", "Don't aim at invisible players", &ii);
            ft::check("Ignore Friends", "Don't aim at friends", &if_);
            ft::endcard();
        }
        else
        {
            ft::card("Magic Bullet");
            ft::toggle("Enable Magic", "Enables magic bullet functionality", &magic);
            ft::keybind("Magic Key", "Key to activate magic bullet", &mkey);
            ft::endcard();
            ft::card("Wheel Silent");
            ft::toggle("Wheel Silent", "Enables wheel silent functionality", &wheel);
            ft::keybind("Wheel Silent Key", "Key to activate wheel silent", &wkey);
            ft::slideri("Fov", &wfov, 0, 1000);
            ft::slideri("Max Distance", &wdist, 0, 500, "m");
            ft::check("Visible Check", "Only aim when target is visible", &wvis);
            ft::check("Draw Fov", "Display field of view circle", &wdraw);
            ft::endcard();
            ft::card("FOV Color");
            static float sfovc[4] = { 1.f, 1.f, 1.f, 1.f };
            ft::color("Silent FOV Color", "Color for silent FOV circle", sfovc);
            ft::endcard();
        }
    }

    inline void trigger(int col)
    {
        static bool en, vis, autos, id_, ii, if_; static int key; static int dist = 100, delay = 10;
        if (col == 0)
        {
            ft::card("General");
            ft::toggle("Enable triggerbot", "Enables triggerbot functionality", &en);
            ft::keybind("Trigger key", "Key to activate triggerbot", &key);
            ft::check("Visible check", "Only trigger when target is visible", &vis);
            ft::check("Auto shoot", "Automatically shoot when target in crosshair", &autos);
            ft::endcard();
            ft::card("Target Filters");
            ft::check("Ignore Dead", "Don't trigger on dead players", &id_);
            ft::check("Ignore Invisible", "Don't trigger on invisible players", &ii);
            ft::check("Ignore Friends", "Don't trigger on friends", &if_);
            ft::endcard();
        }
        else
        {
            ft::card("Triggerbot Settings");
            ft::slideri("Distance", &dist, 0, 200, "m");
            ft::slideri("Delay", &delay, 0, 50, "ms");
            ft::endcard();
        }
    }

    // ============================== VISUALS ==============================
    inline void visuals(int col)
    {
        static bool vis, en, line; static int maxd = 250, boxtype = 0;
        if (col == 0)
        {
            ft::card("Options");
            ft::toggle("Preview", "Show the ESP preview panel", &settings::esp_preview_on);
            ft::check("Visible Check", "Only show visible players", &vis);
            ft::slideri("Max Distance", &maxd, 0, 500, "m");
            ft::endcard();
        }
        else
        {
            ft::card("Visuals");
            ft::check("Enable ESP", "Enables ESP functionality", &en);
            ft::check("Box", "Draw boxes around players", &esps::box);
            if (esps::box)
            {
                static const char* bt[] = { "Normal", "Dashed" };
                boxtype = esps::dashed ? 1 : 0;   // sync from shared state (presets set it)
                ft::combo("Box Type", "Normal box or corners only", &boxtype, bt, 2);
                esps::dashed = (boxtype == 1);
            }
            ft::check("Skeleton", "Show player skeletons", &esps::skeleton);
            ft::check("Name", "Show player names", &esps::name);
            ft::check("Distance", "Show distance to players", &esps::distance);
            ft::check("Weapon Name", "Show weapon names", &esps::weapon);
            ft::check("Line", "Draw lines to players", &line);
            ft::check("Health Bar", "Show health bars", &esps::health);
            ft::check("Armor Bar", "Show armor bars", &esps::armor);
            ft::endcard();
        }
    }

    inline void visuals_colors(int col)
    {
        if (col == 0)
        {
            ft::card("Colors");
            ft::color("Box", "Color for ESP boxes", esps::c_box);
            ft::color("Skeleton", "Color for skeleton", esps::c_skel);
            ft::color("Name", "Color for names", esps::c_name);
            ft::color("Weapon", "Color for weapons", esps::c_wep);
            ft::color("Distance", "Color for distance", esps::c_dist);
            ft::color("Snapline", "Color for snaplines", esps::c_snap);
            ft::color("Health", "Color for health bar", esps::c_health);
            ft::color("Armor", "Color for armor bar", esps::c_armor);
            ft::endcard();
        }
        else
        {
            ft::card("More Colors");
            ft::color("Friend", "Color for friends", esps::c_friend);
            ft::color("Invisible", "Color for invisible players", esps::c_invis);
            ft::endcard();
        }
    }

    inline void visuals_friends(int col)
    {
        static bool show, list, over; static int addkey;
        if (col == 0)
        {
            ft::card("Friends");
            ft::check("Show Friends", "Show ESP on friends", &show);
            ft::check("Friend List", "Manage friend list", &list);
            ft::check("Friend Color Override", "Use special colors for friends", &over);
            ft::keybind("Add Friend", "Key to add the aimed player to friends", &addkey);
            ft::endcard();
        }
        else
        {
            ft::card("Friend List");
            ft::placeholder("No friends added", 140.f);
            ft::endcard();
        }
    }

    // ============================== PLAYER ==============================
    inline void player_general(int col)
    {
        static bool god, semigod, steal, invis, nocol, nofall, cfov, absorp, pspeed, strafe;
        static bool health, armor, ghost; static int gkey, tkey;
        static int k_god, k_semi, k_invis, k_health, k_armor;
        static int minhp = 20, fovv = 90, hpv = 200, arv = 100, absv = 50, strd = 50;
        static float spd = 1.f;
        if (col == 0)
        {
            ft::card("Player");
            ft::toggle_key("Godmode", "Makes you invincible to all damage", &god, &k_god);
            ft::toggle_key("Semi-Godmode", "Keeps your health above minimum threshold", &semigod, &k_semi);
            ft::slideri("Min Health", &minhp, 0, 70);
            ft::check("Steal Car", "Instantly steal any vehicle", &steal);
            ft::toggle_key("Invisible", "Makes you invisible to other players", &invis, &k_invis);
            ft::check("NoCollision", "Walk through walls and objects", &nocol);
            ft::check("No Fall Damage", "Prevents damage from falling", &nofall);
            ft::endcard();
            ft::card("Movement");
            ft::check("Player Speed", "Modify your movement speed", &pspeed);
            ft::sliderf("Speed Value", &spd, 0.f, 1.f);
            ft::toggle("Strafe Macro", "Automated strafe movement", &strafe);
            ft::slideri("Strafe Delay (ms)", &strd, 0, 100);
            ft::endcard();
        }
        else
        {
            ft::card("Stats");
            ft::check("Custom FOV", "Adjust your field of view", &cfov);
            ft::slideri("FOV Value", &fovv, 0, 90);
            ft::toggle_key("Health", "Set your health value", &health, &k_health);
            ft::slideri("Health Value", &hpv, 0, 100);
            ft::toggle_key("Armor", "Set your armor value", &armor, &k_armor);
            ft::slideri("Armor Value", &arv, 0, 100);
            ft::check("Absorption", "Damage absorption percentage", &absorp);
            ft::slideri("Absorption %", &absv, 0, 100);
            ft::endcard();
            ft::card("Ghost Peek");
            ft::check("Enable Ghost Peek", "Enable ghost peek teleport feature", &ghost);
            ft::keybind("Ghost Peek Save", "Key to save current position", &gkey);
            ft::keybind("Ghost Peek TP", "Key to teleport back to saved position", &tkey);
            ft::endcard();
        }
    }

    inline void player_freecam(int col)
    {
        static bool fc; static int speed = 50; static int k1, k2, k3;
        if (col == 0)
        {
            ft::card("Camera");
            ft::toggle("Freecam", "Enable free camera movement", &fc);
            ft::slideri("Freecam Speed", &speed, 0, 100);
            ft::endcard();
        }
        else
        {
            ft::card("Teleport");
            ft::keybind("Teleport to Waypoint", "Teleport to your map waypoint", &k1);
            ft::keybind("Teleport to Saved Location", "Teleport to previously saved location", &k2);
            ft::keybind("Freecam Teleport", "Teleport to freecam position", &k3);
            ft::button("Save Current Location");
            ft::endcard();
        }
    }

    inline void player_players(int col)
    {
        static int rng = 250;
        if (col == 0)
        {
            ft::card("Players");
            ft::search("Search...");
            ft::slideri("Range", &rng, 0, 500, "m");
            ft::placeholder("No players in range", 150.f);
            ft::endcard();
        }
        else
        {
            ft::card("Details");
            ft::placeholder("Select a player from the list", 210.f);
            ft::endcard();
        }
    }

    // ============================== VEHICLE ==============================
    inline void vehicle(int col)
    {
        static bool god, vspeed, superh, bug, rocket, jump, para, grav;
        static float spdv = 1.f, bugv = 1.f, gravv = 1.f;
        if (col == 0)
        {
            ft::card("Protection");
            ft::toggle("Veh Godmode", "Makes your vehicle invincible", &god);
            ft::endcard();
            ft::card("Performance");
            ft::toggle("VehSpeed Modifier", "Modify vehicle speed", &vspeed);
            ft::sliderf("Speed Value", &spdv, 0.f, 5.f);
            ft::toggle("Super Handling", "Enhanced vehicle handling", &superh);
            ft::toggle("BugVehicle", "Vehicle bug exploit", &bug);
            ft::sliderf("Bug Value", &bugv, 0.f, 5.f);
            ft::endcard();
        }
        else
        {
            ft::card("Abilities");
            ft::toggle("Rocket Boost", "Add rocket boost to vehicle", &rocket);
            ft::toggle("Jump", "Vehicle jump ability", &jump);
            ft::toggle("Parachute", "Deploy parachute from vehicle", &para);
            ft::toggle("Vehicle Gravity", "Modify vehicle gravity", &grav);
            ft::sliderf("Gravity Value", &gravv, 0.f, 5.f);
            ft::endcard();
        }
    }

    inline void vehicle_repair(int col)
    {
        static int key;
        if (col == 0)
        {
            ft::card("Repair");
            ft::keybind("Fix Vehicle Key", "Key to fix vehicle", &key);
            ft::button("Fix Vehicle");
            ft::button("Max Fuel");
            ft::button("Engine HP");
            ft::button("All Fix");
            ft::endcard();
        }
        else
        {
            ft::card("Lock");
            ft::button("All Lock");
            ft::button("All Unlock");
            ft::endcard();
        }
    }

    inline void vehicle_vehicles(int col)
    {
        static int rng = 250;
        if (col == 0)
        {
            ft::card("Vehicles");
            ft::search("Search...");
            ft::slideri("Range", &rng, 0, 500, "m");
            ft::placeholder("No vehicles in range", 150.f);
            ft::endcard();
        }
        else
        {
            ft::card("Details");
            ft::placeholder("Select a vehicle from the list", 210.f);
            ft::endcard();
        }
    }

    // ============================== WEAPON ==============================
    inline void weapon_modifiers(int col)
    {
        static bool recoil, spread, reload, range;
        static float ramt = 0.f, samt = 0.f, rspd = 1.f, rval = 500.f;
        if (col == 0)
        {
            ft::card("Recoil");
            ft::toggle("Recoil Modifier", "Reduce weapon recoil", &recoil);
            ft::sliderf("Recoil Amount", &ramt, 0.f, 1.f);
            ft::endcard();
            ft::card("Spread");
            ft::toggle("Spread Modifier", "Reduce weapon spread", &spread);
            ft::sliderf("Spread Amount", &samt, 0.f, 1.f);
            ft::endcard();
        }
        else
        {
            ft::card("Reload");
            ft::toggle("Reload Modifier", "Modify reload speed", &reload);
            ft::sliderf("Reload Speed", &rspd, 0.f, 5.f);
            ft::endcard();
            ft::card("Range");
            ft::toggle("Range Modifier", "Modify weapon range", &range);
            ft::sliderf("Range Value", &rval, 0.f, 1000.f);
            ft::endcard();
        }
    }

    inline void weapon_weapon(int col)
    {
        static bool dmg, safedmg, inf; static int damt = 50; static float samt = 1.f;
        if (col == 0)
        {
            ft::card("Damage");
            ft::toggle("Damage Boost", "Increase weapon damage", &dmg);
            ft::slideri("Damage Amount", &damt, 0, 100);
            ft::toggle("Safe Damage Boost", "Safe damage increase", &safedmg);
            ft::sliderf("Safe Damage", &samt, 0.f, 10.f);
            ft::endcard();
        }
        else
        {
            ft::card("Ammo");
            ft::toggle("Infinite Ammo", "Never run out of ammo", &inf);
            ft::button("Set Ammo to 200");
            ft::endcard();
        }
    }

    // ============================== RADAR ==============================
    inline void radar(int col)
    {
        static bool en, waves = true, anim = true, players = true, names;
        static int size = 5, range = 100;
        if (col == 0)
        {
            ft::card("General");
            ft::check("Enable Radar", "Show radar on screen", &en);
            ft::slideri("Radar Size", &size, 0, 10);
            ft::slideri("Radar Range", &range, 0, 500);
            ft::endcard();
        }
        else
        {
            ft::card("Display");
            ft::check("Show Waves", "Display radar waves", &waves);
            ft::check("Animate Waves", "Animate radar waves", &anim);
            ft::check("Show Players", "Display players on radar", &players);
            ft::check("Show Names", "Display player names on radar", &names);
            ft::endcard();
        }
    }

    inline void radar_colors(int col)
    {
        static float bg[4] = { 0, 0, 0, 1 }, bd[4] = { 0, 0, 0, 1 };
        static float en[4] = { 0.922f, 0.251f, 0.204f, 1.f }, fr[4] = { 0.314f, 0.863f, 0.392f, 1.f };
        if (col == 0)
        {
            ft::card("Radar Colors");
            ft::color("Background Color", "Radar background color", bg);
            ft::color("Border Color", "Radar border color", bd);
            ft::endcard();
        }
        else
        {
            ft::card("Player Colors");
            ft::color("Enemy Color", "Enemy player color", en);
            ft::color("Friend Color", "Friendly player color", fr);
            ft::endcard();
        }
    }

    inline void radar_around(int col)
    {
        static bool en, showd = true; static int dist = 100;
        if (col == 0)
        {
            ft::card("Players Around");
            ft::check("Enable", "Show players around you", &en);
            ft::slideri("Distance (m)", &dist, 0, 500);
            ft::check("Show Distance", "Display distance to players", &showd);
            ft::endcard();
        }
        else
        {
            ft::card("Preview");
            ft::placeholder("No players nearby", 130.f);
            ft::endcard();
        }
    }

    // ============================== SETTINGS ==============================
    inline void settings(int col)
    {
        static bool streamp, alttab = true; static int guikey = ImGuiKey_Insert;
        if (col == 0)
        {
            ft::card("Settings");
            ft::check("StreamProof", "Hide menu from screen capture and streaming", &streamp);
            ft::check("Hide on Alt-Tab", "Automatically hide menu when switching windows", &alttab);
            ft::keybind("Gui Key", "Key to toggle menu visibility", &guikey);
            ft::endcard();
        }
        else
        {
            ft::card("Display");
            ft::check("Watermark", "Show watermark on screen", &settings::watermark);
            ft::color("Accent Color", "Change the menu accent color", settings::accent);
            ft::endcard();
            ft::card("Menu");
            if (ft::button("Unload"))
                MGR->close_requested = true;
            ft::endcard();
        }
    }

    inline void settings_config(int col)
    {
        static char cfgname[64] = "";
        if (col == 0)
        {
            ft::card("Create");
            ft::input("##cfgname", cfgname, sizeof cfgname, "Config name...");
            ft::button("Create Config");
            ft::endcard();
        }
        else
        {
            ft::card("List");
            ft::placeholder("No configs saved", 150.f);
            ft::endcard();
        }
    }
}
