#pragma once
#include "includes.h"
#include "widgets.h"
#include <filesystem>

//struct esp_preview_state;

using config_type = std::variant<bool, int, float, std::string, std::vector<bool>, ImVec2, ImVec4, c_keybinds::config_keybind>;

class config
{
public:
    using options_map_t = std::map<std::string, config_type>;

public:
    config(std::string_view category_name, bool is_main_cfg, std::initializer_list<options_map_t::value_type> list);
    
    template <typename T>
    [[nodiscard]] T& get(const std::string& name) {
        auto it = options.find(name);
        if (it != options.end()) {
            if (T* value = std::get_if<T>(&it->second)) {
                return *value;
            }
        }
        throw std::runtime_error("No option with name '"s + name + "' in config "s + filepath.filename().string());
    }

    template <typename T>
    [[nodiscard]] const T& get(const std::string& name) const {
        auto it = options.find(name);
        if (it != options.end()) {
            if (const T* value = std::get_if<T>(&it->second)) {
                return *value;
            }
        }
        throw std::runtime_error("No option with name '"s + name + "' in config "s + filepath.filename().string());
    }

    [[nodiscard]] options_map_t& get_options() noexcept { return options; }
    [[nodiscard]] const options_map_t& get_options() const noexcept { return options; }

    [[nodiscard]] bool save_to_disk(std::string_view config_name) const;
    [[nodiscard]] bool load_from_disk(std::string_view config_name);

    void delete_from_disk(std::string_view config_name) const;
    void rename(std::string_view old_name, std::string_view new_name) const;
    
    void save_to_clipboard() const;
    void load_from_clipboard();

    [[nodiscard]] std::filesystem::path get_filepath() const { return filepath; }

private:
    std::filesystem::path filepath;
    options_map_t options;

    constexpr static std::string_view project_folder_name = "premium_menu";
    constexpr static std::string_view configs_folder_name = "configs";

private:
    void load_from_stream(std::istream& stream);
};