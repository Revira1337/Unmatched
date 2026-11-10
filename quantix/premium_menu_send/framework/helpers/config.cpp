#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include "../settings/elements.h"
#include "../headers/config.h"

using namespace std::string_view_literals;

std::ostream& operator<<(std::ostream& stream, const ImVec2& value)
{
	return stream << "vec2(" << value.x << ", " << value.y << ")";
}

std::ostream& operator<<(std::ostream& stream, const ImVec4& value)
{
	return stream << "vec4(" << value.x << ", " << value.y << ", " << value.z << ", " << value.w << ")";
}

std::ostream& operator<<(std::ostream& stream, const c_keybinds::config_keybind& value) {
    return stream << "key = " << static_cast<int>(value.key) << ", type = " << static_cast<int>(value.type);
}

/*
std::ostream& operator<<(std::ostream& stream, const esp_preview_state& value) {
	stream << "{\n";
	stream << "\tenable_outline_rect = " << std::boolalpha << value.enable_outline_rect << "\n";
	stream << "\thead = " << std::boolalpha << value.head << "\n";
	stream << "\tneck = " << std::boolalpha << value.neck << "\n";
	stream << "\tstomach = " << std::boolalpha << value.stomach << "\n";
	stream << "\tleft_hand = " << std::boolalpha << value.left_hand << "\n";
	stream << "\tleft_elbow = " << std::boolalpha << value.left_elbow << "\n";
	stream << "\tleft_shoulder = " << std::boolalpha << value.left_shoulder << "\n";
	stream << "\tright_hand = " << std::boolalpha << value.right_hand << "\n";
	stream << "\tright_elbow = " << std::boolalpha << value.right_elbow << "\n";
	stream << "\tright_shoulder = " << std::boolalpha << value.right_shoulder << "\n";
	stream << "\tleft_thigh = " << std::boolalpha << value.left_thigh << "\n";
	stream << "\tleft_knee = " << std::boolalpha << value.left_knee << "\n";
	stream << "\tleft_foot = " << std::boolalpha << value.left_foot << "\n";
	stream << "\tright_thigh = " << std::boolalpha << value.right_thigh << "\n";
	stream << "\tright_knee = " << std::boolalpha << value.right_knee << "\n";
	stream << "\tright_foot = " << std::boolalpha << value.right_foot << "\n";
	stream << "\ttexts: {\n";
	for (const auto& text : value.texts) {
		stream << "\t\t" << static_cast<int>(text.first) << " = { uuid = " << static_cast<int>(text.second.uuid) << ", text = \"" << text.second.text << "\", color = " << text.second.color << ", enabled = " << std::boolalpha << text.second.enabled << " }\n";
	}
	stream << "\t}\n";
	stream << "\thp_bars: {\n";
	for (const auto& hp_bar : value.hp_bars) {
		stream << "\t\t" << static_cast<int>(hp_bar.first) << " = { uuid = " << static_cast<int>(hp_bar.second.uuid) << ", value = " << hp_bar.second.value << ", max_value = " << hp_bar.second.max_value << ", enabled =  " << std::boolalpha << hp_bar.second.enabled << " }\n";
	}
	stream << "\t}\n";
	stream << "}";
	return stream;
}
*/

std::ostream& operator<<(std::ostream& stream, const std::vector<bool>& value) {
    stream << "{\n";
    for (const auto& element : value) {
        stream << "\t" << std::boolalpha << element << "\n";
    }
    stream << "}";
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const config_type& value)
{
    std::visit([&stream](const auto& v) {
        if constexpr (std::is_same_v<std::decay_t<decltype(v)>, bool>) {
            stream << std::boolalpha << v;
        }
        else if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::string>) {
            stream << '\"' << v << '\"';
        }
        else {
            stream << v;
        }
    }, value);
    return stream;
}

std::istream& operator>>(std::istream& stream, c_keybinds::config_keybind& value) {
    std::string line;
    while (std::getline(stream, line)) {
        if (line.find("key") != std::string::npos) {
            size_t equals_pos = line.find(" = ");
            if (equals_pos != std::string::npos) {
                std::string key_value = line.substr(equals_pos + 3);
                value.key = static_cast<ImGuiKey>(std::stoi(key_value));
            }
        }
        else if (line.find("type") != std::string::npos) {
            size_t equals_pos = line.find(" = ");
            if (equals_pos != std::string::npos) {
                std::string type_value = line.substr(equals_pos + 3);
                value.type = static_cast<c_keybinds::switch_type>(std::stoi(type_value));
            }
        }
    }
    return stream;
}

