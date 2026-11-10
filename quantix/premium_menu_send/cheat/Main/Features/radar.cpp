#define IMGUI_DEFINE_MATH_OPERATORS
#include "radar.hpp"
#include <Main/Cheat.hpp>
#include <Utils/Render/Interface.hpp>
#include <cmath>
#include <string>
#include <algorithm>

// menu palette (accent-synced, inline globals) + fonts, so the overlays match the menu
#include "../../../menu_new/defs/colors.h"
#include "../../../menu_new/defs/fonts.h"

extern M_base::Interface interfaceObject;
#define M_PI 3.14159265358979323846

namespace Cheat {
namespace Radar {
    // multiply an ImColor's alpha (menu-style translucent layering)
    static inline ImColor col_a(ImColor col, float alpha) { col.Value.w *= alpha; return col; }
    // menu fonts with a safe fallback in case the atlas isn't built yet
    static inline ImFont* ui_font() { return f::medium14 ? f::medium14 : ImGui::GetFont(); }
    static inline ImFont* sm_font() { return f::medium12 ? f::medium12 : ImGui::GetFont(); }
    static Vector3D cachedCameraPos = Vector3D(0, 0, 0);
    static Vector3D cachedPlayerPos = Vector3D(0, 0, 0);
    static float cachedHeading = 0.0f;
    static float targetHeading = 0.0f;
    static float animationTime = 0.0f;
    static int frameSkip = 0;
    static const int FRAME_SKIP_COUNT = 2;

    float GetPlayerHeading() {
        if (frameSkip <= 0) {
            frameSkip = FRAME_SKIP_COUNT;
            CCamGameplayDirector* pCamGameplayDirector = (CCamGameplayDirector*)M_base::mem::Read<uint64_t>(C_Five.Camera);
            if (!pCamGameplayDirector) return cachedHeading;
            CCamFollowPedCamera* FollowPedCamera = pCamGameplayDirector->GetFollowPedCamera();
            if (!FollowPedCamera) return cachedHeading;
            cachedCameraPos = FollowPedCamera->GetCameraPosition();
            cachedPlayerPos = C_Five.GetLocalPlayerInfo().WorldPos;
            float dx = cachedCameraPos.x - cachedPlayerPos.x;
            float dy = cachedCameraPos.y - cachedPlayerPos.y;
            float headingRadians = atan2(dy, dx);
            float headingDegrees = headingRadians * (180.0f / M_PI);
            headingDegrees -= 90.0f;
            while (headingDegrees < 0.0f) headingDegrees += 360.0f;
            while (headingDegrees > 360.0f) headingDegrees -= 360.0f;
            targetHeading = headingDegrees;
        } else {
            frameSkip--;
        }
        float diff = targetHeading - cachedHeading;
        if (diff > 180.0f) diff -= 360.0f;
        if (diff < -180.0f) diff += 360.0f;
        cachedHeading += diff * 0.2f;
        while (cachedHeading < 0.0f) cachedHeading += 360.0f;
        while (cachedHeading > 360.0f) cachedHeading -= 360.0f;
        return cachedHeading;
    }

    bool HandleRadarDragging(ImVec2 radarPos, int radarSize, bool isMenuOpen) {
        if (!isMenuOpen) return false;
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 mousePos = io.MousePos;
        bool mouseOverRadar = (mousePos.x >= radarPos.x && mousePos.x <= radarPos.x + radarSize &&
                               mousePos.y >= radarPos.y && mousePos.y <= radarPos.y + radarSize);
        if (mouseOverRadar && io.MouseDown[0] && !g_Options.Radar.isDragging) {
            g_Options.Radar.isDragging = true;
            g_Options.Radar.useCustomPos = true;
        }
        if (!io.MouseDown[0] && g_Options.Radar.isDragging) {
            g_Options.Radar.isDragging = false;
        }
        if (g_Options.Radar.isDragging) {
            g_Options.Radar.customPosX += io.MouseDelta.x;
            g_Options.Radar.customPosY += io.MouseDelta.y;
            int screenWidth = GetSystemMetrics(SM_CXSCREEN);
            int screenHeight = GetSystemMetrics(SM_CYSCREEN);
            g_Options.Radar.customPosX = (std::max)(0.0f, (std::min)(g_Options.Radar.customPosX, (float)(screenWidth - radarSize)));
            g_Options.Radar.customPosY = (std::max)(0.0f, (std::min)(g_Options.Radar.customPosY, (float)(screenHeight - radarSize)));
            return true;
        }
        return false;
    }

