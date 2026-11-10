#pragma once
#include "../helpers/feats.h"
#include "elements_manager.h"
#include "esp_shared.h"
#include "settings_shared.h"

// Access to cheat options (g_Options is defined as inline in Options.hpp)
#include "../../cheat/Main/Options.hpp"
// Access to the live entity / vehicle lists for the Players / Vehicles pages
#include "../../cheat/Main/FivemSDK/Fivem.hpp"
#include <mutex>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>

// ---------------------------------------------------------------------------
// pages.h — Integrated with real cheat options (g_Options)
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
            ft::toggle("Ignore Dead", "Don't aim at dead players", &g_Options.Aimbot.IgnoreDead);
            ft::toggle("Ignore Invisible", "Don't aim at invisible players", &g_Options.Aimbot.IgnoreInvisible);
            ft::toggle("Ignore Friends", "Don't aim at friends", &g_Options.Aimbot.IgnoreFriends);
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
        if (col == 0)
        {
            ft::card("General");
            ft::toggle("Preview", "Show the aim preview panel", &settings::aim_preview);
            ft::toggle("Enable Silent", "Enables silent aim functionality", &g_Options.Silent.enable);
            ft::keybind("Silent Key", "Key to activate silent aim", &g_Options.Silent.key);
            ft::toggle("Show FOV", "Display field of view circle", &g_Options.Silent.drawFov);
            ft::toggle("Visible Check", "Only aim when target is visible", &g_Options.Silent.cVisible);
            ft::endcard();
            ft::card("Silent Settings");
            ft::slideri("Fov", &g_Options.Silent.fov, 0, 1000);
            ft::slideri("Distance", &g_Options.Silent.distance, 0, 500);
            ft::slideri("Miss Chance", &g_Options.Silent.misschance, 0, 100, "%");
            ft::toggle("Show Locked Bone", "Display locked bone indicator", &g_Options.Silent.showLockedBone);
            ft::toggle("Prediction", "Lead moving targets by predicting their movement", &g_Options.Silent.prediction);
            { int pv = (int)(g_Options.Silent.predictionAmount * 100.f); ft::slideri("Prediction Amount", &pv, 0, 100, "%"); g_Options.Silent.predictionAmount = pv / 100.f; }
            ft::endcard();
        }
        else
        {
            ft::card("Magic Bullet");
            ft::toggle("Enable Magic", "Enables magic bullet functionality", &g_Options.Silent.Magic);
            ft::keybind("Magic Key", "Key to activate magic bullet", &g_Options.Silent.Magic_key);
            ft::endcard();
            ft::card("FOV Color");
            ft::color("Silent FOV Color", "Color for silent FOV circle", g_Options.Silent.SilentFovColor);
            ft::endcard();
            ft::card("Target Filters");
            ft::toggle("Ignore Dead", "Don't aim at dead players", &g_Options.Silent.IgnoreDead);
            ft::toggle("Ignore Invisible", "Don't aim at invisible players", &g_Options.Silent.IgnoreInvisible);
            ft::toggle("Ignore Friends", "Don't aim at friends", &g_Options.Silent.IgnoreFriends);
            ft::endcard();
            ft::card("Wheel Silent");
            ft::toggle("Enable Wheel Silent", "Silently aim at vehicle wheels", &g_Options.Silent.WheelSilent);
            ft::keybind("Wheel Silent Key", "Key to activate wheel silent", &g_Options.Silent.WheelSilentKey);
            ft::slideri("Max Distance", &g_Options.Silent.WheelSilentDistance, 0, 500);
            ft::endcard();
        }
    }

    inline void trigger(int col)
    {
        if (col == 0)
        {
            ft::card("General");
            ft::toggle("Enable triggerbot", "Enables triggerbot functionality", &g_Options.Triggerbot.enable);
            ft::keybind("Trigger key", "Key to activate triggerbot", &g_Options.Triggerbot.key);
            ft::toggle("Visible check", "Only trigger when target is visible", &g_Options.Triggerbot.cVisible);
            ft::toggle("Auto shoot", "Automatically shoot when target in crosshair", &g_Options.Triggerbot.autoShoot);
            ft::endcard();
            ft::card("Target Filters");
            ft::toggle("Ignore Dead", "Don't trigger on dead players", &g_Options.Triggerbot.IgnoreDead);
            ft::toggle("Ignore Invisible", "Don't trigger on invisible players", &g_Options.Triggerbot.IgnoreInvisible);
            ft::toggle("Ignore Friends", "Don't trigger on friends", &g_Options.Triggerbot.IgnoreFriends);
            ft::endcard();
        }
        else
        {
            ft::card("Triggerbot Settings");
            ft::slideri("Distance", &g_Options.Triggerbot.distance, 0, 200, "m");
            ft::slideri("Delay", &g_Options.Triggerbot.delay, 0, 50, "ms");
            ft::endcard();
        }
    }

    // ============================== VISUALS ==============================
    inline void visuals(int col)
    {
        if (col == 0)
        {
            ft::card("Options");
            ft::toggle("Preview", "Show the ESP preview panel", &settings::esp_preview_on);
            ft::toggle("Visible Check", "Only show visible players", &g_Options.ESP.visible);
            ft::slideri("Max Distance", &g_Options.ESP.MaxDistance, 0, 500, "m");
            ft::endcard();

            ft::card("Vehicle ESP");
            ft::toggle("Enable Vehicle ESP", "Draw ESP on nearby vehicles", &g_Options.ESP.VehicleESP);
            ft::toggle("Name", "Show vehicle name", &g_Options.ESP.VehicleName);
            ft::toggle("Distance", "Show distance to vehicle", &g_Options.ESP.VehicleDistance);
            ft::toggle("Health Bar", "Show vehicle health bar", &g_Options.ESP.VehicleHealthBar);
            ft::slideri("Vehicle Distance", &g_Options.ESP.VehicleMaxDistance, 0, 500, "m");
            ft::endcard();

            ft::card("Combat");
            ft::toggle("Show Damage", "Show the damage you deal above the target's head", &g_Options.ESP.ShowDamage);
            ft::slideri("Damage Size", &g_Options.ESP.ShowDamageSize, 1, 10, "x");
            ft::toggle("Hit Marker", "Show an X on your crosshair when you land a hit", &g_Options.ESP.HitMarker);
            ft::endcard();
        }
        else
        {
            ft::card("Visuals");
            ft::toggle("Enable ESP", "Enables ESP functionality", &g_Options.ESP.enable);
            ft::toggle("Box", "Draw boxes around players", &g_Options.ESP.Box);
            if (g_Options.ESP.Box)
            {
                int boxStyle = g_Options.ESP.BoxDashed ? 1 : 0;
                static const char* const boxStyles[] = { "Normal", "Dashed" };
                ft::combo("Box Style", "Full box or corners only", &boxStyle, boxStyles, 2);
                g_Options.ESP.BoxDashed = (boxStyle == 1);
            }
            ft::toggle("Skeleton", "Show player skeletons", &g_Options.ESP.Skeleton);
            ft::toggle("Head Dot", "Draw a hollow circle on players' heads", &g_Options.ESP.HeadCircle);
            ft::toggle("Name", "Show player names", &g_Options.ESP.Name);
            ft::toggle("Distance", "Show distance to players", &g_Options.ESP.DistanceESP);
            ft::toggle("Show ID", "Show each player's server ID", &g_Options.ESP.ShowID);
            ft::toggle("Weapon Name", "Show weapon names", &g_Options.ESP.WeaponName);
            ft::toggle("Health Bar", "Show health bars", &g_Options.ESP.HealthBar);
            ft::toggle("Armor Bar", "Show armor bars", &g_Options.ESP.ArmorBar);
            ft::toggle("Filled Box", "Fill the box with a translucent colour (chams-like)", &g_Options.ESP.FilledBox);
            ft::endcard();
        }
    }

    inline void visuals_colors(int col)
    {
        if (col == 0)
        {
            ft::card("Colors");
            ft::color("Box", "Color for ESP boxes", g_Options.ESP.BoxColor);
            ft::color("Skeleton", "Color for skeleton", g_Options.ESP.SkeletonColor);
            ft::color("Head Dot", "Color for the head dot", g_Options.ESP.HeadCircleColor);
            ft::color("Name", "Color for names", g_Options.ESP.NameColor);
            ft::color("Weapon", "Color for weapons", g_Options.ESP.WeaponNameColor);
            ft::color("Distance", "Color for distance", g_Options.ESP.DistanceColor);
            ft::color("ID", "Color for player IDs", g_Options.ESP.IDColor);
            ft::color("Health", "Color for health bar", g_Options.ESP.HealthBarColor);
            ft::color("Armor", "Color for armor bar", g_Options.ESP.ArmorBarColor);
            ft::endcard();
        }
        else
        {
            ft::card("More Colors");
            ft::color("Friend", "Color for friends", g_Options.ESP.FriendColor);
            ft::color("Locked", "Silent-aim locked target colour", g_Options.ESP.LockedColor);
            ft::color("Invisible", "Color for invisible players", g_Options.ESP.InvisibleColor);
            ft::color("Filled Box", "Fill colour (alpha = transparency)", g_Options.ESP.FilledBoxColor);
            ft::color("Vehicle", "Vehicle ESP colour", g_Options.ESP.VehicleColor);
            ft::endcard();
        }
    }

    inline void visuals_friends(int col)
    {
        if (col == 0)
        {
            ft::card("Friends");
            ft::toggle("Show Friends", "Show the [FRIEND] tag on friends", &g_Options.ESP.ShowFriends);
            ft::toggle("Quick Add Key", "Hotkey to friend the nearest player", &g_Options.ESP.FriendAddMode);
            ft::keybind("Add Friend Key", "Press to friend/unfriend the closest player to your crosshair", &g_Options.ESP.FriendAddKey);
            ft::endcard();
            ft::card("Info");
            ft::placeholder("Add friends from the Players tab,\nthe Quick Add key, or remove them here.", 84.f);
            ft::endcard();
        }
        else
        {
            ft::card("Friend List");

            // Live friend list — every friend currently loaded in the entity list.
            // Friend status persists across rebuilds via NetIdFriendList, so this
            // reflects exactly who is flagged as a friend right now. Only the trash
            // icon on the right removes a friend (clicking the row does nothing).
            struct FRow { std::string name; int netid; int dist; };
            std::vector<FRow> friends;
            {
                std::lock_guard<std::mutex> lk(Cheat::C_Five.GetLockLists());
                for (auto& e : Cheat::C_Five.GetEntitiyList())
                {
                    if (e.StaticInfo.bIsLocalPlayer || !e.IsFriend) continue;
                    std::string nm = e.StaticInfo.Name.empty() ? ("Player " + std::to_string(e.StaticInfo.NetId)) : e.StaticInfo.Name;
                    friends.push_back({ nm, e.StaticInfo.NetId, (int)e.Distance });
                }
            }
            std::sort(friends.begin(), friends.end(), [](const FRow& a, const FRow& b) { return a.dist < b.dist; });

            // small vector trash-can icon centred at c
            auto draw_trash = [](ImDrawList* dl, ImVec2 c, ImColor col) {
                float bw = 4.5f, top = c.y - 5.f, bot = c.y + 6.f;
                dl->AddLine(ImVec2(c.x - bw - 1.5f, top), ImVec2(c.x + bw + 1.5f, top), col, 1.4f);   // lid
                dl->AddLine(ImVec2(c.x - 2.f, top - 2.f), ImVec2(c.x + 2.f, top - 2.f), col, 1.4f);   // handle
                dl->AddLine(ImVec2(c.x - bw, top + 1.f), ImVec2(c.x - bw + 0.8f, bot), col, 1.3f);    // left wall
                dl->AddLine(ImVec2(c.x + bw, top + 1.f), ImVec2(c.x + bw - 0.8f, bot), col, 1.3f);    // right wall
                dl->AddLine(ImVec2(c.x - bw + 0.8f, bot), ImVec2(c.x + bw - 0.8f, bot), col, 1.3f);   // base
                dl->AddLine(ImVec2(c.x - 1.6f, top + 3.f), ImVec2(c.x - 1.6f, bot - 1.5f), col, 1.f); // rib
                dl->AddLine(ImVec2(c.x + 1.6f, top + 3.f), ImVec2(c.x + 1.6f, bot - 1.5f), col, 1.f); // rib
            };

            int removeNet = -1;
            for (auto& fr : friends)
            {
                ImGui::PushID(fr.netid);
                ImVec2 p = ImGui::GetCursorScreenPos();
                float w = ImGui::GetContentRegionAvail().x;
                const float H = 40.f, cy = H * 0.5f;
                ImDrawList* dl = ImGui::GetWindowDrawList();

                if (ImGui::IsMouseHoveringRect(p, p + ImVec2(w, H)))
                    dl->AddRectFilled(p, p + ImVec2(w, H), c::white2, 6.f);
                if (&fr != &friends.front())
                    dl->AddLine(p + ImVec2(15.f, 0.f), p + ImVec2(w - 15.f, 0.f), c::white6, 1.f);

                // trash button (right) — the ONLY thing that removes the friend
                ImVec2 trashC(p.x + w - 15.f - 8.f, p.y + cy);
                ImGui::SetCursorScreenPos(ImVec2(trashC.x - 13.f, trashC.y - 13.f));
                if (ImGui::InvisibleButton("##del", ImVec2(26.f, 26.f))) removeNet = fr.netid;
                bool delHov = ImGui::IsItemHovered();
                if (delHov)
                    dl->AddRectFilled(ImVec2(trashC.x - 13.f, trashC.y - 13.f), ImVec2(trashC.x + 13.f, trashC.y + 13.f), ImColor(235, 90, 90, 36), 6.f);

                // name (green) + distance
                ImVec2 ns = f::medium14->CalcTextSizeA(14.f, FLT_MAX, -1.f, fr.name.c_str(), 0, NULL);
                dl->AddText(f::medium14, 14.f, p + ImVec2(15.f, cy - ns.y * 0.5f), ImColor(119, 215, 125, 255), fr.name.c_str());
                char d[16]; snprintf(d, sizeof d, "%dm", fr.dist);
                ImVec2 dsz = f::medium12->CalcTextSizeA(12.f, FLT_MAX, -1.f, d, 0, NULL);
                dl->AddText(f::medium12, 12.f, p + ImVec2(w - 15.f - 26.f - dsz.x, cy - dsz.y * 0.5f), c::white48, d);

                draw_trash(dl, trashC, delHov ? ImColor(235, 90, 90, 255) : c::white48);

                ImGui::SetCursorScreenPos(p);
                ImGui::Dummy(ImVec2(w, H));
                ImGui::PopID();
            }
            if (friends.empty())
                ft::placeholder("No friends nearby", 140.f);

            if (removeNet != -1)
            {
                std::lock_guard<std::mutex> lk(Cheat::C_Five.GetLockLists());
                for (auto& e : Cheat::C_Five.GetEntitiyList())
                {
                    if (e.StaticInfo.NetId == removeNet)
                    {
                        e.IsFriend = false; e.StaticInfo.IsFriend = false;
                        Cheat::C_Five.FriendList.erase(e.StaticInfo.Ped);
                        Cheat::C_Five.NetIdFriendList.erase(removeNet);
                        break;
                    }
                }
            }
            ft::endcard();
        }
    }

    // ============================== PLAYER ==============================
    inline void player_general(int col)
    {
        if (col == 0)
        {
            ft::card("Player");
            ft::toggle_key("Godmode", "Makes you invincible to all damage", &g_Options.Player.God, &g_Options.Player.GodKey);
            ft::toggle_key("Semi-Godmode", "Keeps your health above minimum threshold", &g_Options.Player.SemiGod, &g_Options.Player.SemiGodKey);
            int minhp = (int)g_Options.Player.SemiGodHealth;
            ft::slideri("Min Health", &minhp, 0, 70);
            g_Options.Player.SemiGodHealth = (float)minhp;
            ft::toggle_key("Immortality", "No one can shoot at you", &g_Options.Player.Passive, &g_Options.Player.PassiveKey);
            ft::toggle("Steal Car", "Instantly steal any vehicle", &g_Options.Exploit.stealcar);
            ft::toggle_key("Invisible", "Makes you invisible to other players", &g_Options.Exploit.KediciShadow, &g_Options.Exploit.KediciShadowKey);
            ft::toggle("NoCollision", "Walk through walls and objects", &g_Options.Player.Nocollision);
            ft::toggle("No Fall Damage", "Prevents damage from falling", &g_Options.Player.NoFallDamage);
            ft::toggle_key("Noclip", "Fly through walls with WASD", &g_Options.Player.noclip, &g_Options.Player.NoclipKey);
            { int nv = (int)g_Options.Player.noclipspeed; ft::slideri("Noclip Speed", &nv, 1, 300); g_Options.Player.noclipspeed = (float)nv; }
            ft::endcard();
            ft::card("Movement");
            ft::toggle("Player Speed", "Modify your movement speed (1 = normal)", &g_Options.Player.PSpeed);
            ft::sliderf("Speed Value", &g_Options.Player.PSpeedAmount, 1.0f, 3.0f);
            ft::toggle("Strafe Macro", "Enable strafe macro (W-A-S-D cycle)", &g_Options.Exploit.StrafeMacro);
            ft::keybind("Strafe Key", "Hold this key to strafe (required)", &g_Options.Exploit.StrafeMacroBind);
            ft::slideri("Strafe Delay (ms)", &g_Options.Exploit.StrafeMacroDelay, 0, 100);
            ft::endcard();
        }
        else
        {
            ft::card("Stats");
            ft::toggle("Custom FOV", "Adjust your field of view", &g_Options.Player.CamFovModify);
            int fovv = (int)g_Options.Player.CamFovAmount;
            ft::slideri("FOV Value", &fovv, 0, 90);
            g_Options.Player.CamFovAmount = (float)fovv;
            ft::toggle_key("Health", "Set your health value", &g_Options.Player.Health, &g_Options.Player.HealthKey);
            int hpv = (int)g_Options.Player.HealthAmmount;
            ft::slideri("Health Value", &hpv, 0, 100);
            g_Options.Player.HealthAmmount = (float)hpv;
            ft::toggle_key("Armor", "Set your armor value", &g_Options.Player.Armor, &g_Options.Player.ArmorKey);
            int arv = (int)g_Options.Player.ArmorAmmount;
            ft::slideri("Armor Value", &arv, 0, 100);
            g_Options.Player.ArmorAmmount = (float)arv;
            ft::toggle("Absorption", "Damage absorption percentage", &g_Options.Player.Absorption);
            int absv = (int)g_Options.Player.AbsorptionPercent;
            ft::slideri("Absorption %", &absv, 0, 100);
            g_Options.Player.AbsorptionPercent = (float)absv;
            ft::endcard();
            ft::card("Ghost Peek");
            ft::toggle("Enable Ghost Peek", "Enable ghost peek teleport feature", &g_Options.Player.GhostPeekEnabled);
            ft::keybind("Ghost Peek Save", "Key to save current position", &g_Options.Player.GhostPeekMarkKey);
            ft::keybind("Ghost Peek TP", "Key to teleport back to saved position", &g_Options.Player.GhostPeekReturnKey);
            ft::endcard();
        }
    }

    inline void player_freecam(int col)
    {
        if (col == 0)
        {
            ft::card("Camera");
            ft::toggle_key("Freecam", "Enable free camera movement", &g_Options.Player.Freecam, &g_Options.Player.FreecamKey);
            int speed = (int)g_Options.Player.FreecamSpeed;
            ft::slideri("Freecam Speed", &speed, 0, 100);
            g_Options.Player.FreecamSpeed = (float)speed;
            ft::endcard();
        }
        else
        {
            ft::card("Teleport");
            ft::keybind("Teleport to Waypoint", "Teleport to your map waypoint", &g_Options.Player.TeleportToWaypointKey);
            ft::keybind("Teleport to Saved Location", "Teleport to previously saved location", &g_Options.Player.TeleportToSavedLocationKey);
            ft::keybind("Freecam Teleport", "Teleport to freecam position", &g_Options.Player.FreecamTeleportKey);
            if (ft::button("Save Current Location"))
            {
                auto lp = Cheat::C_Five.GetLocalPlayerInfo();
                if (lp.Ped) { g_Options.Player.SavedLocation = lp.WorldPos; g_Options.Player.HasSavedLocation = true; }
            }
            if (g_Options.Player.HasSavedLocation)
            {
                char cb[80];
                snprintf(cb, sizeof cb, "%.0f, %.0f, %.0f", g_Options.Player.SavedLocation.x, g_Options.Player.SavedLocation.y, g_Options.Player.SavedLocation.z);
                ft::list_entry("##svloc", "Saved", cb, c::white72);
            }
            ft::endcard();
        }
    }

    inline int& player_selected_netid() { static int s = -1; return s; }

    inline void player_players(int col)
    {
        static int rng = 500;
        int& sel_netid = player_selected_netid();
        if (col == 0)
        {
            ft::card("Players");
            ft::search("Search...");
            ft::slideri("Range", &rng, 0, 2000, "m");
            static int psort = 0; // 0 = Closest, 1 = A-Z
            static const char* const psortItems[] = { "Closest", "A-Z" };
            ft::segment("##psort", psortItems, 2, &psort);

            struct PRow { std::string name; int dist; int netid; bool friendc; };
            std::vector<PRow> rows; int total = 0;
            {
                std::lock_guard<std::mutex> lk(Cheat::C_Five.GetLockLists());
                for (auto& e : Cheat::C_Five.GetEntitiyList())
                {
                    if (e.StaticInfo.bIsLocalPlayer) continue;
                    ++total;
                    if ((int)e.Distance > rng) continue;
                    std::string nm = e.StaticInfo.Name.empty() ? ("Player " + std::to_string(e.StaticInfo.NetId)) : e.StaticInfo.Name;
                    rows.push_back({ nm, (int)e.Distance, e.StaticInfo.NetId, e.IsFriend });
                }
            }
            if (psort == 1)
                std::sort(rows.begin(), rows.end(), [](const PRow& a, const PRow& b) {
                    std::string x = a.name, y = b.name;
                    std::transform(x.begin(), x.end(), x.begin(), ::tolower);
                    std::transform(y.begin(), y.end(), y.begin(), ::tolower);
                    return x < y; });
            else
                std::sort(rows.begin(), rows.end(), [](const PRow& a, const PRow& b) { return a.dist < b.dist; });

            int shown = 0;
            for (auto& r : rows)
            {
                char d[16]; snprintf(d, sizeof d, "%dm", r.dist);
                char id[64]; snprintf(id, sizeof id, "##plr%d", r.netid);
                ImColor nc = (r.netid == sel_netid) ? c::primary : (r.friendc ? ImColor(119, 215, 125, 255) : c::white);
                if (ft::list_entry(id, r.name.c_str(), d, nc)) sel_netid = r.netid;
                if (++shown >= 40) break;
            }
            if (shown == 0)
            {
                char msg[64];
                if (total > 0) snprintf(msg, sizeof msg, "%d beyond %dm - raise Range", total, rng);
                else           snprintf(msg, sizeof msg, "No players detected");
                ft::placeholder(msg, 150.f);
            }
            ft::endcard();
        }
        else
        {
            ft::card("Details");

            // snapshot the selected player (all ped reads done under the lock so the ped can't free)
            bool found = false; std::string name; int dist = 0, netid = sel_netid; bool isFriend = false;
            bool inVeh = false; std::string weapon = "None"; Vector3D coords(0, 0, 0);
            {
                std::lock_guard<std::mutex> lk(Cheat::C_Five.GetLockLists());
                for (auto& e : Cheat::C_Five.GetEntitiyList())
                {
                    if (!e.StaticInfo.bIsLocalPlayer && e.StaticInfo.NetId == sel_netid)
                    {
                        found = true; name = e.StaticInfo.Name; dist = e.Distance; isFriend = e.IsFriend; coords = e.Cordinates;
                        Cheat::CPed* ped = e.StaticInfo.Ped;
                        if (ped)
                        {
                            inVeh = ped->HasConfigFlag(CPED_CONFIG_FLAG_InVehicle);
                            auto wm = ped->GetWeaponManager();
                            if (wm) { auto wi = wm->GetWeaponInfo(); if (wi) { std::string w = wi->GetWeaponName(); if (!w.empty()) weapon = w; } }
                        }
                        break;
                    }
                }
            }

            if (!found) { ft::placeholder("Select a player from the list", 210.f); ft::endcard(); return; }
            if (name.empty()) name = "Player " + std::to_string(netid);
            std::string discord = "N/A", steam = "N/A";
            { auto it = Cheat::C_Five.PlayerIdToDiscord.find(netid); if (it != Cheat::C_Five.PlayerIdToDiscord.end()) discord = it->second;
              else { auto n = Cheat::C_Five.PlayerNameToDiscord.find(name); if (n != Cheat::C_Five.PlayerNameToDiscord.end()) discord = n->second; } }
            { auto it = Cheat::C_Five.PlayerIdToSteam.find(netid);   if (it != Cheat::C_Five.PlayerIdToSteam.end())   steam   = it->second;
              else { auto n = Cheat::C_Five.PlayerNameToSteam.find(name);   if (n != Cheat::C_Five.PlayerNameToSteam.end())   steam   = n->second; } }

            ft::list_entry("##dn", "Name", name.c_str(), c::white);
            char nidb[24]; snprintf(nidb, sizeof nidb, "%d", netid);
            ft::list_entry("##dnid", "Net ID", nidb, c::white72);
            char db[16]; snprintf(db, sizeof db, "%dm", dist);
            ft::list_entry("##dd", "Distance", db, c::white);
            char posb[52]; snprintf(posb, sizeof posb, "%.0f, %.0f, %.0f", coords.x, coords.y, coords.z);
            ft::list_entry("##dpos", "Position", posb, c::white72);
            ft::list_entry("##dw", "Weapon", weapon.c_str(), c::white);
            ft::list_entry("##dv", "In Vehicle", inVeh ? "true" : "false", inVeh ? ImColor(119, 215, 125, 255) : c::white48);
            ft::list_entry("##dc", "Discord ID", discord.c_str(), c::white72);
            ft::list_entry("##ds", "Steam", steam.c_str(), c::white72);

            if (ft::button("Teleport to"))
            {
                auto lp = Cheat::C_Five.GetLocalPlayerInfo();
                if (lp.Ped)
                {
                    uintptr_t pedAddr = (uintptr_t)lp.Ped;
                    uintptr_t nav = M_base::mem::Read<uintptr_t>(pedAddr + 0x30);
                    uintptr_t model = M_base::mem::Read<uintptr_t>(pedAddr + 0x20);
                    Vector3D t = coords; t.z += 1.0f;
                    Cheat::C_Five.TeleportFunc(pedAddr, nav, model, t, t, true);
                }
            }
            if (ft::button(isFriend ? "Remove Friend" : "Add Friend"))
            {
                std::lock_guard<std::mutex> lk(Cheat::C_Five.GetLockLists());
                for (auto& e : Cheat::C_Five.GetEntitiyList())
                {
                    if (e.StaticInfo.NetId == netid)
                    {
                        bool nf = !e.IsFriend;
                        e.IsFriend = nf; e.StaticInfo.IsFriend = nf;
                        if (nf) { Cheat::C_Five.FriendList[e.StaticInfo.Ped] = true; Cheat::C_Five.NetIdFriendList[netid] = true; }
                        else { Cheat::C_Five.FriendList.erase(e.StaticInfo.Ped); Cheat::C_Five.NetIdFriendList.erase(netid); }
                        break;
                    }
                }
            }
            if (discord != "N/A" && ft::button("Copy Discord ID")) ImGui::SetClipboardText(discord.c_str());
            if (steam != "N/A" && ft::button("Copy Steam Link"))
            {
                try { unsigned long long sid = std::stoull(steam, nullptr, 16);
                      std::string link = "https://steamcommunity.com/profiles/" + std::to_string(sid);
                      ImGui::SetClipboardText(link.c_str()); }
                catch (...) {}
            }
            ft::endcard();
        }
    }

    // ============================== VEHICLE ==============================
    inline void vehicle(int col)
    {
        if (col == 0)
        {
            ft::card("Protection");
            ft::toggle("Veh Godmode", "Makes your vehicle invincible", &g_Options.Player.VehicleGod);
            ft::endcard();
            ft::card("Performance");
            ft::toggle("VehSpeed Modifier", "Modify vehicle speed", &g_Options.Player.VehicleSpeed);
            ft::sliderf("Speed Value", &g_Options.Player.VehicleSpeedAmount, 0.f, 5.f);
            ft::toggle("Super Handling", "Enhanced vehicle handling", &g_Options.Exploit.SuperHandling);
            ft::toggle("BugVehicle", "Vehicle bug exploit", &g_Options.Exploit.Test);
            ft::sliderf("Bug Value", &g_Options.Exploit.TestValue, 0.f, 5.f);
            ft::endcard();
        }
        else
        {
            ft::card("Abilities");
            ft::toggle("Rocket Boost", "Add rocket boost to vehicle", &g_Options.Exploit.rocket_boost);
            ft::toggle("Jump", "Vehicle jump ability", &g_Options.Exploit.jump);
            ft::toggle("Parachute", "Deploy parachute from vehicle", &g_Options.Exploit.parachute);
            ft::toggle("Vehicle Gravity", "Modify vehicle gravity", &g_Options.Exploit.Gravity);
            ft::sliderf("Gravity Value", &g_Options.Exploit.GravityCount, 0.f, 5.f);
            ft::endcard();
        }
    }

    inline void vehicle_repair(int col)
    {
        if (col == 0)
        {
            ft::card("Repair");
            ft::keybind("Fix Vehicle Key", "Key to fix vehicle", &g_Options.Player.FixVehicleKey);
            if (ft::button("Fix Vehicle"))
            {
                auto lp = Cheat::C_Five.GetLocalPlayerInfo();
                if (lp.Ped) { auto* cv = lp.Ped->GetLastVehicle(); if (cv && (uintptr_t)cv > 0x10000) cv->FixVehicle(); }
            }
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

    inline int& vehicle_selected_id() { static int s = -1; return s; }

    inline void vehicle_vehicles(int col)
    {
        static int rng = 500;
        int& sel_vid = vehicle_selected_id();
        if (col == 0)
        {
            ft::card("Vehicles");
            ft::search("Search...");
            ft::slideri("Range", &rng, 0, 2000, "m");
            static int vsort = 0; // 0 = Closest, 1 = A-Z
            static const char* const vsortItems[] = { "Closest", "A-Z" };
            ft::segment("##vsort", vsortItems, 2, &vsort);

            struct VRow { std::string name; int dist; int vid; };
            std::vector<VRow> rows; int total = 0;
            {
                std::lock_guard<std::mutex> lk(Cheat::C_Five.GetLockLists());
                for (auto& v : Cheat::C_Five.GetVehicleList())
                {
                    ++total;
                    if ((int)v.Distance > rng) continue;
                    std::string nm = v.Name.empty() ? std::string("Vehicle") : v.Name;
                    rows.push_back({ nm, (int)v.Distance, v.randomId });
                }
            }
            if (vsort == 1)
                std::sort(rows.begin(), rows.end(), [](const VRow& a, const VRow& b) {
                    std::string x = a.name, y = b.name;
                    std::transform(x.begin(), x.end(), x.begin(), ::tolower);
                    std::transform(y.begin(), y.end(), y.begin(), ::tolower);
                    return x < y; });
            else
                std::sort(rows.begin(), rows.end(), [](const VRow& a, const VRow& b) { return a.dist < b.dist; });

            int shown = 0;
            for (auto& r : rows)
            {
                char d[16]; snprintf(d, sizeof d, "%dm", r.dist);
                char id[64]; snprintf(id, sizeof id, "##veh%d", r.vid);
                ImColor nc = (r.vid == sel_vid) ? c::primary : c::white;
                if (ft::list_entry(id, r.name.c_str(), d, nc)) sel_vid = r.vid;
                if (++shown >= 40) break;
            }
            if (shown == 0)
            {
                char msg[64];
                if (total > 0) snprintf(msg, sizeof msg, "%d beyond %dm - raise Range", total, rng);
                else           snprintf(msg, sizeof msg, "No vehicles detected");
                ft::placeholder(msg, 150.f);
            }
            ft::endcard();
        }
        else
        {
            ft::card("Details");

            bool found = false; std::string name; int dist = 0; bool locked = false;
            Cheat::CVehicle* veh = nullptr; Vector3D coords(0, 0, 0);
            {
                std::lock_guard<std::mutex> lk(Cheat::C_Five.GetLockLists());
                for (auto& v : Cheat::C_Five.GetVehicleList())
                {
                    if (v.randomId == sel_vid)
                    {
                        found = true; name = v.Name.empty() ? std::string("Vehicle") : v.Name;
                        dist = v.Distance; locked = v.LockState; veh = v.Vehicle; coords = v.Cordinates; break;
                    }
                }
            }

            if (!found) { ft::placeholder("Select a vehicle from the list", 210.f); ft::endcard(); return; }

            ft::list_entry("##vn", "Name", name.c_str(), c::white);
            char db[16]; snprintf(db, sizeof db, "%dm", dist);
            ft::list_entry("##vd", "Distance", db, c::white);
            ft::list_entry("##vl", "Locked", locked ? "true" : "false", locked ? ImColor(215, 120, 120, 255) : ImColor(119, 215, 125, 255));

            if (ft::button("Teleport to"))
            {
                auto lp = Cheat::C_Five.GetLocalPlayerInfo();
                if (lp.Ped)
                {
                    uintptr_t pedAddr = (uintptr_t)lp.Ped;
                    uintptr_t nav = M_base::mem::Read<uintptr_t>(pedAddr + 0x30);
                    uintptr_t model = M_base::mem::Read<uintptr_t>(pedAddr + 0x20);
                    Vector3D t = coords; t.z += 1.0f;
                    Cheat::C_Five.TeleportFunc(pedAddr, nav, model, t, t, true);
                }
            }
            if (veh && (uintptr_t)veh > 0x10000)
            {
                if (ft::button("Fix Vehicle")) veh->FixVehicle();
                if (ft::button("Unlock")) veh->SetDoorLock(1);
                if (ft::button("Lock"))   veh->SetDoorLock(2);
                if (ft::button("Pull"))
                {
                    // bring the vehicle to the local player
                    auto lp = Cheat::C_Five.GetLocalPlayerInfo();
                    if (lp.Ped)
                    {
                        uintptr_t vehNav = veh->GetNavigation();
                        uintptr_t vehModel = veh->GetModelInfo();
                        Vector3D dest = lp.WorldPos; dest.z += 0.5f;
                        Cheat::C_Five.TeleportFunc((uintptr_t)veh, vehNav, vehModel, dest, dest, true);
                    }
                }
                if (ft::button("Teleport Into"))
                {
                    // teleport the local player straight to the vehicle (driver seat height)
                    auto lp = Cheat::C_Five.GetLocalPlayerInfo();
                    if (lp.Ped)
                    {
                        uintptr_t pedAddr = (uintptr_t)lp.Ped;
                        uintptr_t nav = M_base::mem::Read<uintptr_t>(pedAddr + 0x30);
                        uintptr_t model = M_base::mem::Read<uintptr_t>(pedAddr + 0x20);
                        Cheat::C_Five.TeleportFunc(pedAddr, nav, model, coords, coords, true);
                    }
                }
            }
            ft::endcard();
        }
    }

    // ============================== WEAPON ==============================
    inline void weapon_modifiers(int col)
    {
        if (col == 0)
        {
            ft::card("Recoil");
            ft::toggle("Recoil Modifier", "Reduce weapon recoil", &g_Options.Player.NoRecoil);
            ft::sliderf("Recoil Amount", &g_Options.Player.NoRecoilValue, 0.f, 1.f);
            ft::endcard();
            ft::card("Spread");
            ft::toggle("Spread Modifier", "Reduce weapon spread", &g_Options.Player.NoSpread);
            ft::sliderf("Spread Amount", &g_Options.Player.NoSpreadValue, 0.f, 1.f);
            ft::endcard();
        }
        else
        {
            ft::card("Reload");
            ft::toggle("Reload Modifier", "Modify reload speed", &g_Options.Player.NoReload);
            ft::sliderf("Reload Value", &g_Options.Player.NoReloadValue, 0.f, 2000.f);
            ft::endcard();
            ft::card("Range");
            ft::toggle("Range Modifier", "Modify weapon range", &g_Options.Player.RangeModify);
            ft::sliderf("Range Value", &g_Options.Player.RangeModifyValue, 0.f, 1000.f);
            ft::endcard();
        }
    }

    inline void weapon_weapon(int col)
    {
        if (col == 0)
        {
            ft::card("Damage");
            ft::toggle("Damage Boost", "Increase weapon damage", &g_Options.Player.DamageBoost);
            int damt = (int)g_Options.Player.DamageAmount;
            ft::slideri("Damage Amount", &damt, 0, 100);
            g_Options.Player.DamageAmount = (float)damt;
            ft::toggle("Safe Damage Boost", "Safe damage increase", &g_Options.Player.SDamageBoost);
            ft::sliderf("Safe Damage", &g_Options.Player.SDamageBoostValue, 0.f, 10.f);
            ft::endcard();
        }
        else
        {
            ft::card("Ammo");
            ft::toggle("Infinite Ammo", "Never run out of ammo", &g_Options.Player.InfiniteAmmo);
            ft::button("Set Ammo to 200");
            ft::endcard();
        }
    }

    // ============================== RADAR ==============================
    inline void radar(int col)
    {
        if (col == 0)
        {
            ft::card("General");
            ft::toggle("Enable Radar", "Show radar on screen", &g_Options.Radar.enable);
            ft::slideri("Radar Size", &g_Options.Radar.size, 120, 400);
            ft::slideri("Radar Range", &g_Options.Radar.range, 0, 500);
            ft::endcard();
        }
        else
        {
            ft::card("Display");
            ft::toggle("Show Waves", "Display radar waves", &g_Options.Radar.showWaves);
            ft::toggle("Animate Waves", "Animate radar waves", &g_Options.Radar.animateWaves);
            ft::toggle("Show Players", "Display players on radar", &g_Options.Radar.showPlayers);
            ft::toggle("Show Vehicles", "Display vehicles on radar", &g_Options.Radar.showVehicles);
            ft::toggle("Show Names", "Display player names on radar", &g_Options.Radar.showNames);
            ft::endcard();
        }
    }

    inline void radar_colors(int col)
    {
        if (col == 0)
        {
            ft::card("Radar Colors");
            ft::color("Background Color", "Radar background color", g_Options.Radar.backgroundColor);
            ft::color("Border Color", "Radar border color", g_Options.Radar.borderColor);
            ft::endcard();
        }
        else
        {
            ft::card("Player Colors");
            ft::color("Enemy Color", "Enemy player color", g_Options.Radar.enemyColor);
            ft::color("Friend Color", "Friendly player color", g_Options.Radar.friendColor);
            ft::endcard();
        }
    }

    inline void radar_around(int col)
    {
        if (col == 0)
        {
            ft::card("Players Around");
            ft::toggle("Enable", "Show players around you", &g_Options.PlayersAround.enable);
            ft::slideri("Distance (m)", &g_Options.PlayersAround.distance, 0, 500);
            ft::toggle("Show Distance", "Display distance to players", &g_Options.PlayersAround.showDistance);
            ft::endcard();
        }
        else
        {
            ft::card("Warnings");
            ft::toggle("Aiming At You", "Alert when a player aims at you", &g_Options.ESP.AimingAtYou);
            ft::color("Alert Color", "Colour of the aiming warning", g_Options.ESP.AimingAtYouColor);
            ft::endcard();
        }
    }

    // ============================== SETTINGS ==============================
    inline void settings(int col)
    {
        if (col == 0)
        {
            ft::card("Interface");
            { static const char* const layouts[] = { "Sidebar", "Topbar" }; ft::combo("Menu Layout", "Left sidebar rail or the classic top icon bar", &settings::layout, layouts, 2); }
            ft::endcard();
            ft::card("Settings");
            ft::toggle("StreamProof", "Hide menu from screen capture and streaming", &g_Options.General.StreamProof);
            ft::toggle("Hide on Alt-Tab", "Automatically hide menu when switching windows", &g_Options.General.HideOnAltTab);
            ft::toggle("Show On Features", "Show the on-screen list of enabled features", &g_Options.General.ShowActiveFeatures);
            // ft::keybind("Gui Key", "Key to toggle menu visibility", &g_Options.General.MenuKey);
            ft::endcard();
            ft::card("Menu");
            if (ft::button("Unload"))
                MGR->unload_prompt = true;   // opens the confirm splash (handled in menu_i.cpp)
            ft::endcard();
        }
        else
        {
            ft::card("Display");
            ft::toggle("Watermark", "Show watermark on screen", &settings::watermark);
            ft::color("Accent Color", "Change the menu accent color", settings::accent);
            ft::endcard();
            ft::card("Crosshair");
            ft::toggle("Enable Crosshair", "Draw a custom crosshair at screen center", &g_Options.Crosshair.enable);
            { int st = g_Options.Crosshair.style; static const char* const styles[] = { "Cross", "Cross + Dot", "Dot", "Circle" }; ft::combo("Style", "Crosshair style", &st, styles, 4); g_Options.Crosshair.style = st; }
            { int sz = (int)g_Options.Crosshair.size; ft::slideri("Size", &sz, 1, 30); g_Options.Crosshair.size = (float)sz; }
            { int th = (int)g_Options.Crosshair.thickness; ft::slideri("Thickness", &th, 1, 6); g_Options.Crosshair.thickness = (float)th; }
            { int gp = (int)g_Options.Crosshair.gap; ft::slideri("Gap", &gp, 0, 20); g_Options.Crosshair.gap = (float)gp; }
            ft::color("Color", "Crosshair colour", g_Options.Crosshair.color);
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
