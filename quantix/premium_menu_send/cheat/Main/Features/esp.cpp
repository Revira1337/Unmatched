#define IMGUI_DEFINE_MATH_OPERATORS
#include "esp.hpp"
#include <cmath>
#include <algorithm>
#include "../FivemSDK/Fivem.hpp"
#include <Main/FivemSDK/GTADefines.hpp>
#include "../Options.hpp"
#include "silent.hpp"
#include "../../../menu_new/defs/fonts.h"   // clean menu font for ESP name/weapon/distance
#include "../../../menu_new/defs/colors.h"  // c:: accent palette for the active-features card
#include <Utils/Render/Interface.hpp>       // menu-open state (drag only while the menu is open)

extern M_base::Interface interfaceObject;

namespace Cheat
{
    Vector3D Normalize(Vector3D vec) {
    float length = std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    if (length > 0.0f) {
        vec.x /= length;
        vec.y /= length;
        vec.z /= length;
    }
    return vec;
    }

    void DrawLineOutline(ImVec2 from, ImVec2 to, ImColor color, float thickness) {
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        // Draw main line only (no outline)
        drawList->AddLine(from, to, color, thickness);
    }


    void ESP::run()
    {
        if (!g_Options.ESP.enable)
            return;

        if (!C_Five.GetLocalPlayerInfo().Ped)
			return;

        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        if (!drawList)
            return;

        // Snapshot the view-projection once for this frame so the skeleton bones below
        // project via cached pure math instead of each re-reading the view matrix from
        // the game process (that per-bone read was a big cost with many players on).
        C_Five.CacheW2S();

        // Clean, minimalist font for ESP text (name / weapon / distance)
        ImFont* espFont = f::medium14 ? f::medium14 : ImGui::GetFont();
        const float ESP_FS = 13.f;

        // Cache all options at start - avoid repeated access
        const bool box = g_Options.ESP.Box;
        const bool skeleton = g_Options.ESP.Skeleton;
        const bool name = g_Options.ESP.Name;
        const bool distanceesp = g_Options.ESP.DistanceESP;
        const bool weapon = g_Options.ESP.WeaponName;
        const bool line = g_Options.ESP.SnapLines;
        const bool health = g_Options.ESP.HealthBar;
        const bool armor = g_Options.ESP.ArmorBar;
        const bool arrow = g_Options.ESP.Arrow;
        const bool headCircle = g_Options.ESP.HeadCircle;
        const bool friendshow = g_Options.ESP.ShowFriends;
        const bool Cvis = g_Options.ESP.visible;
        const bool drawBoxEnabled = g_Options.ESP.DrawBox.Enabled;
        const int maxdistance = g_Options.ESP.MaxDistance;
        
        // Pre-calculate colors once
        const ImColor boxcolor(g_Options.ESP.BoxColor[0], g_Options.ESP.BoxColor[1], g_Options.ESP.BoxColor[2], g_Options.ESP.BoxColor[3]);
        const ImColor skelcolor(g_Options.ESP.SkeletonColor[0], g_Options.ESP.SkeletonColor[1], g_Options.ESP.SkeletonColor[2], g_Options.ESP.SkeletonColor[3]);
        const ImColor DistanceColor(g_Options.ESP.DistanceColor[0], g_Options.ESP.DistanceColor[1], g_Options.ESP.DistanceColor[2], g_Options.ESP.DistanceColor[3]);
        const ImColor linecolor(g_Options.ESP.SnaplinesColor[0], g_Options.ESP.SnaplinesColor[1], g_Options.ESP.SnaplinesColor[2], g_Options.ESP.SnaplinesColor[3]);
        const ImColor weaponcolor(g_Options.ESP.WeaponNameColor[0], g_Options.ESP.WeaponNameColor[1], g_Options.ESP.WeaponNameColor[2], g_Options.ESP.WeaponNameColor[3]);
        const ImColor namecolor(g_Options.ESP.NameColor[0], g_Options.ESP.NameColor[1], g_Options.ESP.NameColor[2], g_Options.ESP.NameColor[3]);
        const ImColor healthcolor(g_Options.ESP.HealthBarColor[0], g_Options.ESP.HealthBarColor[1], g_Options.ESP.HealthBarColor[2], g_Options.ESP.HealthBarColor[3]);
        const ImColor armorcolor(g_Options.ESP.ArmorBarColor[0], g_Options.ESP.ArmorBarColor[1], g_Options.ESP.ArmorBarColor[2], g_Options.ESP.ArmorBarColor[3]);
        const ImColor ArrowColor(g_Options.ESP.ArrowColor[0], g_Options.ESP.ArrowColor[1], g_Options.ESP.ArrowColor[2], g_Options.ESP.ArrowColor[3]);
        const ImColor InvisibleColor(g_Options.ESP.InvisibleColor[0], g_Options.ESP.InvisibleColor[1], g_Options.ESP.InvisibleColor[2], g_Options.ESP.InvisibleColor[3]);
        const ImColor friendColor(g_Options.ESP.FriendColor[0], g_Options.ESP.FriendColor[1], g_Options.ESP.FriendColor[2], g_Options.ESP.FriendColor[3]);
        const ImColor headCircleColor(g_Options.ESP.HeadCircleColor[0], g_Options.ESP.HeadCircleColor[1], g_Options.ESP.HeadCircleColor[2], g_Options.ESP.HeadCircleColor[3]);
        
        const ImColor lockedColor(g_Options.ESP.LockedColor[0], g_Options.ESP.LockedColor[1], g_Options.ESP.LockedColor[2], g_Options.ESP.LockedColor[3]);
        const int lockedNetId = Cheat::Silent::GetLockedNetId();

        const float distance3 = g_Options.ESP.DistanceColor[3];
        const float weapon3 = g_Options.ESP.WeaponNameColor[3];
        const float name3 = g_Options.ESP.NameColor[3];
        const float ArrowRad = g_Options.ESP.ArrowRadius;
        const float headCircleRadius = g_Options.ESP.HeadCircleRadius;
        const float BoxWidthMultiplier = 1.2f;
        const float MaxHealth = 100.0f; // Current.Health is now effective 0-100 HP (see ResolveBars)
        const float MaxArmor = 100.0f;

        auto localplayer = C_Five.GetLocalPlayerInfo();
        const auto localpos = localplayer.WorldPos;
        
        const ImVec2 bottomCenter(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y);
        const ImVec2 screenCenter(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
        const float screenW = ImGui::GetIO().DisplaySize.x;
        const float screenH = ImGui::GetIO().DisplaySize.y;

        ImGuiIO& io = ImGui::GetIO();

        if (g_Options.Player.godtest)
        {
            float textScale = 1.35f;
            ImFont* font = ImGui::GetFont();
            ImVec2 textSize = ImGui::CalcTextSize(xorstr_("GODMODE ON"));
            textSize.x *= textScale;
            textSize.y *= textScale;
            float posX = (io.DisplaySize.x - textSize.x) * 0.5f;
            float posY = 25.0f;
            float paddingX = 15.0f;
            float paddingY = 8.0f;
            drawList->AddRectFilled(
                ImVec2(posX - paddingX, posY - paddingY),
                ImVec2(posX + textSize.x + paddingX, posY + textSize.y + paddingY),
                ImColor(0, 0, 0, 180), 5.0f
            );
        }

        std::lock_guard<std::mutex> lock(C_Five.GetLockLists());
        std::vector<Entity>& PedList = C_Five.GetEntitiyList();

        // Friend Add Key handling
        static bool friendKeyWasPressed = false;
        bool friendKeyPressed = g_Options.ESP.FriendAddMode && g_Options.ESP.FriendAddKey != 0 && GetAsyncKeyState(g_Options.ESP.FriendAddKey) & 0x8000;
        
        if (friendKeyPressed && !friendKeyWasPressed)
        {
            Entity* closestEntity = nullptr;
            float closestDist = FLT_MAX;
            
            for (Entity& ent : PedList)
            {
                if (ent.StaticInfo.bIsLocalPlayer) continue;
                ImVec2 entHead = ent.HeadPos;
                if (entHead.x <= 0 || entHead.y <= 0) continue;
                float dx = entHead.x - screenCenter.x;
                float dy = entHead.y - screenCenter.y;
                float dist = dx * dx + dy * dy; // Skip sqrt for comparison
                if (dist < closestDist && dist < 40000.f) // 200^2
                {
                    closestDist = dist;
                    closestEntity = &ent;
                }
            }
            
            if (closestEntity)
            {
                if (closestEntity->IsFriend)
                {
                    C_Five.FriendList.erase(closestEntity->StaticInfo.Ped);
                    C_Five.NetIdFriendList.erase(closestEntity->StaticInfo.NetId);
                    closestEntity->IsFriend = false;
                }
                else
                {
                    C_Five.FriendList[closestEntity->StaticInfo.Ped] = true;
                    C_Five.NetIdFriendList[closestEntity->StaticInfo.NetId] = true;
                    closestEntity->IsFriend = true;
                }
            }
        }
        friendKeyWasPressed = friendKeyPressed;

        for (Entity& Current : PedList)
        {
            if (Current.StaticInfo.bIsLocalPlayer)
                continue;

            const float distancessss = Current.Distance;
            if (distancessss > maxdistance)
                continue;

            const ImVec2 PedLocation = Current.PedLocation;
            if (PedLocation.x <= 0 || PedLocation.y <= 0)
                continue;

            const ImVec2 Head = Current.HeadPos;
            if (Head.x <= 0 || Head.y <= 0 || Head.x > screenW || Head.y > screenH)
                continue;

            const ImVec2 LeftFoot = Current.LeftFootPos;
            if (LeftFoot.x <= 0 || LeftFoot.y <= 0)
                continue;

            const ImVec2 RightFoot = Current.RightFootPos;
            if (RightFoot.x <= 0 || RightFoot.y <= 0)
                continue;

            const bool Visible = Current.Visible;
            const bool isLocked = (lockedNetId > 0 && Current.StaticInfo.NetId == lockedNetId);
            auto tint = [&](const ImColor& base) -> ImColor { if (isLocked) return lockedColor; if (Current.IsFriend) return friendColor; return base; };
            ImVec2 Padding[4] = { ImVec2(0, 0), ImVec2(0, 0), ImVec2(0, 0), ImVec2(0, 0) };


            float Height = (LeftFoot.y > RightFoot.y ? LeftFoot.y : RightFoot.y) - Head.y;
            float Width = Height / 1.8f;
            float PedCenterY = Head.y + Height * 0.5f;
            Height *= 1.2f;
            float NewWidth = Width * BoxWidthMultiplier;

            // Box edges — shared by bars + text slots so the in-game ESP matches the menu preview exactly.
            const float boxLeft   = PedLocation.x - NewWidth * 0.5f;
            const float boxRight  = PedLocation.x + NewWidth * 0.5f;
            const float boxTop    = PedCenterY - Height * 0.5f;
            const float boxBottom = PedCenterY + Height * 0.5f;
            const float boxCX     = PedLocation.x;
            // per-slot text stacking (0=head-top,1=upper-left,2=upper-right,3=lower-left,4=lower-right,5=bottom)
            float slotStack[6] = { 0,0,0,0,0,0 };

            // Advanced Box Drawing System - Optimized
            if (drawBoxEnabled) {
                ImColor color(g_Options.ESP.DrawBox.Color[0], g_Options.ESP.DrawBox.Color[1], g_Options.ESP.DrawBox.Color[2], g_Options.ESP.DrawBox.Color[3]);
                
                // Use friend/invisible color
                if (Current.IsFriend) {
                    color = friendColor;
                }
                else if (!Visible && Cvis) {
                    color = InvisibleColor;
                }
                
                if (isLocked) color = lockedColor;

                float reducedWidth = (NewWidth * 0.5f) * g_Options.ESP.DrawBox.Size;
                float pTop = PedCenterY - Height * 0.5f;
                float pBottom = PedCenterY + Height * 0.5f;
                
                int boxType = g_Options.ESP.DrawBox.SelectedType;
                
                if (boxType == 0) { // Full Box
                    if (g_Options.ESP.DrawBox.GradientEnabled) {
                        ImColor topColor, bottomColor;
                        float intensity = g_Options.ESP.DrawBox.GradientIntensity;
                        if (g_Options.ESP.DrawBox.UseCustomGradient) {
                            topColor = ImColor(g_Options.ESP.DrawBox.BoxGradientTopColor[0], g_Options.ESP.DrawBox.BoxGradientTopColor[1], g_Options.ESP.DrawBox.BoxGradientTopColor[2], g_Options.ESP.DrawBox.BoxGradientTopColor[3] * intensity);
                            bottomColor = ImColor(g_Options.ESP.DrawBox.BoxGradientBottomColor[0], g_Options.ESP.DrawBox.BoxGradientBottomColor[1], g_Options.ESP.DrawBox.BoxGradientBottomColor[2], g_Options.ESP.DrawBox.BoxGradientBottomColor[3] * intensity);
                        } else {
                            topColor = ImColor(color.Value.x * 0.6f, color.Value.y * 0.6f, color.Value.z * 0.6f, 0.25f * intensity);
                            bottomColor = ImColor(color.Value.x * 0.1f, color.Value.y * 0.1f, color.Value.z * 0.1f, 0.35f * intensity);
                        }
                        drawList->AddRectFilledMultiColor(
                            ImVec2(PedLocation.x - reducedWidth + 1, pTop + 1), 
                            ImVec2(PedLocation.x + reducedWidth - 1, pBottom - 1), 
                            ImGui::ColorConvertFloat4ToU32(topColor), ImGui::ColorConvertFloat4ToU32(topColor), 
                            ImGui::ColorConvertFloat4ToU32(bottomColor), ImGui::ColorConvertFloat4ToU32(bottomColor)
                        );
                    }
                    drawList->AddRect(ImVec2(PedLocation.x - reducedWidth, pTop), ImVec2(PedLocation.x + reducedWidth, pBottom), color, 0.f, 0, 1.f);
                }
                else if (boxType == 1) { // Corner Box
                    float bScale = Height * 0.25f;
                    if (g_Options.ESP.DrawBox.GradientEnabled) {
                        ImColor topColor, bottomColor;
                        float intensity = g_Options.ESP.DrawBox.GradientIntensity;
                        if (g_Options.ESP.DrawBox.UseCustomGradient) {
                            topColor = ImColor(g_Options.ESP.DrawBox.BoxGradientTopColor[0], g_Options.ESP.DrawBox.BoxGradientTopColor[1], g_Options.ESP.DrawBox.BoxGradientTopColor[2], g_Options.ESP.DrawBox.BoxGradientTopColor[3] * intensity);
                            bottomColor = ImColor(g_Options.ESP.DrawBox.BoxGradientBottomColor[0], g_Options.ESP.DrawBox.BoxGradientBottomColor[1], g_Options.ESP.DrawBox.BoxGradientBottomColor[2], g_Options.ESP.DrawBox.BoxGradientBottomColor[3] * intensity);
                        } else {
                            topColor = ImColor(color.Value.x * 0.6f, color.Value.y * 0.6f, color.Value.z * 0.6f, 0.25f * intensity);
                            bottomColor = ImColor(color.Value.x * 0.1f, color.Value.y * 0.1f, color.Value.z * 0.1f, 0.35f * intensity);
                        }
                        drawList->AddRectFilledMultiColor(
                            ImVec2(PedLocation.x - reducedWidth + 1, pTop + 1), 
                            ImVec2(PedLocation.x + reducedWidth - 1, pBottom - 1), 
                            ImGui::ColorConvertFloat4ToU32(topColor), ImGui::ColorConvertFloat4ToU32(topColor), 
                            ImGui::ColorConvertFloat4ToU32(bottomColor), ImGui::ColorConvertFloat4ToU32(bottomColor)
                        );
                    }
                    // Corners
                    float left = PedLocation.x - reducedWidth;
                    float right = PedLocation.x + reducedWidth;
                    drawList->AddLine(ImVec2(left, pTop), ImVec2(left + bScale, pTop), color, 1.f);
                    drawList->AddLine(ImVec2(right - bScale, pTop), ImVec2(right, pTop), color, 1.f);
                    drawList->AddLine(ImVec2(left, pTop), ImVec2(left, pTop + bScale), color, 1.f);
                    drawList->AddLine(ImVec2(right, pTop), ImVec2(right, pTop + bScale), color, 1.f);
                    drawList->AddLine(ImVec2(left, pBottom - bScale), ImVec2(left, pBottom), color, 1.f);
                    drawList->AddLine(ImVec2(right, pBottom - bScale), ImVec2(right, pBottom), color, 1.f);
                    drawList->AddLine(ImVec2(left, pBottom), ImVec2(left + bScale, pBottom), color, 1.f);
                    drawList->AddLine(ImVec2(right - bScale, pBottom), ImVec2(right, pBottom), color, 1.f);
                }
                else if (boxType == 2) { // 3D Box
                    float depth = 10.0f;
                    ImColor darkColor(color.Value.x * 0.5f, color.Value.y * 0.5f, color.Value.z * 0.5f, color.Value.w);
                    float left = PedLocation.x - reducedWidth;
                    float right = PedLocation.x + reducedWidth;
                    float top = Head.y;
                    float bottom = pBottom;
                    drawList->AddRect(ImVec2(left + depth, top - depth), ImVec2(right + depth, bottom - depth), ImColor(0.f, 0.f, 0.f, 0.3f), 0.f, 0, 1.5f);
                    drawList->AddLine(ImVec2(left, top), ImVec2(left + depth, top - depth), darkColor, 1.5f);
                    drawList->AddLine(ImVec2(right, top), ImVec2(right + depth, top - depth), darkColor, 1.5f);
                    drawList->AddLine(ImVec2(left, bottom), ImVec2(left + depth, bottom - depth), darkColor, 1.5f);
                    drawList->AddLine(ImVec2(right, bottom), ImVec2(right + depth, bottom - depth), darkColor, 1.5f);
                    drawList->AddRect(ImVec2(left, top), ImVec2(right, bottom), color, 0.f, 0, 2.0f);
                }
            }

            // Legacy box support — thin outline, Normal (full) or Dashed (corners only)
            if (box)
            {
                ImColor bxcol = boxcolor;
                if (!Visible && Cvis)      bxcol = InvisibleColor;
                else if (Current.IsFriend) bxcol = friendColor;

                if (isLocked) bxcol = lockedColor;

                const ImVec2 rMin(boxLeft, boxTop), rMax(boxRight, boxBottom);
                const ImColor shadow(0.f, 0.f, 0.f, 0.5f);

                if (!g_Options.ESP.BoxDashed)
                {
                    // Full box — crisp 1px line with a 1px dark backing for contrast
                    drawList->AddRect(rMin - ImVec2(1, 1), rMax + ImVec2(1, 1), shadow, 0.f, 0, 1.f);
                    drawList->AddRect(rMin, rMax, bxcol, 0.f, 0, 1.f);
                }
                else
                {
                    // Corner box — only the corners, thin and equal-length on every side
                    const float L = (rMax.x - rMin.x) * 0.22f;   // same length for horizontal & vertical strokes
                    const ImVec2 tl = rMin, tr(rMax.x, rMin.y), bl(rMin.x, rMax.y), br = rMax;
                    auto corners = [&](ImColor col, ImVec2 o) {
                        drawList->AddLine(tl + o, ImVec2(tl.x + L, tl.y) + o, col, 1.f); drawList->AddLine(tl + o, ImVec2(tl.x, tl.y + L) + o, col, 1.f);
                        drawList->AddLine(tr + o, ImVec2(tr.x - L, tr.y) + o, col, 1.f); drawList->AddLine(tr + o, ImVec2(tr.x, tr.y + L) + o, col, 1.f);
                        drawList->AddLine(bl + o, ImVec2(bl.x + L, bl.y) + o, col, 1.f); drawList->AddLine(bl + o, ImVec2(bl.x, bl.y - L) + o, col, 1.f);
                        drawList->AddLine(br + o, ImVec2(br.x - L, br.y) + o, col, 1.f); drawList->AddLine(br + o, ImVec2(br.x, br.y - L) + o, col, 1.f);
                    };
                    corners(shadow, ImVec2(1.f, 1.f));
                    corners(bxcol, ImVec2(0.f, 0.f));
                }
            }


			// Head Dot — hollow circle on the head, size scales with the player's on-screen size
			// Filled box (chams-like translucent 2D fill)
			if (g_Options.ESP.FilledBox)
			{
				ImColor fillcol(g_Options.ESP.FilledBoxColor[0], g_Options.ESP.FilledBoxColor[1], g_Options.ESP.FilledBoxColor[2], g_Options.ESP.FilledBoxColor[3]);
				if (isLocked) fillcol = ImColor(g_Options.ESP.LockedColor[0], g_Options.ESP.LockedColor[1], g_Options.ESP.LockedColor[2], g_Options.ESP.FilledBoxColor[3]);
				drawList->AddRectFilled(ImVec2(boxLeft, boxTop), ImVec2(boxRight, boxBottom), fillcol, 0.f);
			}

			if (headCircle)
			{
				// Match the ESP preview: a big, clean hollow ring with no dark outline.
				float hr = (boxBottom - boxTop) * 0.075f;
				if (hr < 3.f) hr = 3.f; else if (hr > 16.f) hr = 16.f;
				drawList->AddCircle(Head, hr, tint(headCircleColor), 32, 1.8f);
			}

			if (skeleton && Current.HasPrevBones)
            {
                // Determine color once
                ImColor lineColor = skelcolor;
                if (!Visible && Cvis)
                    lineColor = InvisibleColor;
                else if (Current.IsFriend)
                    lineColor = friendColor;
                
                if (isLocked) lineColor = lockedColor;
                const Vector3D& head = Current.HeadVEC;
                const Vector3D& neck = Current.NeckVEC;
                const Vector3D& pelvis = Current.PelvisVEC;
                
                // Quick validation - skip if core bones invalid
                if (head.x == 0.0f && head.y == 0.0f && head.z == 0.0f) continue;
                if (neck.x == 0.0f && neck.y == 0.0f && neck.z == 0.0f) continue;
                if (pelvis.x == 0.0f && pelvis.y == 0.0f && pelvis.z == 0.0f) continue;
                
                // Optimized draw function
                auto DrawBoneLine = [&](const Vector3D& from, const Vector3D& to) {
                    if (from.x == 0.0f && from.y == 0.0f) return;
                    if (to.x == 0.0f && to.y == 0.0f) return;
                    ImVec2 s_from = C_Five.WorldToScreenC(from);
                    ImVec2 s_to = C_Five.WorldToScreenC(to);
                    if (s_from.x < 1 || s_from.y < 1 || s_to.x < 1 || s_to.y < 1) return;
                    if (s_from.x > screenW - 1 || s_from.y > screenH - 1) return;
                    if (s_to.x > screenW - 1 || s_to.y > screenH - 1) return;
                    // Skip glitched lines
                    float dx = s_from.x - s_to.x;
                    float dy = s_from.y - s_to.y;
                    if (dx*dx + dy*dy > 250000.0f) return;
                    // colored line only — no dark outline behind it
                    drawList->AddLine(s_from, s_to, lineColor, 1.2f);
                };
                
                // Spine: head -> neck -> chest -> pelvis
                const Vector3D& chest = Current.ChestVEC;
                DrawBoneLine(head, neck);
                DrawBoneLine(neck, chest);
                DrawBoneLine(chest, pelvis);

                // Arms — shoulders connect to the NECK (not the chest)
                DrawBoneLine(neck, Current.L_UpperArmVEC);
                DrawBoneLine(Current.L_UpperArmVEC, Current.L_ForearmVEC);
                DrawBoneLine(Current.L_ForearmVEC, Current.L_HandVEC);
                DrawBoneLine(neck, Current.R_UpperArmVEC);
                DrawBoneLine(Current.R_UpperArmVEC, Current.R_ForearmVEC);
                DrawBoneLine(Current.R_ForearmVEC, Current.R_HandVEC);
                
                // Legs
                DrawBoneLine(pelvis, Current.L_ThighVEC);
                DrawBoneLine(Current.L_ThighVEC, Current.L_CalfVEC);
                DrawBoneLine(Current.L_CalfVEC, Current.LeftFootVEC);
                DrawBoneLine(pelvis, Current.R_ThighVEC);
                DrawBoneLine(Current.R_ThighVEC, Current.R_CalfVEC);
                DrawBoneLine(Current.R_CalfVEC, Current.RightFootVEC);
            }

            if (health)
            {
                float Health = Current.Health;
                if (Health < 0.0f) Health = 0.0f;
                if (Health > MaxHealth) Health = MaxHealth;

                // rounded, slightly thicker bars that keep a gap from the box
                const float BAR_TH = 3.0f, BAR_GAP = 4.0f, BAR_R = BAR_TH * 0.5f;
                ImVec2 DrawPos, DrawSize, bgPos, BackgroundSize;

                // Bar position — matches the ESP preview encoding: 0=Bottom, 1=Left, 2=Right (never Top)
                int healthPos = g_Options.ESP.HealthBarPosition;
                if (healthPos < 0 || healthPos > 2) healthPos = 2;

                if (healthPos == 1) {          // Left (vertical)
                    float h = (Height / MaxHealth) * Health;
                    float x = boxLeft - BAR_GAP - BAR_TH - Padding[1].x;
                    bgPos = ImVec2(x, boxTop); BackgroundSize = ImVec2(BAR_TH, Height);
                    DrawPos = ImVec2(x, boxBottom - h); DrawSize = ImVec2(BAR_TH, h);
                    Padding[1].x += BAR_TH + 2;
                }
                else if (healthPos == 2) {     // Right (vertical)
                    float h = (Height / MaxHealth) * Health;
                    float x = boxRight + BAR_GAP + Padding[3].x;
                    bgPos = ImVec2(x, boxTop); BackgroundSize = ImVec2(BAR_TH, Height);
                    DrawPos = ImVec2(x, boxBottom - h); DrawSize = ImVec2(BAR_TH, h);
                    Padding[3].x += BAR_TH + 2;
                }
                else {                         // 0 = Bottom (horizontal)
                    float wv = (NewWidth / MaxHealth) * Health;
                    float y = boxBottom + BAR_GAP + Padding[2].y;
                    bgPos = ImVec2(boxLeft, y); BackgroundSize = ImVec2(NewWidth, BAR_TH);
                    DrawPos = ImVec2(boxLeft, y); DrawSize = ImVec2(wv, BAR_TH);
                    Padding[2].y += BAR_TH + 2;
                }

                drawList->AddRectFilled(bgPos, bgPos + BackgroundSize, ImColor(0, 0, 0, 140), BAR_R);
                drawList->AddRect(bgPos - ImVec2(0.5f, 0.5f), bgPos + BackgroundSize + ImVec2(0.5f, 0.5f), ImColor(0, 0, 0, 180), BAR_R, 0, 1.f);
                drawList->AddRectFilled(DrawPos, DrawPos + DrawSize, isLocked ? lockedColor : healthcolor, BAR_R);
            }

            if (armor)
            {
                float Armor = Current.Armor;
                if (Armor < 0.0f) Armor = 0.0f;
                if (Armor > MaxArmor) Armor = MaxArmor;

                const float BAR_TH = 3.0f, BAR_GAP = 4.0f, BAR_R = BAR_TH * 0.5f;
                ImVec2 DrawPos, DrawSize, bgPos, BackgroundSize;

                // Bar position — matches the ESP preview encoding: 0=Bottom, 1=Left, 2=Right (never Top)
                int armorPos = g_Options.ESP.ArmorBarPosition;
                if (armorPos < 0 || armorPos > 2) armorPos = 2;

                if (armorPos == 1) {           // Left (vertical)
                    float h = (Height / MaxArmor) * Armor;
                    float x = boxLeft - BAR_GAP - BAR_TH - Padding[1].x;
                    bgPos = ImVec2(x, boxTop); BackgroundSize = ImVec2(BAR_TH, Height);
                    DrawPos = ImVec2(x, boxBottom - h); DrawSize = ImVec2(BAR_TH, h);
                    Padding[1].x += BAR_TH + 2;
                }
                else if (armorPos == 2) {      // Right (vertical)
                    float h = (Height / MaxArmor) * Armor;
                    float x = boxRight + BAR_GAP + Padding[3].x;
                    bgPos = ImVec2(x, boxTop); BackgroundSize = ImVec2(BAR_TH, Height);
                    DrawPos = ImVec2(x, boxBottom - h); DrawSize = ImVec2(BAR_TH, h);
                    Padding[3].x += BAR_TH + 2;
                }
                else {                         // 0 = Bottom (horizontal)
                    float wv = (NewWidth / MaxArmor) * Armor;
                    float y = boxBottom + BAR_GAP + Padding[2].y;
                    bgPos = ImVec2(boxLeft, y); BackgroundSize = ImVec2(NewWidth, BAR_TH);
                    DrawPos = ImVec2(boxLeft, y); DrawSize = ImVec2(wv, BAR_TH);
                    Padding[2].y += BAR_TH + 2;
                }

                drawList->AddRectFilled(bgPos, bgPos + BackgroundSize, ImColor(0, 0, 0, 140), BAR_R);
                drawList->AddRect(bgPos - ImVec2(0.5f, 0.5f), bgPos + BackgroundSize + ImVec2(0.5f, 0.5f), ImColor(0, 0, 0, 180), BAR_R, 0, 1.f);
                drawList->AddRectFilled(DrawPos, DrawPos + DrawSize, isLocked ? lockedColor : armorcolor, BAR_R);
            }

            // Places a text element into one of the 6 preview-matched slots (and stacks repeats).
            // slot: 0=head-top, 1=upper-left, 2=upper-right, 3=lower-left, 4=lower-right, 5=bottom
            auto placeText = [&](int slot, ImVec2 ts) -> ImVec2 {
                if (slot < 0 || slot > 5) slot = 0;
                ImVec2 p; int ha, va;
                switch (slot)
                {
                case 1: p = ImVec2(boxLeft  - 6.f - Padding[1].x, boxTop + 2.f);    ha = -1; va = +1; break; // upper-left
                case 2: p = ImVec2(boxRight + 6.f + Padding[3].x, boxTop + 2.f);    ha = +1; va = +1; break; // upper-right
                case 3: p = ImVec2(boxLeft  - 6.f - Padding[1].x, boxBottom - 2.f); ha = -1; va = -1; break; // lower-left
                case 4: p = ImVec2(boxRight + 6.f + Padding[3].x, boxBottom - 2.f); ha = +1; va = -1; break; // lower-right
                case 5: p = ImVec2(boxCX, boxBottom + 4.f + Padding[2].y);          ha =  0; va = +1; break; // bottom
                default: p = ImVec2(boxCX, boxTop - 4.f);                           ha =  0; va = -1; slot = 0; break; // head-top
                }
                float x = (ha < 0) ? p.x - ts.x : (ha > 0 ? p.x : p.x - ts.x * 0.5f);
                float baseY = (va < 0) ? p.y - ts.y : (va > 0 ? p.y : p.y - ts.y * 0.5f);
                float y = baseY + ((va < 0) ? -slotStack[slot] : slotStack[slot]);
                slotStack[slot] += ts.y + 2.f;
                return ImVec2(x, y);
            };


			if (name)
            {
                std::string Name = Current.StaticInfo.Name;

                if (Name.empty())
                    Name = xorstr_("Unknown");

                ImVec2 TextSize = espFont->CalcTextSizeA(ESP_FS, FLT_MAX, -1.f, Name.c_str(), 0, NULL);
                ImVec2 DrawPos = placeText(g_Options.ESP.NamePosition, TextSize);

                // FRIEND tag stacks in the same slot as the name
                if (Current.IsFriend && friendshow)
                {
                    std::string friendTag = xorstr_("[FRIEND]");
                    ImVec2 FriendTextSize = espFont->CalcTextSizeA(ESP_FS, FLT_MAX, -1.f, friendTag.c_str(), 0, NULL);
                    ImVec2 FriendDrawPos = placeText(g_Options.ESP.NamePosition, FriendTextSize);

                    // Shadow effect
                    drawList->AddText(espFont, ESP_FS, FriendDrawPos + ImVec2(1, 1), ImColor(0.f, 0.f, 0.f, 0.5f), friendTag.c_str());
                    drawList->AddText(espFont, ESP_FS, FriendDrawPos, friendColor, friendTag.c_str());
                }

                // Shadow effect
                drawList->AddText(espFont, ESP_FS, DrawPos + ImVec2(1, 1), ImColor(0.f, 0.f, 0.f, name3 * 0.5f), Name.c_str());
                // Use friend color for name if player is a friend (always, regardless of ShowFriends setting)
                if (Current.IsFriend)
                    drawList->AddText(espFont, ESP_FS, DrawPos, tint(namecolor), Name.c_str());
                else
                    drawList->AddText(espFont, ESP_FS, DrawPos, tint(namecolor), Name.c_str());
            }

            if (weapon)
            {
                CWeaponManager* WeaponManager = Current.StaticInfo.Ped->GetWeaponManager();
                if (WeaponManager)
                {
                    CWeaponInfo* WeaponInfo = WeaponManager->GetWeaponInfo();
                    if (WeaponInfo)
                    {
                        std::string WeaponName = WeaponInfo->GetWeaponName();
                        if (WeaponName.size() > 0)
                        {
                            ImVec2 TextSize = espFont->CalcTextSizeA(ESP_FS, FLT_MAX, -1.f, WeaponName.c_str(), 0, NULL);
                            ImVec2 DrawPos = placeText(g_Options.ESP.WeaponPosition, TextSize);

                            // Shadow effect
                            drawList->AddText(espFont, ESP_FS, DrawPos + ImVec2(1, 1), ImColor(0.f, 0.f, 0.f, weapon3 * 0.5f), WeaponName.c_str());
                            drawList->AddText(espFont, ESP_FS, DrawPos, tint(weaponcolor), WeaponName.c_str());
                        }
                    }
                }
            }

            if (distanceesp)
            {
                if (!Current.StaticInfo.bIsLocalPlayer)
                {
                    std::string distancetext = "[" + std::to_string((int)distancessss) + xorstr_("m") + "]";

                    ImVec2 TextSize = espFont->CalcTextSizeA(ESP_FS, FLT_MAX, -1.f, distancetext.c_str(), 0, NULL);
                    ImVec2 DrawPos = placeText(g_Options.ESP.DistancePosition, TextSize);

                    // Shadow effect
                    drawList->AddText(espFont, ESP_FS, DrawPos + ImVec2(1, 1), ImColor(0.f, 0.f, 0.f, distance3 * 0.5f), distancetext.c_str());
                    drawList->AddText(espFont, ESP_FS, DrawPos, tint(DistanceColor), distancetext.c_str());
                }
            }

            // Show ID — the player's server/network id (position follows IDPosition slot)
            if (g_Options.ESP.ShowID)
            {
                if (!Current.StaticInfo.bIsLocalPlayer && Current.StaticInfo.NetId > 0)
                {
                    std::string idText = "#" + std::to_string(Current.StaticInfo.NetId);
                    ImVec2 TextSize = espFont->CalcTextSizeA(ESP_FS, FLT_MAX, -1.f, idText.c_str(), 0, NULL);
                    ImVec2 DrawPos = placeText(g_Options.ESP.IDPosition, TextSize);
                    ImColor idcol(g_Options.ESP.IDColor[0], g_Options.ESP.IDColor[1], g_Options.ESP.IDColor[2], g_Options.ESP.IDColor[3]);
                    drawList->AddText(espFont, ESP_FS, DrawPos + ImVec2(1, 1), ImColor(0.f, 0.f, 0.f, 0.5f), idText.c_str());
                    drawList->AddText(espFont, ESP_FS, DrawPos, tint(idcol), idText.c_str());
                }
            }

            if (line)
            {
                if (!Current.StaticInfo.bIsLocalPlayer)
                {
                    drawList->AddLine(bottomCenter, PedLocation, isLocked ? lockedColor : linecolor);
                }
            }
        }
    }


    // short, human-friendly key label for the active-features card
    static std::string VkName(int vk)
    {
        switch (vk)
        {
            case VK_LBUTTON: return "M1"; case VK_RBUTTON: return "M2"; case VK_MBUTTON: return "M3";
            case VK_XBUTTON1: return "M4"; case VK_XBUTTON2: return "M5";
            case VK_LSHIFT: case VK_SHIFT: return "Shift"; case VK_RSHIFT: return "RShift";
            case VK_LCONTROL: case VK_CONTROL: return "Ctrl"; case VK_RCONTROL: return "RCtrl";
            case VK_LMENU: case VK_MENU: return "Alt"; case VK_RMENU: return "RAlt";
            case VK_SPACE: return "Space"; case VK_TAB: return "Tab"; case VK_RETURN: return "Enter";
            case VK_CAPITAL: return "Caps"; case VK_ESCAPE: return "Esc";
            case VK_UP: return "Up"; case VK_DOWN: return "Down"; case VK_LEFT: return "Left"; case VK_RIGHT: return "Right";
        }
        if (vk >= '0' && vk <= '9') return std::string(1, (char)vk);
        if (vk >= 'A' && vk <= 'Z') return std::string(1, (char)vk);
        if (vk >= VK_F1 && vk <= VK_F12) return "F" + std::to_string(vk - VK_F1 + 1);
        if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9) return "Num" + std::to_string(vk - VK_NUMPAD0);
        return "Key";
    }