    void CalcRadarPoint(Vector3D worldPos, float* screenX, float* screenY, float radarRange, float radarSize, ImVec2 radarCenter, float playerYaw) {
        float fy = playerYaw * M_PI / 180.0f;
        float fc = cos(fy);
        float fs = sin(fy);
        auto localplayer = C_Five.GetLocalPlayerInfo();
        Vector3D myPos = localplayer.WorldPos;
        float dx = worldPos.x - myPos.x;
        float dy = worldPos.y - myPos.y;
        float px = -(dy * (-fc) + (dx * fs));
        float py = -(dx * (-fc) - (dy * fs));
        float scaledX = radarCenter.x + (px / radarRange) * (radarSize / 2);
        float scaledY = radarCenter.y + (py / radarRange) * (radarSize / 2);
        if (g_Options.Radar.circleStyle) {
            float dx = scaledX - radarCenter.x;
            float dy = scaledY - radarCenter.y;
            float distance = sqrt(dx * dx + dy * dy);
            float maxDistance = radarSize / 2;
            if (distance > maxDistance) {
                float scale = maxDistance / distance;
                scaledX = radarCenter.x + dx * scale;
                scaledY = radarCenter.y + dy * scale;
            }
        } else {
            float halfSize = radarSize / 2;
            if (scaledX < radarCenter.x - halfSize) scaledX = radarCenter.x - halfSize;
            if (scaledY < radarCenter.y - halfSize) scaledY = radarCenter.y - halfSize;
            if (scaledX > radarCenter.x + halfSize) scaledX = radarCenter.x + halfSize;
            if (scaledY > radarCenter.y + halfSize) scaledY = radarCenter.y + halfSize;
        }
        *screenX = scaledX;
        *screenY = scaledY;
    }

    void DrawPlayerDirection(ImDrawList* drawList, ImVec2 radarCenter, float playerYaw, ImColor color) {
        float rad_dir = (-playerYaw) * M_PI / 180.0f;
        ImVec2 points[3] = { ImVec2(0, -10), ImVec2(-5, 5), ImVec2(5, 5) };
        ImVec2 rotatedPoints[3];
        for (int i = 0; i < 3; i++) {
            rotatedPoints[i].x = (points[i].x * cos(rad_dir) - points[i].y * sin(rad_dir)) + radarCenter.x;
            rotatedPoints[i].y = (points[i].y * cos(rad_dir) + points[i].x * sin(rad_dir)) + radarCenter.y;
        }
        drawList->AddTriangleFilled(rotatedPoints[0], rotatedPoints[1], rotatedPoints[2], color);
    }

