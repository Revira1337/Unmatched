#pragma once
#include <imgui.h>
#include "../defs/colors.h"

// Menu-wide settings shared between the Settings page and the renderer.
namespace settings
{
    inline bool  watermark = true;
    inline bool  aim_preview = true;   // Aim Assist preview visible
    inline bool  esp_preview_on = true;// Visuals ESP preview visible
    inline float accent[4] = { 1.f, 1.f, 1.f, 1.f };  // menu accent (== default white)

    // Navigation layout: 0 = Sidebar (left rail with accordion subtabs),
    //                    1 = Topbar  (classic centered icon bar + floating subtab strip)
    inline int   layout = 0;

    // recompute the c::primary/secondary family from the accent colour
    inline void apply_accent()
    {
        float r = accent[0], g = accent[1], b = accent[2];
        c::primary   = ImColor(r, g, b, 1.000f);
        c::primary48 = ImColor(r, g, b, 0.478f);
        c::primary24 = ImColor(r, g, b, 0.239f);
        c::primary12 = ImColor(r, g, b, 0.118f);
        c::primary6  = ImColor(r, g, b, 0.059f);
        c::primary0  = ImColor(r, g, b, 0.000f);
        float sr = ImMin(1.f, r + 0.25f), sg = ImMin(1.f, g + 0.25f), sb = ImMin(1.f, b + 0.12f);
        c::secondary   = ImColor(sr, sg, sb, 1.000f);
        c::secondary12 = ImColor(sr, sg, sb, 0.118f);
        c::secondary2  = ImColor(sr, sg, sb, 0.020f);
        c::secondary0  = ImColor(sr, sg, sb, 0.000f);
    }
}