/*
std::istream& operator>>(std::istream& stream, esp_preview_state& value) {
    std::string line;
    while (std::getline(stream, line)) {
        if (line.find("enable_outline_rect") != std::string::npos) {
            value.enable_outline_rect = (line.find("true") != std::string::npos);
        }
        else if (line.find("head") != std::string::npos) {
            value.head = (line.find("true") != std::string::npos);
        }
        else if (line.find("neck") != std::string::npos) {
            value.neck = (line.find("true") != std::string::npos);
        }
        else if (line.find("stomach") != std::string::npos) {
            value.stomach = (line.find("true") != std::string::npos);
        }
        else if (line.find("left_hand") != std::string::npos) {
            value.left_hand = (line.find("true") != std::string::npos);
        }
        else if (line.find("left_elbow") != std::string::npos) {
            value.left_elbow = (line.find("true") != std::string::npos);
        }
        else if (line.find("left_shoulder") != std::string::npos) {
            value.left_shoulder = (line.find("true") != std::string::npos);
        }
        else if (line.find("right_hand") != std::string::npos) {
            value.right_hand = (line.find("true") != std::string::npos);
        }
        else if (line.find("right_elbow") != std::string::npos) {
            value.right_elbow = (line.find("true") != std::string::npos);
        }
        else if (line.find("right_shoulder") != std::string::npos) {
            value.right_shoulder = (line.find("true") != std::string::npos);
        }
        else if (line.find("left_thigh") != std::string::npos) {
            value.left_thigh = (line.find("true") != std::string::npos);
        }
        else if (line.find("left_knee") != std::string::npos) {
            value.left_knee = (line.find("true") != std::string::npos);
        }
        else if (line.find("left_foot") != std::string::npos) {
            value.left_foot = (line.find("true") != std::string::npos);
        }
        else if (line.find("right_thigh") != std::string::npos) {
            value.right_thigh = (line.find("true") != std::string::npos);
        }
        else if (line.find("right_knee") != std::string::npos) {
            value.right_knee = (line.find("true") != std::string::npos);
        }
        else if (line.find("right_foot") != std::string::npos) {
            value.right_foot = (line.find("true") != std::string::npos);
        }
        else if (line.find("texts") != std::string::npos) {
            std::string text_line;
            while (std::getline(stream, text_line)) {
                if (text_line.find("}") != std::string::npos) {
                    break;
                }
                size_t colon_pos = text_line.find("=");
                if (colon_pos != std::string::npos) {
                    std::string text_key = text_line.substr(0, colon_pos);
                    std::string text_value = text_line.substr(colon_pos + 1);
                    esp_preview_state::text text;
                    size_t uuid_pos = text_value.find("uuid = ");
                    if (uuid_pos != std::string::npos) {
                        size_t uuid_end_pos = text_value.find(",", uuid_pos);
                        if (uuid_end_pos != std::string::npos) {
                            text.uuid = std::stoi(text_value.substr(uuid_pos + 6, uuid_end_pos - uuid_pos - 6));
                        }
                    }
                    size_t text_pos = text_value.find("text = \"");
                    if (text_pos != std::string::npos) {
                        size_t text_end_pos = text_value.find("\"", text_pos + 7);
                        if (text_end_pos != std::string::npos) {
                            text.text = text_value.substr(text_pos + 7, text_end_pos - text_pos - 7);
                        }
                    }
                    size_t color_pos = text_value.find("color = ");
                    size_t color_end_pos = text_value.find(",", color_pos);
                    if (color_end_pos != std::string::npos) {
                        std::string color_str = text_value.substr(color_pos + 7, color_end_pos - color_pos - 7);
                        size_t x_pos = color_str.find("(");
                        size_t y_pos = color_str.find(", ", x_pos + 1);
                        size_t z_pos = color_str.find(", ", y_pos + 2);
                        size_t w_pos = color_str.find(")", z_pos + 2);
                        if (x_pos != std::string::npos && y_pos != std::string::npos && z_pos != std::string::npos && w_pos != std::string::npos) {
                            text.color = ImVec4(std::stof(color_str.substr(x_pos + 1, y_pos - x_pos - 1)),
                                std::stof(color_str.substr(y_pos + 2, z_pos - y_pos - 2)),
                                std::stof(color_str.substr(z_pos + 2, w_pos - z_pos - 2)),
                                std::stof(color_str.substr(w_pos - 1, 1)));
                        }
                    }
                    size_t enabled_pos = text_value.find("enabled = ");
                    if (enabled_pos != std::string::npos) {
                        text.enabled = (text_value.find("true", enabled_pos + 9) != std::string::npos);
                    }
                    value.texts[static_cast<esp_preview_state::text_placement>(std::stoi(text_key))] = text;
                }
            }
        }
        else if (line.find("hp_bars") != std::string::npos) {
            std::string hp_bar_line;
            while (std::getline(stream, hp_bar_line)) {
                if (hp_bar_line.find("}") != std::string::npos) {
                    break;
                }
                size_t colon_pos = hp_bar_line.find("=");
                if (colon_pos != std::string::npos) {
                    std::string hp_bar_key = hp_bar_line.substr(0, colon_pos);
                    std::string hp_bar_value = hp_bar_line.substr(colon_pos + 1);
                    esp_preview_state::hp_bar hp_bar;
                    size_t uuid_pos = hp_bar_value.find("uuid = ");
                    if (uuid_pos != std::string::npos) {
                        size_t uuid_end_pos = hp_bar_value.find(",", uuid_pos);
                        if (uuid_end_pos != std::string::npos) {
                            hp_bar.uuid = std::stoi(hp_bar_value.substr(uuid_pos + 6, uuid_end_pos - uuid_pos - 6));
                        }
                    }
                    size_t value_pos = hp_bar_value.find("value = ");
                    if (value_pos != std::string::npos) {
                        size_t value_end_pos = hp_bar_value.find(",", value_pos);
                        if (value_end_pos != std::string::npos) {
                            hp_bar.value = std::stoi(hp_bar_value.substr(value_pos + 7, value_end_pos - value_pos - 7));
                        }
                    }
                    size_t max_value_pos = hp_bar_value.find("max_value = ");
                    if (max_value_pos != std::string::npos) {
                        size_t max_value_end_pos = hp_bar_value.find(",", max_value_pos);
                        if (max_value_end_pos != std::string::npos) {
                            hp_bar.max_value = std::stoi(hp_bar_value.substr(max_value_pos + 10, max_value_end_pos - max_value_pos - 10));
                        }
                    }
                    size_t enabled_pos = hp_bar_value.find("enabled = ");
                    if (enabled_pos != std::string::npos) {
                        hp_bar.enabled = (hp_bar_value.find("true", enabled_pos + 9) != std::string::npos);
                    }
                    value.hp_bars[static_cast<esp_preview_state::hp_bar_placement>(std::stoi(hp_bar_key))] = hp_bar;
                }
            }
        }
    }
    return stream;
}
*/