    void DrawRadar(ImDrawList* drawList) {
        static float lastFrameTime = ImGui::GetTime();
        float currentTime = ImGui::GetTime();
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;
        animationTime += deltaTime * 0.5f;
        if (animationTime > 1.0f) animationTime -= 1.0f;
        if (!g_Options.Radar.enable) return;
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        int radarSize = g_Options.Radar.size;
        ImVec2 radarPos;
        if (g_Options.Radar.useCustomPos) {
            radarPos = ImVec2(g_Options.Radar.customPosX, g_Options.Radar.customPosY);
        } else {
            switch (g_Options.Radar.position) {
                case 0: radarPos = ImVec2(20, 20); break;
                case 1: radarPos = ImVec2(screenWidth - radarSize - 20, 20); break;
                case 2: radarPos = ImVec2(20, screenHeight - radarSize - 20); break;
                case 3: radarPos = ImVec2(screenWidth - radarSize - 20, screenHeight - radarSize - 20); break;
                default: radarPos = ImVec2(20, 20);
            }
        }
        bool isMenuOpen = interfaceObject.GetMenuOpen();
        bool isDragging = HandleRadarDragging(radarPos, radarSize, isMenuOpen);
        if (isDragging) {
            radarPos = ImVec2(g_Options.Radar.customPosX, g_Options.Radar.customPosY);
        }
        ImVec2 radarCenter = ImVec2(radarPos.x + radarSize / 2, radarPos.y + radarSize / 2);
        // honour the user's picked colors, layered with a menu-dark tint for a cohesive look
        ImColor userBg = ImColor(
            g_Options.Radar.backgroundColor[0], g_Options.Radar.backgroundColor[1],
            g_Options.Radar.backgroundColor[2], g_Options.Radar.backgroundColor[3]);

        const float rounding = ImMax(8.f, radarSize * 0.06f);
        const float animOffset = (g_Options.Radar.showWaves && g_Options.Radar.animateWaves)
            ? sinf(animationTime * 2.0f * (float)M_PI) * 0.04f : 0.0f;
        ImVec2 radarMax = ImVec2(radarPos.x + radarSize, radarPos.y + radarSize);
        const float rr = radarSize * 0.5f;

        if (g_Options.Radar.circleStyle) {
            drawList->AddShadowCircle(radarCenter, rr, col_a(c::black, 0.50f), 28.f, ImVec2(0.f, 4.f), 0, 64);
            drawList->AddCircleFilled(radarCenter, rr, userBg, 64);
            drawList->AddCircleFilled(radarCenter, rr, col_a(c::bg, 0.35f), 64);
            drawList->AddCircle(radarCenter, rr, col_a(c::white12, 1.f), 64, 1.4f);
            drawList->AddCircle(radarCenter, rr, col_a(c::primary, 0.20f), 64, 1.4f);
            if (g_Options.Radar.showWaves)
                for (int i = 1; i <= 4; i++)
                    drawList->AddCircle(radarCenter, rr * (0.25f * i) * (1.f + animOffset * (i / 4.f)), col_a(c::white, 0.05f), 48, 1.f);
            drawList->AddLine(ImVec2(radarCenter.x - rr, radarCenter.y), ImVec2(radarCenter.x + rr, radarCenter.y), col_a(c::white, 0.04f), 1.f);
            drawList->AddLine(ImVec2(radarCenter.x, radarCenter.y - rr), ImVec2(radarCenter.x, radarCenter.y + rr), col_a(c::white, 0.04f), 1.f);
        } else {
            drawList->AddShadowRect(radarPos, radarMax, col_a(c::black, 0.50f), 28.f, ImVec2(0.f, 4.f), 0, rounding);
            drawList->AddRectFilled(radarPos, radarMax, userBg, rounding);
            drawList->AddRectFilled(radarPos, radarMax, col_a(c::bg, 0.35f), rounding);
            drawList->AddLine(ImVec2(radarPos.x + rounding, radarPos.y + 1.f), ImVec2(radarMax.x - rounding, radarPos.y + 1.f), col_a(c::white, 0.06f), 1.f);
            drawList->AddRect(radarPos, radarMax, col_a(c::white12, 1.f), rounding, 0, 1.4f);
            drawList->AddRect(ImVec2(radarPos.x + 1, radarPos.y + 1), ImVec2(radarMax.x - 1, radarMax.y - 1), col_a(c::black, 0.30f), rounding, 0, 1.f);
            drawList->PushClipRect(radarPos, radarMax, true);
            if (g_Options.Radar.showWaves)
                for (int i = 1; i <= 4; i++)
                    drawList->AddCircle(radarCenter, rr * (0.25f * i) * (1.f + animOffset * (i / 4.f)), col_a(c::white, 0.05f), 48, 1.f);
            drawList->AddLine(ImVec2(radarPos.x, radarCenter.y), ImVec2(radarMax.x, radarCenter.y), col_a(c::white, 0.04f), 1.f);
            drawList->AddLine(ImVec2(radarCenter.x, radarPos.y), ImVec2(radarCenter.x, radarMax.y), col_a(c::white, 0.04f), 1.f);
            drawList->PopClipRect();
        }
        auto localplayer = C_Five.GetLocalPlayerInfo();
        float playerYaw = GetPlayerHeading();
        float radarRange = (float)g_Options.Radar.range;
        if (g_Options.Radar.showPlayers) {
            ImColor enemyColor = ImColor(
                g_Options.Radar.enemyColor[0],
                g_Options.Radar.enemyColor[1],
                g_Options.Radar.enemyColor[2],
                g_Options.Radar.enemyColor[3]);
            ImColor friendColor = ImColor(
                g_Options.Radar.friendColor[0],
                g_Options.Radar.friendColor[1],
                g_Options.Radar.friendColor[2],
                g_Options.Radar.friendColor[3]);
            try {
                auto& entityList = Cheat::C_Five.GetEntitiyList();
                if (!entityList.empty()) {
                    Cheat::C_Five.GetLockLists().lock();
                    for (const auto& entity : entityList) {
                        if (entity.StaticInfo.bIsLocalPlayer) continue;
                        if (entity.Distance > (int)radarRange) continue;
                        if (entity.Cordinates.x == 0 && entity.Cordinates.y == 0) continue;
                        float x, y;
                        CalcRadarPoint(entity.Cordinates, &x, &y, radarRange, radarSize, radarCenter, playerYaw);
                        ImColor entityColor = entity.IsFriend ? friendColor : enemyColor;
                        drawList->AddCircleFilled(ImVec2(x, y), 6.5f, col_a(entityColor, 0.22f), 16);
                        drawList->AddCircleFilled(ImVec2(x, y), 3.5f, entityColor, 16);
                        drawList->AddCircle(ImVec2(x, y), 3.5f, col_a(c::white, 0.55f), 16, 1.f);
                        if (g_Options.Radar.showNames && !entity.StaticInfo.Name.empty()) {
                            drawList->AddText(sm_font(), 12.f, ImVec2(x + 8, y - 7), col_a(c::black, 0.6f), entity.StaticInfo.Name.c_str());
                            drawList->AddText(sm_font(), 12.f, ImVec2(x + 7, y - 8), c::white, entity.StaticInfo.Name.c_str());
                        }
                    }
                    Cheat::C_Five.GetLockLists().unlock();
                }
            } catch (...) {}
        }
        if (g_Options.Radar.showVehicles) {
            ImColor vehicleColor = ImColor(
                g_Options.Radar.vehicleColor[0],
                g_Options.Radar.vehicleColor[1],
                g_Options.Radar.vehicleColor[2],
                g_Options.Radar.vehicleColor[3]);
            try {
                auto& vehicleList = Cheat::C_Five.GetVehicleList();
                if (!vehicleList.empty()) {
                    Cheat::C_Five.GetLockLists().lock();
                    for (const auto& vehicle : vehicleList) {
                        if (vehicle.Distance > (int)radarRange) continue;
                        if (vehicle.Cordinates.x == 0 && vehicle.Cordinates.y == 0) continue;
                        float x, y;
                        CalcRadarPoint(vehicle.Cordinates, &x, &y, radarRange, radarSize, radarCenter, playerYaw);
                        float rectSize = 7.0f;
                        drawList->AddRectFilled(ImVec2(x - rectSize / 2, y - rectSize / 2), ImVec2(x + rectSize / 2, y + rectSize / 2), col_a(vehicleColor, 0.22f), 2.f);
                        drawList->AddRectFilled(ImVec2(x - rectSize / 2 + 1, y - rectSize / 2 + 1), ImVec2(x + rectSize / 2 - 1, y + rectSize / 2 - 1), vehicleColor, 2.f);
                        drawList->AddRect(ImVec2(x - rectSize / 2, y - rectSize / 2), ImVec2(x + rectSize / 2, y + rectSize / 2), col_a(c::white, 0.45f), 2.f, 0, 1.f);
                        if (!vehicle.Name.empty()) {
                            std::string idText = "ID:" + std::to_string(vehicle.randomId);
                            drawList->AddText(sm_font(), 12.f, ImVec2(x + 8, y - 7), col_a(c::black, 0.6f), idText.c_str());
                            drawList->AddText(sm_font(), 12.f, ImVec2(x + 7, y - 8), col_a(c::white, 0.9f), idText.c_str());
                            if (g_Options.Radar.showNames) {
                                drawList->AddText(sm_font(), 12.f, ImVec2(x + 8, y + 5), col_a(c::black, 0.6f), vehicle.Name.c_str());
                                drawList->AddText(sm_font(), 12.f, ImVec2(x + 7, y + 4), col_a(c::white, 0.7f), vehicle.Name.c_str());
                            }
                        }
                    }
                    Cheat::C_Five.GetLockLists().unlock();
                }
            } catch (...) {}
        }
        // center: local player marker with a small FIXED arrow pointing straight up
        {
            ImVec2 tri[3] = {
                ImVec2(radarCenter.x,       radarCenter.y - 6.f),   // tip (always up)
                ImVec2(radarCenter.x - 4.f, radarCenter.y + 4.f),
                ImVec2(radarCenter.x + 4.f, radarCenter.y + 4.f),
            };
            if (g_Options.Radar.showDirection) {
                drawList->AddTriangleFilled(tri[0], tri[1], tri[2], c::primary);
                drawList->AddTriangle(tri[0], tri[1], tri[2], col_a(c::white, 0.7f), 1.f);
            }
            drawList->AddCircleFilled(radarCenter, 2.5f, c::white, 16);
        }
    }

