#pragma once
#include "../Options.hpp"
#include "../FivemSDK/Fivem.hpp"
#include <imgui.h>

namespace Cheat
{
    namespace Radar
    {
        void run();
        void Render();
        void DrawRadar(ImDrawList* drawList);
        void DrawDraggableRadar();
        void CalcRadarPoint(Vector3D worldPos, float* screenX, float* screenY, float radarRange, float radarSize, ImVec2 radarCenter, float playerYaw);
        float GetPlayerHeading(CPed* playerPed);
        bool HandleRadarDragging(ImVec2 radarPos, int radarSize, bool isMenuOpen);
    }
}
