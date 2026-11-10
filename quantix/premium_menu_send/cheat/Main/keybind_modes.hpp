#pragma once
// ---------------------------------------------------------------------------
// Shared keybind-mode runtime. Each keybind can operate in one of two modes:
//   HOLD   — the action is active only while the key is physically held.
//   TOGGLE — a single press flips the action on; the next press flips it off.
// The menu widgets (menu_new) write the chosen mode; the feature loops
// (cheat backend, running on their own threads) read it via kb::active_vk().
// Both sides include this header so the maps are a single shared instance.
// All access is mutex-guarded because the menu thread and several feature
// threads touch these maps concurrently.
// ---------------------------------------------------------------------------
#include <unordered_map>
#include <string>
#include <mutex>
#include <windows.h>

namespace kb
{
    enum Mode { HOLD = 0, TOGGLE = 1 };

    inline std::mutex g_mtx;
    inline std::unordered_map<std::string, int> g_mode;   // label -> Mode

    struct Rt { bool prevDown = false; bool latched = false; };
    inline std::unordered_map<std::string, Rt> g_rt;      // label -> runtime state

    inline int mode(const std::string& label)
    {
        std::lock_guard<std::mutex> lk(g_mtx);
        auto it = g_mode.find(label);
        return it == g_mode.end() ? HOLD : it->second;
    }
    inline void set_mode(const std::string& label, int m)
    {
        std::lock_guard<std::mutex> lk(g_mtx);
        g_mode[label] = m;
    }

    // Given the raw physical key-down for a bind, return whether the bind's
    // action should be ACTIVE this frame, honouring the mode.
    //   HOLD   -> active == down
    //   TOGGLE -> latched state, flipped on each rising edge
    inline bool active(const std::string& label, bool down)
    {
        std::lock_guard<std::mutex> lk(g_mtx);
        int m; { auto it = g_mode.find(label); m = (it == g_mode.end()) ? HOLD : it->second; }
        Rt& rt = g_rt[label];
        if (m == HOLD) { rt.prevDown = down; return down; }
        if (down && !rt.prevDown) rt.latched = !rt.latched;
        rt.prevDown = down;
        return rt.latched;
    }

    // Convenience: read the VK directly. vk == 0 means "unbound" -> never active.
    inline bool active_vk(const std::string& label, int vk)
    {
        bool down = vk != 0 && (GetAsyncKeyState(vk) & 0x8000) != 0;   // outside the lock
        return active(label, down);
    }

    // Keep the latch in sync when a feature is toggled from the UI (so TOGGLE
    // mode continues from the state the user just set with the mouse).
    inline void set_latched(const std::string& label, bool on)
    {
        std::lock_guard<std::mutex> lk(g_mtx);
        g_rt[label].latched = on;
    }
    inline bool get_latched(const std::string& label)
    {
        std::lock_guard<std::mutex> lk(g_mtx);
        return g_rt[label].latched;
    }

    // Sensible per-keybind default modes so existing muscle-memory is preserved:
    //   toggle-style features default to Press (toggle), activation keys to Hold.
    // Runs once at static-init time, before any feature thread starts.
    inline bool _kb_defaults = []()
    {
        const char* toggles[] = { "Godmode", "Semi-Godmode", "Immortality", "Invisible", "Noclip", "Freecam", "Health", "Armor" };
        const char* holds[]   = { "Aimbot key", "Silent Key", "Magic Key", "Wheel Silent Key", "Trigger key" };
        for (auto* l : toggles) g_mode[l] = TOGGLE;
        for (auto* l : holds)   g_mode[l] = HOLD;
        return true;
    }();
}