    void DrawDraggableRadar() {
        if (!g_Options.Radar.enable) return;
        DrawRadar(ImGui::GetBackgroundDrawList());
    }

    bool HandlePlayersAroundDragging(float panelX, float panelY, float panelWidth, float panelHeight, bool isMenuOpen) {
        if (!isMenuOpen) return false;
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 mousePos = io.MousePos;
        bool mouseOverPanel = (mousePos.x >= panelX && mousePos.x <= panelX + panelWidth &&
                               mousePos.y >= panelY && mousePos.y <= panelY + panelHeight);
        if (mouseOverPanel && io.MouseDown[0] && !g_Options.PlayersAround.isDragging) {
            g_Options.PlayersAround.isDragging = true;
        }
        if (!io.MouseDown[0] && g_Options.PlayersAround.isDragging) {
            g_Options.PlayersAround.isDragging = false;
        }
        if (g_Options.PlayersAround.isDragging) {
            g_Options.PlayersAround.posX += io.MouseDelta.x;
            g_Options.PlayersAround.posY += io.MouseDelta.y;
            int screenWidth = GetSystemMetrics(SM_CXSCREEN);
            int screenHeight = GetSystemMetrics(SM_CYSCREEN);
            g_Options.PlayersAround.posX = (std::max)(0.0f, (std::min)(g_Options.PlayersAround.posX, (float)(screenWidth - panelWidth)));
            g_Options.PlayersAround.posY = (std::max)(0.0f, (std::min)(g_Options.PlayersAround.posY, (float)(screenHeight - panelHeight)));
            return true;
        }
        return false;
    }

