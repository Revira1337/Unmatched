#pragma once
#include "includes.h"

//
// All binds must have unique name! You can use bindname##id to set similar visual names, but real ids be different.
//

class config;

class c_keybinds {
public:
    enum class switch_type : uint8_t {
        hold,
        toggle
    };

    using press_callback_t = std::function<void(bool, ImGuiKey, switch_type)>;

    struct config_keybind {
        ImGuiKey key = ImGuiKey_None;
        switch_type type = switch_type::hold;
        bool initialized = false;
    };

    struct keybind {
        ImGuiKey key = ImGuiKey_None;
        switch_type type = switch_type::hold;
        press_callback_t callback = nullptr;
        config_keybind* linked_cfg_bind = nullptr;
        
        bool value = false;
        bool callback_called = false;
        bool disabled = false;
    };


public:
    c_keybinds() = default;

    c_keybinds(const c_keybinds&) = delete;
    c_keybinds& operator=(const c_keybinds&) = delete;

    void load_binds_from_config(config& cfg);

    void process_binds();

    keybind& try_add_bind(const std::string& name)
    {
        return binds.try_emplace(name, keybind{}).first->second;
    }

    void try_erase_bind(const std::string& name)
    {
        if (auto bind_it = binds.find(name); bind_it != binds.end())
        {
            binds.erase(bind_it);
        }
    }

    [[nodiscard]] keybind* get_bind(const std::string& name)
    {
        if (auto bind_it = binds.find(name); bind_it != binds.end())
        {
            return &bind_it->second;
        }

        return nullptr;
    }

    [[nodiscard]] const keybind* get_bind(const std::string& name) const
    {
        if (const auto bind_it = binds.find(name); bind_it != binds.cend())
        {
            return &bind_it->second;
        }

        return nullptr;
    }

    [[nodiscard]] bool is_bind_active(const std::string& name) const { auto* bind = get_bind(name); return (bind != nullptr) ? bind->value : false; }

    [[nodiscard]] const std::unordered_map<std::string, keybind>& get_binds() const { return binds; }

    [[nodiscard]] size_t active_binds_coint() const {
        return active_binds_coint_;
    }

private:
    std::unordered_map<std::string, keybind> binds{};
    size_t active_binds_coint_{ 0 };
};

inline std::unique_ptr<c_keybinds> keybinds = std::make_unique<c_keybinds>();