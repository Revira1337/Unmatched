#pragma once
#include <imgui.h>
#include <imgui_internal.h>   // ShadeVertsLinearGradY for the toast gradients
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include "../defs/fonts.h"
#include "../defs/colors.h"
#include "../../cheat/Main/Options.hpp"

// ---------------------------------------------------------------------------
// notify.h — lightweight top-right toast notifications.
//   * at most 3 visible at once; the rest wait in a queue and slide in as
//     slots free up.
//   * short lifetime + gentle fade in/out so it never strains the eye.
//   * anti-spam: an identical text is refused while a copy is still active or
//     queued (rapid toggle on/off/on won't spam the same toast).
//   watch() diffs the tracked feature flags every frame and fires
//   "<Feature> enabled/disabled" toasts; draw() renders them.
// ---------------------------------------------------------------------------
namespace notify
{
    struct Item { std::string text; bool good = true; float t = 0.f; float life = 2.6f; };
    inline std::vector<Item> g_active;
    inline std::deque<Item>  g_queue;
    inline constexpr int MAX_ACTIVE = 3;

    // queue a toast (deduped against everything currently on screen / waiting)
    inline void push(const std::string& text, bool good = true)
    {
        for (auto& it : g_active) if (it.text == text) return;
        for (auto& it : g_queue)  if (it.text == text) return;
        Item n; n.text = text; n.good = good;
        g_queue.push_back(n);
    }

    inline void draw()
    {
        float dt = ImGui::GetIO().DeltaTime; if (dt > 0.1f) dt = 0.1f;
        while ((int)g_active.size() < MAX_ACTIVE && !g_queue.empty()) { g_active.push_back(g_queue.front()); g_queue.pop_front(); }
        for (auto& it : g_active) it.t += dt;
        g_active.erase(std::remove_if(g_active.begin(), g_active.end(),
            [](const Item& i) { return i.t >= i.life; }), g_active.end());
        if (g_active.empty()) return;

        ImDrawList* dl = ImGui::GetForegroundDrawList();
        ImFont* uf = f::medium14 ? f::medium14 : ImGui::GetFont();
        ImFont* sf = f::medium12 ? f::medium12 : ImGui::GetFont();
        auto fade = [](ImColor col, float al) { col.Value.w *= al; return col; };

        const float W = 254.f, H = 46.f, gap = 9.f, rnd = 10.f;
        const float sw = ImGui::GetIO().DisplaySize.x;
        const float rightX = sw - 18.f;
        float y = 18.f;

        for (auto& it : g_active)
        {
            float fin  = ImClamp(it.t / 0.24f, 0.f, 1.f);
            float fout = ImClamp((it.life - it.t) / 0.4f, 0.f, 1.f);
            float A = ImMin(fin, fout);
            float e = 1.f - (1.f - fin) * (1.f - fin);        // easeOutQuad slide-in
            float slide = (1.f - e) * 36.f;
            ImVec2 p0(rightX - W + slide, y), p1(rightX + slide, y + H);
            ImColor accent = it.good ? (ImColor)c::primary : ImColor(236, 92, 92, 255);

            const ImDrawFlags rc = ImDrawFlags_RoundCornersRight;   // sharp on the left, rounded on the right
            // soft accent glow + drop shadow
            dl->AddShadowRect(p0, p1, fade(accent, 0.26f * A), 30.f, ImVec2(0.f, 0.f), rc, rnd);
            dl->AddShadowRect(p0, p1, fade(ImColor(0, 0, 0, 160), A), 24.f, ImVec2(0.f, 5.f), rc, rnd);
            // glassy body with a vertical gradient
            int v0 = dl->VtxBuffer.Size;
            dl->AddRectFilled(p0, p1, IM_COL32_WHITE, rnd, rc);
            ShadeVertsLinearGradY(dl, v0, dl->VtxBuffer.Size, p0, p1, fade(ImColor(32, 33, 40, 247), A), fade(ImColor(18, 19, 24, 247), A));
            // top inner highlight + hairline border
            dl->AddLine(ImVec2(p0.x, p0.y + 1.f), ImVec2(p1.x - rnd, p0.y + 1.f), fade(ImColor(255, 255, 255, 20), A), 1.f);
            dl->AddRect(p0, p1, fade(ImColor(255, 255, 255, 28), A), rnd, rc, 1.f);
            // left accent edge (sharp)
            dl->AddRectFilled(p0, ImVec2(p0.x + 3.f, p1.y), fade(accent, A), 0.f);

            float cy = (p0.y + p1.y) * 0.5f;

            // ---- icon badge: check = enabled, x = disabled ----
            ImVec2 bc(p0.x + 26.f, cy);
            const float br = 12.f;
            dl->AddCircleFilled(bc, br + 2.5f, fade(accent, 0.16f * A));       // halo
            int bv = dl->VtxBuffer.Size;
            dl->AddCircleFilled(bc, br, IM_COL32_WHITE);
            ShadeVertsLinearGradY(dl, bv, dl->VtxBuffer.Size, ImVec2(bc.x, bc.y - br), ImVec2(bc.x, bc.y + br), fade(accent, A), fade(accent, 0.75f * A));
            dl->AddCircle(bc, br, fade(ImColor(255, 255, 255, 45), A), 24, 1.f);
            ImColor icol = fade(ImColor(255, 255, 255, 255), A);
            if (it.good) {
                dl->AddLine(ImVec2(bc.x - 5.f, bc.y + 0.5f), ImVec2(bc.x - 1.5f, bc.y + 4.f), icol, 2.f);
                dl->AddLine(ImVec2(bc.x - 1.5f, bc.y + 4.f), ImVec2(bc.x + 5.f, bc.y - 3.5f), icol, 2.f);
            } else {
                dl->AddLine(ImVec2(bc.x - 4.f, bc.y - 4.f), ImVec2(bc.x + 4.f, bc.y + 4.f), icol, 2.f);
                dl->AddLine(ImVec2(bc.x + 4.f, bc.y - 4.f), ImVec2(bc.x - 4.f, bc.y + 4.f), icol, 2.f);
            }

            // ---- two-line text: feature name (white) + status (accent) ----
            std::string name = it.text, status;
            size_t sppos = name.find_last_of(' ');
            if (sppos != std::string::npos) { status = name.substr(sppos + 1); name = name.substr(0, sppos); }
            if (!status.empty() && status[0] >= 'a' && status[0] <= 'z') status[0] = (char)(status[0] - 32);
            float tx = p0.x + 48.f;
            dl->AddText(uf, 14.f, ImVec2(tx, cy - 14.f), fade(ImColor(255, 255, 255, 255), A), name.c_str());
            if (!status.empty())
                dl->AddText(sf, 11.5f, ImVec2(tx, cy + 1.5f), fade(accent, A), status.c_str());

            // ---- remaining-life bar ----
            float pr = 1.f - it.t / it.life;
            dl->AddRectFilled(ImVec2(p0.x + 4.f, p1.y - 2.5f), ImVec2(p0.x + 4.f + (W - rnd - 8.f) * pr, p1.y - 1.f), fade(accent, 0.7f * A));

            y += H + gap;
        }
    }