    void DrawPlayersAround(ImDrawList* drawList) {
        if (!g_Options.PlayersAround.enable) return;
        
        auto localplayer = C_Five.GetLocalPlayerInfo();
        if (!localplayer.Ped) return;
        
        float maxDistance = (float)g_Options.PlayersAround.distance;
        
        // Panel settings - menu style
        float panelWidth = 220.f;
        float lineHeight = 22.f;
        float padding = 12.f;
        float headerHeight = 28.f;
        float itemPadding = 8.f;
        float rounding = 8.f;
        
        // Collect nearby players (name, distance, isFriend)
        struct PlayerInfo {
            std::string name;
            std::string displayName;
            float distance;
            bool isFriend;
        };
        std::vector<PlayerInfo> nearbyPlayers;
        
        try {
            auto& entityList = Cheat::C_Five.GetEntitiyList();
            if (!entityList.empty()) {
                Cheat::C_Five.GetLockLists().lock();
                for (const auto& entity : entityList) {
                    if (entity.StaticInfo.bIsLocalPlayer) continue;
                    if (entity.Distance > (int)maxDistance) continue;
                    if (entity.StaticInfo.Name.empty()) continue;
                    
                    // Truncate name if longer than 20 characters
                    std::string displayName = entity.StaticInfo.Name;
                    if (displayName.length() > 20) {
                        displayName = displayName.substr(0, 17) + "...";
                    }
                    
                    nearbyPlayers.push_back({entity.StaticInfo.Name, displayName, (float)entity.Distance, entity.IsFriend});
                }
                Cheat::C_Five.GetLockLists().unlock();
            }
        } catch (...) {
            try { Cheat::C_Five.GetLockLists().unlock(); } catch (...) {}
        }
        
        // Sort by distance
        std::sort(nearbyPlayers.begin(), nearbyPlayers.end(), 
            [](const auto& a, const auto& b) { return a.distance < b.distance; });
        
        // Calculate panel height
        int playerCount = (int)nearbyPlayers.size();
        int maxVisiblePlayers = 10;
        int visibleCount = playerCount > maxVisiblePlayers ? maxVisiblePlayers : playerCount;
        float contentHeight = visibleCount > 0 ? visibleCount * lineHeight : lineHeight;
        float panelHeight = headerHeight + contentHeight + padding * 2;
        
        // Get panel position
        float panelX = g_Options.PlayersAround.posX;
        float panelY = g_Options.PlayersAround.posY;
        
        // Handle dragging
        bool isMenuOpen = interfaceObject.GetMenuOpen();
        HandlePlayersAroundDragging(panelX, panelY, panelWidth, panelHeight, isMenuOpen);
        
        // Update position after dragging
        panelX = g_Options.PlayersAround.posX;
        panelY = g_Options.PlayersAround.posY;
        
        // ---- menu-matching palette + fonts ----
        const ImColor panelBg  = col_a(c::bg, 0.96f);
        const ImColor headerBg = col_a(c::white, 0.03f);
        const ImColor border   = c::white12;
        const ImColor titleCol = c::white72;
        const ImColor textCol  = c::white;
        const ImColor dimCol   = c::white48;
        const ImColor friendCol = ImColor(119, 215, 125, 255);
        ImFont* uf = ui_font();
        ImFont* sf = sm_font();

        ImVec2 p0(panelX, panelY), p1(panelX + panelWidth, panelY + panelHeight);
        float  cx0 = panelX + panelWidth * 0.5f;

        // soft shadow + panel body + header + borders
        drawList->AddShadowRect(p0, p1, col_a(c::black, 0.55f), 30.f, ImVec2(0.f, 5.f), 0, rounding);
        drawList->AddRectFilled(p0, p1, panelBg, rounding);
        drawList->AddRectFilled(p0, ImVec2(p1.x, panelY + headerHeight), headerBg, rounding, ImDrawFlags_RoundCornersTop);
        drawList->AddLine(ImVec2(p0.x + rounding, panelY + 1.f), ImVec2(p1.x - rounding, panelY + 1.f), col_a(c::white, 0.06f), 1.f);
        drawList->AddLine(ImVec2(panelX, panelY + headerHeight), ImVec2(panelX + panelWidth, panelY + headerHeight), border, 1.f);
        drawList->AddRect(p0, p1, border, rounding, 0, 1.4f);
        drawList->AddRectFilled(p0, ImVec2(panelX + 3.f, panelY + headerHeight), c::primary, rounding, ImDrawFlags_RoundCornersLeft);

        // header: title + accent count badge
        {
            const char* title = "PLAYERS NEARBY";
            ImVec2 ts = sf->CalcTextSizeA(12.f, FLT_MAX, -1.f, title, 0, NULL);
            drawList->AddText(sf, 12.f, ImVec2(panelX + 12.f, panelY + (headerHeight - ts.y) * 0.5f), titleCol, title);

            char cnt[16]; snprintf(cnt, sizeof cnt, "%d", playerCount);
            ImVec2 cs = sf->CalcTextSizeA(12.f, FLT_MAX, -1.f, cnt, 0, NULL);
            float bw = cs.x + 12.f, bh = 16.f;
            ImVec2 badge0(panelX + panelWidth - 10.f - bw, panelY + (headerHeight - bh) * 0.5f);
            drawList->AddRectFilled(badge0, badge0 + ImVec2(bw, bh), col_a(c::primary, 0.16f), bh * 0.5f);
            drawList->AddText(sf, 12.f, badge0 + ImVec2((bw - cs.x) * 0.5f, (bh - cs.y) * 0.5f), c::primary, cnt);
        }

        // rows
        float currentY = panelY + headerHeight + padding;
        if (nearbyPlayers.empty()) {
            const char* none = "No players nearby";
            ImVec2 ns = sf->CalcTextSizeA(12.f, FLT_MAX, -1.f, none, 0, NULL);
            drawList->AddText(sf, 12.f, ImVec2(cx0 - ns.x * 0.5f, currentY), dimCol, none);
        } else {
            float textH = uf->CalcTextSizeA(13.f, FLT_MAX, -1.f, "Ag", 0, NULL).y;
            int shown = 0;
            for (const auto& player : nearbyPlayers) {
                if (shown >= maxVisiblePlayers) break;
                float itemY = currentY - 2.f;
                ImVec2 r0(panelX + itemPadding, itemY), r1(panelX + panelWidth - itemPadding, itemY + lineHeight);
                drawList->AddRectFilled(r0, r1, col_a(c::white, 0.03f), 5.f);

                ImColor nameCol = player.isFriend ? friendCol : textCol;
                float tcy = itemY + (lineHeight - textH) * 0.5f;
                drawList->AddText(uf, 13.f, ImVec2(panelX + padding, tcy), nameCol, player.displayName.c_str());

                if (g_Options.PlayersAround.showDistance) {
                    char dtxt[32]; snprintf(dtxt, sizeof dtxt, "%dm", (int)player.distance);
                    ImVec2 ds = sf->CalcTextSizeA(12.f, FLT_MAX, -1.f, dtxt, 0, NULL);
                    ImColor badgeTx = (player.distance < 50)  ? ImColor(232, 130, 132, 255)
                                    : (player.distance < 150) ? ImColor(232, 185, 130, 255)
                                                              : (ImColor)dimCol;
                    float bpad = 6.f, bw = ds.x + bpad * 2.f, bh = 16.f;
                    ImVec2 badge0(panelX + panelWidth - padding - bw, itemY + (lineHeight - bh) * 0.5f);
                    drawList->AddRectFilled(badge0, badge0 + ImVec2(bw, bh), col_a(badgeTx, 0.15f), bh * 0.5f);
                    drawList->AddText(sf, 12.f, badge0 + ImVec2(bpad, (bh - ds.y) * 0.5f), badgeTx, dtxt);
                }
                currentY += lineHeight;
                shown++;
            }
            if (playerCount > maxVisiblePlayers) {
                char more[32]; snprintf(more, sizeof more, "+ %d more", playerCount - maxVisiblePlayers);
                ImVec2 ms = sf->CalcTextSizeA(12.f, FLT_MAX, -1.f, more, 0, NULL);
                drawList->AddText(sf, 12.f, ImVec2(cx0 - ms.x * 0.5f, currentY), dimCol, more);
            }
        }
    }

    void run() {
        if (g_Options.Radar.enable) {
            DrawDraggableRadar();
        }
        DrawPlayersAround(ImGui::GetBackgroundDrawList());
    }

    void Render() {
        run();
    }
}
}