    void ESP::DrawActiveFeatures()
    {
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        if (!drawList)
            return;

        // snapshot the view-projection once for all cached WorldToScreenC() calls below
        C_Five.CacheW2S();

        ImVec2 displaySize = ImGui::GetIO().DisplaySize;
        
        // ---- Active features card (draggable while the menu is open; shows keybinds) ----
        ImColor shadowColor(0.0f, 0.0f, 0.0f, 0.5f);   // reused by the Freecam / StreamProof labels below
        if (g_Options.General.ShowActiveFeatures)
        {
            struct FRow { const char* name; int key; };
            std::vector<FRow> rows;
            auto add = [&](bool on, const char* name, int key) { if (on) rows.push_back({ name, key }); };
            add(g_Options.Aimbot.enable,        "Aimbot",         g_Options.Aimbot.key);
            add(g_Options.Silent.enable,        "Silent Aim",     g_Options.Silent.key);
            add(g_Options.Silent.Magic,         "Magic Bullet",   g_Options.Silent.Magic_key);
            add(g_Options.Triggerbot.enable,    "Triggerbot",     g_Options.Triggerbot.key);
            add(g_Options.ESP.enable,           "ESP",            0);
            add(g_Options.Radar.enable,         "Radar",          0);
            add(g_Options.Player.God,           "Godmode",        g_Options.Player.GodKey);
            add(g_Options.Player.SemiGod,       "Semi-God",       g_Options.Player.SemiGodKey);
            add(g_Options.Player.Passive,       "Immortality",    g_Options.Player.PassiveKey);
            add(g_Options.Exploit.KediciShadow, "Invisible",      g_Options.Exploit.KediciShadowKey);
            add(g_Options.Player.noclip,        "Noclip",         g_Options.Player.NoclipKey);
            add(g_Options.Player.Freecam,       "Freecam",        g_Options.Player.FreecamKey);
            add(g_Options.Player.Nocollision,   "No Collision",   0);
            add(g_Options.Player.NoFallDamage,  "No Fall Damage", 0);
            add(g_Options.Player.PSpeed,        "Player Speed",   0);
            add(g_Options.Exploit.StrafeMacro,  "Strafe Macro",   g_Options.Exploit.StrafeMacroBind);
            add(g_Options.Player.NoRecoil,      "No Recoil",      0);
            add(g_Options.Player.NoSpread,      "No Spread",      0);
            add(g_Options.Player.NoReload,      "No Reload",      0);
            add(g_Options.Player.RangeModify,   "Range Mod",      0);
            add(g_Options.Player.DamageBoost,   "Damage Boost",   0);
            add(g_Options.Player.InfiniteAmmo,  "Infinite Ammo",  0);
            add(g_Options.Player.VehicleGod,    "Veh Godmode",    0);
            add(g_Options.Exploit.stealcar,     "Steal Car",      0);
            add(g_Options.Player.Absorption,    "Absorption",     0);

            if (!rows.empty())
            {
                ImFont* uf = f::medium14 ? f::medium14 : ImGui::GetFont();
                ImFont* sf = f::medium12 ? f::medium12 : ImGui::GetFont();
                const float FS = 12.5f, KFS = 11.f;
                const float rowH = 19.f, padX = 11.f, headerH = 23.f, padY = 6.f;

                float maxNameW = 0.f, maxKeyW = 0.f;
                for (auto& r : rows)
                {
                    maxNameW = ImMax(maxNameW, uf->CalcTextSizeA(FS, FLT_MAX, -1.f, r.name, 0, NULL).x);
                    if (r.key > 0) { std::string kn = VkName(r.key); maxKeyW = ImMax(maxKeyW, sf->CalcTextSizeA(KFS, FLT_MAX, -1.f, kn.c_str(), 0, NULL).x + 12.f); }
                }
                float panelW = padX * 2.f + maxNameW + (maxKeyW > 0.f ? 16.f + maxKeyW : 0.f);
                if (panelW < 148.f) panelW = 148.f;
                float panelH = headerH + padY * 2.f + (float)rows.size() * rowH;

                static float px = -1.f, py = -1.f;      // default: middle-right of the screen (set once)
                if (px < 0.f) { px = displaySize.x - panelW - 18.f; py = (displaySize.y - panelH) * 0.5f; }
                static bool dragging = false;
                bool menuOpen = interfaceObject.GetMenuOpen();
                ImGuiIO& io = ImGui::GetIO();
                ImVec2 mp = io.MousePos;
                bool over = (mp.x >= px && mp.x <= px + panelW && mp.y >= py && mp.y <= py + panelH);
                if (menuOpen && over && io.MouseDown[0] && !dragging) dragging = true;
                if (!io.MouseDown[0]) dragging = false;
                if (dragging && menuOpen) { px += io.MouseDelta.x; py += io.MouseDelta.y; }
                px = ImClamp(px, 0.f, displaySize.x - panelW);
                py = ImClamp(py, 0.f, displaySize.y - panelH);

                ImVec2 p0(px, py), p1(px + panelW, py + panelH);
                const float rnd = 8.f;
                // palette matched to the Players-Around panel
                ImColor cardBg(c::bg.Value.x, c::bg.Value.y, c::bg.Value.z, 0.96f);
                ImColor stripe(1.f, 1.f, 1.f, 0.03f);
                ImColor border   = c::white12;
                ImColor titleCol = c::white72;
                ImColor accent   = c::primary;
                ImColor accentDim(accent.Value.x, accent.Value.y, accent.Value.z, 0.14f);

                drawList->AddShadowRect(p0, p1, ImColor(0.f, 0.f, 0.f, 0.55f), 30.f, ImVec2(0.f, 5.f), 0, rnd);
                drawList->AddRectFilled(p0, p1, cardBg, rnd);
                drawList->AddRectFilled(p0, ImVec2(p1.x, py + headerH), stripe, rnd, ImDrawFlags_RoundCornersTop);
                drawList->AddLine(ImVec2(p0.x + rnd, py + 1.f), ImVec2(p1.x - rnd, py + 1.f), ImColor(1.f, 1.f, 1.f, 0.06f), 1.f);
                drawList->AddLine(ImVec2(p0.x, py + headerH), ImVec2(p1.x, py + headerH), border, 1.f);
                drawList->AddRect(p0, p1, border, rnd, 0, 1.4f);
                drawList->AddRectFilled(p0, ImVec2(px + 3.f, py + headerH), accent, rnd, ImDrawFlags_RoundCornersLeft);

                drawList->AddText(sf, 11.f, ImVec2(px + padX, py + (headerH - 11.f) * 0.5f - 1.f), titleCol, "ACTIVE");
                {
                    std::string cs = std::to_string((int)rows.size());
                    ImVec2 css = sf->CalcTextSizeA(11.f, FLT_MAX, -1.f, cs.c_str(), 0, NULL);
                    float bw = css.x + 12.f, bh = 15.f;
                    ImVec2 bpos(p1.x - 9.f - bw, py + (headerH - bh) * 0.5f);
                    drawList->AddRectFilled(bpos, ImVec2(bpos.x + bw, bpos.y + bh), accentDim, bh * 0.5f);
                    drawList->AddText(sf, 11.f, ImVec2(bpos.x + (bw - css.x) * 0.5f, bpos.y + (bh - css.y) * 0.5f), accent, cs.c_str());
                }

                float ry = py + headerH + padY;
                for (auto& r : rows)
                {
                    drawList->AddText(uf, FS, ImVec2(px + padX, ry + (rowH - FS) * 0.5f - 1.f), ImColor(255, 255, 255, 255), r.name);
                    if (r.key > 0)
                    {
                        std::string kn = VkName(r.key);
                        ImVec2 ks = sf->CalcTextSizeA(KFS, FLT_MAX, -1.f, kn.c_str(), 0, NULL);
                        ImVec2 k1(p1.x - padX, ry + rowH - 3.5f), k0(k1.x - ks.x - 10.f, ry + 3.5f);
                        drawList->AddRectFilled(k0, k1, accentDim, 4.f);
                        drawList->AddText(sf, KFS, ImVec2(k0.x + 5.f, ry + (rowH - ks.y) * 0.5f - 0.5f), accent, kn.c_str());
                    }
                    ry += rowH;
                }
            }
        }
        
        // Freecam - crosshair'in altında küçük yazı
        if (g_Options.Player.Freecam) {
            ImVec2 center(displaySize.x * 0.5f, displaySize.y * 0.5f);
            const char* freecamText = "Freecam";
            ImVec2 textSize = ImGui::CalcTextSize(freecamText);
            ImVec2 textPos(center.x - textSize.x * 0.5f, center.y + 25.0f);
            
            ImColor whiteColor(1.0f, 1.0f, 1.0f, 1.0f); // Beyaz
            drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), shadowColor, freecamText);
            drawList->AddText(textPos, whiteColor, freecamText);
        }
        
        // StreamProof - sağ alt köşe
        if (g_Options.General.StreamProof) {
            const char* streamText = "StreamProof";
            ImVec2 textSize = ImGui::CalcTextSize(streamText);
            ImVec2 textPos(displaySize.x - textSize.x - 15.0f, displaySize.y - textSize.y - 30.0f);
            
            ImColor streamColor(1.0f, 1.0f, 1.0f, 0.85f); // Beyaz, az saydam
            drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), shadowColor, streamText);
            drawList->AddText(textPos, streamColor, streamText);
        }

        // ---- Custom crosshair ----
        if (g_Options.Crosshair.enable)
        {
            const ImVec2 cc(displaySize.x * 0.5f, displaySize.y * 0.5f);
            ImColor xcol(g_Options.Crosshair.color[0], g_Options.Crosshair.color[1], g_Options.Crosshair.color[2], g_Options.Crosshair.color[3]);
            const float sz = g_Options.Crosshair.size, th = g_Options.Crosshair.thickness, gp = g_Options.Crosshair.gap;
            const int st = g_Options.Crosshair.style;
            if (st == 0 || st == 1) {
                drawList->AddLine(ImVec2(cc.x - gp - sz, cc.y), ImVec2(cc.x - gp, cc.y), xcol, th);
                drawList->AddLine(ImVec2(cc.x + gp, cc.y), ImVec2(cc.x + gp + sz, cc.y), xcol, th);
                drawList->AddLine(ImVec2(cc.x, cc.y - gp - sz), ImVec2(cc.x, cc.y - gp), xcol, th);
                drawList->AddLine(ImVec2(cc.x, cc.y + gp), ImVec2(cc.x, cc.y + gp + sz), xcol, th);
            }
            if (st == 1 || st == 2) drawList->AddCircleFilled(cc, th, xcol);
            if (st == 3) drawList->AddCircle(cc, sz, xcol, 32, th);
        }

        // ---- Vehicle ESP (clean stacked layout — same font as player ESP) ----
        //   Name
        // ----------   health bar
        //   [24m]
        if (g_Options.ESP.VehicleESP)
        {
            ImFont* vFont = f::medium14 ? f::medium14 : ImGui::GetFont();
            const float V_FS = 13.f;
            const bool wantName = g_Options.ESP.VehicleName;
            const bool wantDist = g_Options.ESP.VehicleDistance;
            const bool wantHP   = g_Options.ESP.VehicleHealthBar;
            const int  vmax     = g_Options.ESP.VehicleMaxDistance;
            ImColor vcol(g_Options.ESP.VehicleColor[0], g_Options.ESP.VehicleColor[1], g_Options.ESP.VehicleColor[2], g_Options.ESP.VehicleColor[3]);

            // Snapshot only what we need, holding the lock for microseconds (cached
            // projection = pure math, no game reads), then draw fully unlocked so we
            // never stall behind the vehicle-list rebuild thread. This is what keeps
            // the framerate up with many vehicles on screen.
            struct VDraw { ImVec2 sp; float health; int dist; std::string name; };
            static std::vector<VDraw> vsnap;
            vsnap.clear();
            try {
                std::lock_guard<std::mutex> lock(C_Five.GetLockLists2());
                auto& vl = C_Five.GetVehicleList();
                vsnap.reserve(vl.size());
                for (auto& veh : vl)
                {
                    if (!veh.Vehicle) continue;
                    if (veh.Distance < 0 || veh.Distance > vmax) continue;
                    ImVec2 sp = C_Five.WorldToScreenC(veh.Cordinates);
                    if (sp.x <= 0.f || sp.y <= 0.f) continue;
                    VDraw d; d.sp = sp; d.health = veh.Health; d.dist = veh.Distance;
                    if (wantName) d.name = veh.Name;
                    vsnap.push_back(std::move(d));
                }
            } catch (...) {}

            for (auto& d : vsnap)
            {
                // health-bar width scales down with distance
                float barW = 3000.f / (float)(d.dist + 1);
                if (barW < 34.f) barW = 34.f; else if (barW > 110.f) barW = 110.f;
                const float barTh = 3.5f, barR = barTh * 0.5f;
                const float barLeft = d.sp.x - barW * 0.5f, barRight = d.sp.x + barW * 0.5f, barY = d.sp.y;

                // health bar (middle)
                if (wantHP) {
                    float hp = d.health; if (hp < 0.f) hp = 0.f; else if (hp > 1000.f) hp = 1000.f;
                    float frac = hp / 1000.f;
                    ImVec2 bg0(barLeft, barY - barR), bg1(barRight, barY + barR);
                    drawList->AddRectFilled(bg0, bg1, ImColor(0, 0, 0, 150), barR);
                    drawList->AddRect(bg0 - ImVec2(0.5f, 0.5f), bg1 + ImVec2(0.5f, 0.5f), ImColor(0, 0, 0, 180), barR, 0, 1.f);
                    ImColor hcol(1.f - frac, 0.35f + 0.55f * frac, 0.20f, 1.f);   // red -> green
                    drawList->AddRectFilled(bg0, ImVec2(barLeft + barW * frac, barY + barR), hcol, barR);
                }

                // name (top, centered)
                if (wantName && !d.name.empty()) {
                    ImVec2 ts = vFont->CalcTextSizeA(V_FS, FLT_MAX, -1.f, d.name.c_str(), 0, NULL);
                    ImVec2 tp(d.sp.x - ts.x * 0.5f, barY - barR - ts.y - 3.f);
                    drawList->AddText(vFont, V_FS, tp + ImVec2(1, 1), ImColor(0, 0, 0, 160), d.name.c_str());
                    drawList->AddText(vFont, V_FS, tp, vcol, d.name.c_str());
                }

                // distance (bottom, centered)
                if (wantDist) {
                    char db[24]; snprintf(db, sizeof db, "[%dm]", d.dist);
                    ImVec2 ts = vFont->CalcTextSizeA(V_FS, FLT_MAX, -1.f, db, 0, NULL);
                    ImVec2 tp(d.sp.x - ts.x * 0.5f, barY + barR + 3.f);
                    drawList->AddText(vFont, V_FS, tp + ImVec2(1, 1), ImColor(0, 0, 0, 160), db);
                    drawList->AddText(vFont, V_FS, tp, vcol, db);
                }
            }
        }

        // ---- Aiming At You alert ----
        if (g_Options.ESP.AimingAtYou)
        {
            auto lp = C_Five.GetLocalPlayerInfo();
            if (lp.Ped)
            {
                Vector3D myPos = lp.WorldPos;
                bool anyAim = false;
                try {
                    std::lock_guard<std::mutex> lock(C_Five.GetLockLists());
                    for (auto& ent : C_Five.GetEntitiyList())
                    {
                        if (ent.StaticInfo.bIsLocalPlayer) continue;
                        CPed* ped = ent.StaticInfo.Ped;
                        if (!ped) continue;
                        if (!ped->HasConfigFlag(CPED_CONFIG_FLAG_IsAimingGun)) continue;
                        Vector3D pp = ent.Cordinates;
                        Vector3D fwd = M_base::mem::Read<Vector3D>((uintptr_t)ped + 0x70);
                        Vector3D to = { myPos.x - pp.x, myPos.y - pp.y, myPos.z - pp.z };
                        float tl = sqrtf(to.x * to.x + to.y * to.y + to.z * to.z);
                        float fl = sqrtf(fwd.x * fwd.x + fwd.y * fwd.y + fwd.z * fwd.z);
                        if (tl < 0.01f || fl < 0.01f) continue;
                        float dot = (to.x * fwd.x + to.y * fwd.y + to.z * fwd.z) / (tl * fl);
                        if (dot > 0.90f) { anyAim = true; break; }
                    }
                } catch (...) {}
                if (anyAim)
                {
                    ImColor acol(g_Options.ESP.AimingAtYouColor[0], g_Options.ESP.AimingAtYouColor[1], g_Options.ESP.AimingAtYouColor[2], g_Options.ESP.AimingAtYouColor[3]);
                    const char* txt = "! AIMING AT YOU !";
                    ImVec2 ts = ImGui::CalcTextSize(txt);
                    ImVec2 tp(displaySize.x * 0.5f - ts.x * 0.5f, displaySize.y * 0.20f);
                    drawList->AddText(tp + ImVec2(1, 1), ImColor(0, 0, 0, 180), txt);
                    drawList->AddText(tp, acol, txt);
                }
            }
        }

        // ---- Ghost Peek marker: shows where you'll teleport back to ----
        if (g_Options.Player.GhostPeekEnabled && g_Options.Player.HasGhostPeekPosition)
        {
            ImVec2 sp = C_Five.WorldToScreen(g_Options.Player.GhostPeekPosition);
            if (sp.x > 0.f && sp.y > 0.f)
            {
                ImColor mcol(0.3f, 1.0f, 0.5f, 1.0f);
                drawList->AddLine(ImVec2(sp.x, sp.y), ImVec2(sp.x, sp.y - 24.f), mcol, 2.f);
                drawList->AddCircleFilled(sp, 4.f, mcol);
                drawList->AddCircle(sp, 8.f, ImColor(0, 0, 0, 160), 16, 3.f);
                drawList->AddCircle(sp, 8.f, mcol, 16, 1.5f);
                const char* gt = "Ghost Peek";
                ImVec2 gts = ImGui::CalcTextSize(gt);
                ImVec2 gtp(sp.x - gts.x * 0.5f, sp.y - 40.f);
                drawList->AddText(gtp + ImVec2(1, 1), ImColor(0, 0, 0, 160), gt);
                drawList->AddText(gtp, mcol, gt);
            }
        }
    }

    // Floating damage number over the last player we hit + a crosshair hit-marker X.
    // Damage is attributed heuristically: an enemy losing health while we hold fire = our hit.
    // Only ONE damage popup is shown at a time (it accumulates on the same target for a moment).
    void ESP::DrawDamageHitmarker()
    {
        const bool showDmg = g_Options.ESP.ShowDamage;
        const bool hitMark = g_Options.ESP.HitMarker;
        if (!showDmg && !hitMark) return;

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        if (!dl) return;

        double now = ImGui::GetTime();
        static std::unordered_map<int, float> prevHp;
        static int    dmgNetId = -1;
        static float  dmgAmount = 0.f;
        static double dmgTime = -10.0;
        static double hitTime = -10.0;

        ImVec2 headForNet(0.f, 0.f); bool haveHead = false;
        {
            std::lock_guard<std::mutex> lock(C_Five.GetLockLists());
            for (auto& e : C_Five.GetEntitiyList())
            {
                if (e.StaticInfo.bIsLocalPlayer) continue;
                int nid = e.StaticInfo.NetId;
                float h = e.Health;
                auto it = prevHp.find(nid);
                if (it != prevHp.end())
                {
                    float delta = it->second - h;              // positive = took damage (same 0-100 HP the health bar uses)
                    if (delta >= 1.f && h >= 0.f)
                    {
                        if (dmgNetId == nid && now - dmgTime < 1.0) dmgAmount += delta;  // accumulate a burst
                        else { dmgNetId = nid; dmgAmount = delta; }
                        dmgTime = now;
                        hitTime = now;
                    }
                    it->second = h;
                }
                else prevHp[nid] = h;

                if (nid == dmgNetId) { headForNet = e.HeadPos; haveHead = (headForNet.x > 0.f && headForNet.y > 0.f); }
            }
        }

        // ---- single damage popup above the target's head ----
        if (showDmg && dmgNetId >= 0)
        {
            double age = now - dmgTime;
            const double LIFE = 1.4;
            if (age < LIFE && haveHead)
            {
                float A = (float)(1.0 - age / LIFE);
                float rise = (float)age * 22.f;
                ImFont* fnt = f::sbold16 ? f::sbold16 : ImGui::GetFont();   // semibold = slightly bold
                std::string buf = "-" + std::to_string((int)(dmgAmount + 0.5f));
                int szMul = g_Options.ESP.ShowDamageSize; if (szMul < 1) szMul = 1; else if (szMul > 10) szMul = 10;
                const float fs = 22.f * (float)szMul;   // 1 = current size ... 10 = 10x
                ImVec2 ts = fnt->CalcTextSizeA(fs, FLT_MAX, -1.f, buf.c_str(), 0, NULL); (void)ts;
                // next to the head (to the right), floating up as it fades
                ImVec2 tp(headForNet.x + 14.f, headForNet.y - fs * 0.5f - rise);
                dl->AddText(fnt, fs, ImVec2(tp.x + 1.f, tp.y + 1.f), ImColor(0.f, 0.f, 0.f, 0.6f * A), buf.c_str());
                dl->AddText(fnt, fs, tp, ImColor(1.f, 0.28f, 0.28f, A), buf.c_str());
            }
        }

        // ---- crosshair hit-marker X ----
        if (hitMark)
        {
            double age = now - hitTime;
            const double LIFE = 0.35;
            if (age < LIFE)
            {
                float A = (float)(1.0 - age / LIFE);
                ImVec2 cc(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
                const float gap = 5.f, len = 8.f, th = 2.f;
                ImColor xc(1.f, 1.f, 1.f, A), xs(0.f, 0.f, 0.f, 0.5f * A);
                const ImVec2 dirs[4] = { ImVec2(-1, -1), ImVec2(1, -1), ImVec2(-1, 1), ImVec2(1, 1) };
                for (auto& d : dirs)
                {
                    ImVec2 a0(cc.x + d.x * gap, cc.y + d.y * gap);
                    ImVec2 a1(cc.x + d.x * (gap + len), cc.y + d.y * (gap + len));
                    dl->AddLine(ImVec2(a0.x + 1.f, a0.y + 1.f), ImVec2(a1.x + 1.f, a1.y + 1.f), xs, th);
                    dl->AddLine(a0, a1, xc, th);
                }
            }
        }

        if (prevHp.size() > 512) prevHp.clear();   // keep the tracking map bounded
    }

    void ESP::Render()
    {
        run();
        DrawActiveFeatures();
        DrawDamageHitmarker();
    }
}