std::istream& operator>>(std::istream& stream, std::vector<bool>& value) {
    std::string line;
    value.clear();
    while (std::getline(stream, line)) {
        if (line.find("}") != std::string::npos) {
            break;
        }
        size_t pos = line.find("\t");
        if (pos != std::string::npos) {
            std::string element_str = line.substr(pos + 1);
            size_t end_pos = element_str.find("\n");
            if (end_pos != std::string::npos) {
                element_str = element_str.substr(0, end_pos);
            }
            value.push_back((element_str == "true"));
        }
    }
    return stream;
}

config::config(std::string_view category_name, bool is_main_cfg, std::initializer_list<options_map_t::value_type> list) : options{ list } {
	PWSTR ppsz_path;
	HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &ppsz_path);

	if (SUCCEEDED(hr)) {
        if (is_main_cfg) {
            filepath = std::filesystem::path(ppsz_path) / project_folder_name / configs_folder_name;
        }
        else
        {
		    filepath = std::filesystem::path(ppsz_path) / project_folder_name / configs_folder_name / category_name;
        }
	}

    if (!std::filesystem::exists(filepath))
    {
        std::filesystem::create_directories(filepath);
    }

	CoTaskMemFree(ppsz_path);
};

bool config::save_to_disk(std::string_view config_name) const
{
    auto full_path = filepath / config_name;
    full_path.replace_extension(".cfg");

    std::ofstream ofs(full_path);
    if (!ofs.is_open()) {
        return false;
    }

    for (const auto& [option_name, option_value] : options)
    {
        ofs << option_name << " = " << option_value << '\n';
    }
    return true;
}

bool config::load_from_disk(std::string_view config_name)
{
    auto full_path = filepath / config_name;
    full_path.replace_extension(".cfg");

    std::ifstream ifs(full_path);
    if (!ifs.is_open()) {
        return false;
    }

    load_from_stream(ifs);
    return true;
}

