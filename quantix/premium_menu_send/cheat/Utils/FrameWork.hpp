#pragma once

// Windows Libraries
#include <Windows.h>
#include <iostream>
#include <dwmapi.h>
#include <TlHelp32.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <chrono>
#include <functional>
#include <cmath>

// Security
#include "Security/XorStr.hpp"
#include "Security/LazyImporter.hpp"

// Math
#include "Math/Matrix4x4.hpp"
#include "Math/Vectors/Vector2D.hpp"
#include "Math/Vectors/Vector3D.hpp"
#include "Math/Vectors/Vector4D.hpp"

// Utilities
#include "Utilities/Misc.hpp"
#include "Utilities/Memory.hpp"

// Render
#include "Render/Overlay.hpp"

// ImGui - use premium_menu_send imgui
#include <imgui.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include <imgui_internal.h>

// Json
#include "../Main/nlohmann/json.hpp"

// DirectX
#include <d3d11.h>
#include <dxgi.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwmapi.lib")

#define LI_FN(str) lzimpLI_FN(str)
#define SafeCall(str) lzimpLI_FN(str)