    // diff the tracked feature flags and fire enable/disable toasts on change
    inline void watch()
    {
        struct Watched { const bool* v; bool prev; const char* name; };
        static Watched ws[] = {
            { &g_Options.ESP.enable,           false, "ESP" },
            { &g_Options.Aimbot.enable,        false, "Aimbot" },
            { &g_Options.Silent.enable,        false, "Silent Aim" },
            { &g_Options.Silent.Magic,         false, "Magic Bullet" },
            { &g_Options.Triggerbot.enable,    false, "Triggerbot" },
            { &g_Options.Radar.enable,         false, "Radar" },
            { &g_Options.PlayersAround.enable, false, "Players Around" },
            { &g_Options.Player.God,           false, "Godmode" },
            { &g_Options.Player.SemiGod,       false, "Semi-Godmode" },
            { &g_Options.Player.Passive,       false, "Immortality" },
            { &g_Options.Exploit.KediciShadow, false, "Invisible" },
            { &g_Options.Player.noclip,        false, "Noclip" },
            { &g_Options.Player.Freecam,       false, "Freecam" },
            { &g_Options.Player.Nocollision,   false, "No Collision" },
            { &g_Options.Player.NoFallDamage,  false, "No Fall Damage" },
            { &g_Options.Player.PSpeed,        false, "Player Speed" },
            { &g_Options.Exploit.StrafeMacro,  false, "Strafe Macro" },
            { &g_Options.Player.NoRecoil,      false, "No Recoil" },
            { &g_Options.Player.NoSpread,      false, "No Spread" },
            { &g_Options.Player.InfiniteAmmo,  false, "Infinite Ammo" },
            { &g_Options.Player.DamageBoost,   false, "Damage Boost" },
            { &g_Options.Player.VehicleGod,    false, "Vehicle Godmode" },
            { &g_Options.Crosshair.enable,     false, "Crosshair" },
        };
        static bool init = false;
        if (!init) { for (auto& w : ws) w.prev = *w.v; init = true; return; }   // seed silently
        for (auto& w : ws)
        {
            bool cur = *w.v;
            if (cur != w.prev)
            {
                push(std::string(w.name) + (cur ? " enabled" : " disabled"), cur);
                w.prev = cur;
            }
        }
    }
}
