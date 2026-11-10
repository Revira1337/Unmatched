#pragma once
#include <vector>
#include <string>
#include <functional>
#include <D3D11.h>

struct subtab_t
{
    subtab_t(std::string n, std::function<void(int)> c) : name(n), callback(c) {}
    std::string name{};
    std::function<void(int)> callback; // draws the page; called once per column (0/1)
};

struct tab_t
{
    tab_t(std::string l, std::string i) : label(l), icon(i) {}
    std::string label{};
    std::string icon{};
    std::vector<subtab_t> subtabs{};
    int active_sub = 0; // remembered per tab
};

class e_manager
{
public:
    void add_tab(const char* label, const char* icon);
    void add_subtab(const char* name, std::function<void(int)> callback = nullptr);

    std::vector<tab_t> tabs{};
    bool esp_p = false;
    bool close_requested = false; // set by the confirm splash to quit the app
    bool unload_prompt = false;   // set by the Unload button -> opens the confirm splash

    //help me
    ID3D11Device* g_pd3dDevice = nullptr;
};

inline e_manager* MGR = new e_manager();
