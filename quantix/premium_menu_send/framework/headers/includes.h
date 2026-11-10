#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DISABLE_DEBUG_TOOLS

#include "imgui.h"
#include "imgui_internal.h"

#include <vector>
#include <map>
#include <array>
#include <unordered_map>
#include <sstream>
#include <string>
#include <string_view>
#include <memory>
#include <variant>
#include <chrono>
#include <functional>
#include <span>

using namespace std::string_literals;
using namespace std::chrono_literals;