void config::save_to_clipboard() const
{
    std::stringstream ss;
    for (auto& [option_name, option_value] : options)
    {
        ss << option_name << " = " << option_value << '\n';
    }
    ImGui::SetClipboardText(ss.str().data());
}

void config::load_from_clipboard()
{
    const char* clipboard_text = ImGui::GetClipboardText();
    if (clipboard_text)
    {
        std::istringstream iss(clipboard_text);
        load_from_stream(iss);
    }
}

void config::load_from_stream(std::istream& main_stream)
{
    std::string line;
    while (std::getline(main_stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        const auto pos = line.find(" = ");
        if (pos == std::string::npos) {
            continue;
        }

        std::string_view option_name = std::string_view(line).substr(0, pos);
        std::string_view value_str_sv = std::string_view(line).substr(pos + 3);

        auto it = options.find(std::string(option_name));
        if (it == options.end()) {
            continue;
        }

        std::visit([&](auto& value) {
            using T = std::decay_t<decltype(value)>;

            if constexpr (
                std::is_same_v<T, bool> || std::is_same_v<T, int> ||
                std::is_same_v<T, float> || std::is_same_v<T, std::string> ||
                std::is_same_v<T, ImVec2> || std::is_same_v<T, ImVec4>
                ) {
                std::string_view single_line_value = std::string_view(line).substr(pos + 3);

                if constexpr (std::is_same_v<T, bool>) {
                    value = (single_line_value == "true"sv);
                }
                else if constexpr (std::is_same_v<T, int>) {
                    std::from_chars(single_line_value.data(), single_line_value.data() + single_line_value.size(), value);
                }
                else if constexpr (std::is_same_v<T, float>) {
                    std::from_chars(single_line_value.data(), single_line_value.data() + single_line_value.size(), value);
                }
                else if constexpr (std::is_same_v<T, std::string>) {
                    if (single_line_value.starts_with('\"') && single_line_value.ends_with('\"')) {
                        value = std::string(single_line_value.substr(1, single_line_value.length() - 2));
                    }
                    else {
                        value = std::string(single_line_value);
                    }
                }
                else if constexpr (std::is_same_v<T, ImVec2>) {
                    sscanf_s(single_line_value.data(), "vec2(%f, %f)", &value.x, &value.y);
                }
                else if constexpr (std::is_same_v<T, ImVec4>) {
                    sscanf_s(single_line_value.data(), "vec4(%f, %f, %f, %f)", &value.x, &value.y, &value.z, &value.w);
                }
            }
            else if constexpr (std::is_same_v<T, c_keybinds::config_keybind>) {
                size_t key_pos = value_str_sv.find("key = ");
                size_t type_pos = value_str_sv.find("type = ");

                if (key_pos != std::string::npos) {
                    size_t key_start = key_pos + 6;
                    size_t key_end = value_str_sv.find(",", key_start);
                    std::string_view key_val_str = value_str_sv.substr(key_start, key_end - key_start);
                    int key_val;
                    auto [ptr_k, ec_k] = std::from_chars(key_val_str.data(), key_val_str.data() + key_val_str.size(), key_val);
                    if (ec_k == std::errc() && ptr_k == key_val_str.data() + key_val_str.size()) {
                        value.key = static_cast<ImGuiKey>(key_val);
                    }
                }

                if (type_pos != std::string::npos) {
                    size_t type_start = type_pos + 7;
                    std::string_view type_val_str = value_str_sv.substr(type_start);
                    int type_val;
                    auto [ptr_t, ec_t] = std::from_chars(type_val_str.data(), type_val_str.data() + type_val_str.size(), type_val);
                    if (ec_t == std::errc() && ptr_t == type_val_str.data() + type_val_str.size()) {
                        value.type = static_cast<c_keybinds::switch_type>(type_val);
                    }
                }
            }
            else {
                main_stream >> value;
            }
            }, it->second);
    }
}

void config::rename(std::string_view old_name, std::string_view new_name) const
{
    const auto old_target = filepath / (old_name.data() + ".cfg"s);
    const auto new_target = filepath / (new_name.data() + ".cfg"s);

    if (std::filesystem::exists(old_target))
    {
        std::filesystem::rename(old_target, new_target);
    }
}

void config::delete_from_disk(std::string_view config_name) const
{
    const auto target = filepath / (config_name.data() + ".cfg"s);

    if (std::filesystem::exists(target))
    {
        std::filesystem::remove(target);
    }
}