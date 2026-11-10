#pragma once

#include <thread>
#include <imgui.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>

#include "FivemSDK/Fivem.hpp"
#include "Features/esp.hpp"
#include "Features/silent.hpp"
#include "Features/aim.hpp"
#include "Features/player.hpp"
#include "Features/radar.hpp"
#include "Options.hpp"

namespace Cheat
{
    void C_Start();
    void C_Stop();
    bool IsReady();
}