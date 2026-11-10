#include "elements_manager.h"

void e_manager::add_tab(const char* label, const char* icon)
{
    tabs.push_back(tab_t(label, icon));
}

void e_manager::add_subtab(const char* name, std::function<void(int)> callback)
{
    if (!tabs.empty())
        tabs.back().subtabs.push_back(subtab_t(name, callback));
}
