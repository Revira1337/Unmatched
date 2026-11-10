#pragma once
#include <imgui.h>

namespace c
{
    // Neutral black / grey / white surface palette (no blue tint). Accent defaults to white.
    inline ImColor bg = ImColor(18, 18, 18, 255);
    inline ImColor bg0 = ImColor(18, 18, 18, 0);
    inline ImColor black = ImColor(0, 0, 0, 255);
    inline ImColor black48 = ImColor(0, 0, 0, 122);
    inline ImColor black24 = ImColor(0, 0, 0, 61);
    inline ImColor primary = ImColor(255, 255, 255, 255);
    inline ImColor primary48 = ImColor(255, 255, 255, 122);
    inline ImColor primary24 = ImColor(255, 255, 255, 61);
    inline ImColor primary12 = ImColor(255, 255, 255, 30);
    inline ImColor primary6 = ImColor(255, 255, 255, 15);
    inline ImColor primary0 = ImColor(255, 255, 255, 0);
    inline ImColor secondary = ImColor(235, 235, 235, 255);
    inline ImColor secondary12 = ImColor(235, 235, 235, 30);
    inline ImColor secondary2 = ImColor(235, 235, 235, 5);
    inline ImColor secondary0 = ImColor(235, 235, 235, 0);
    inline ImColor white = ImColor(255, 255, 255, 255);
    inline ImColor white72 = ImColor(255, 255, 255, 182);
    inline ImColor white48 = ImColor(255, 255, 255, 122);
    inline ImColor white36 = ImColor(255, 255, 255, 91);
    inline ImColor white24 = ImColor(255, 255, 255, 61);
    inline ImColor white12 = ImColor(255, 255, 255, 30);
    inline ImColor white6 = ImColor(255, 255, 255, 15);
    inline ImColor white4 = ImColor(255, 255, 255, 10);
    inline ImColor white3 = ImColor(255, 255, 255, 7);
    inline ImColor white2 = ImColor(255, 255, 255, 5);
    inline ImColor white0 = ImColor(255, 255, 255, 0);

    inline ImColor gray = ImColor(173, 173, 173, 255);
    inline ImColor gray2 = ImColor(118, 118, 118, 255);

    inline struct highlight_t
    {
        ImColor f = ImColor(175, 215, 255, 255);
        ImColor s = secondary0;
    } highlight;
};
