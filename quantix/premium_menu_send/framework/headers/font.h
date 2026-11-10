#pragma once
#include "includes.h"

class c_font
{
public:
    struct font_ranges {
        font_ranges() = default;
        font_ranges(unsigned int min, unsigned int max) : data{ ::new ImWchar[](min, max, 0) } {};
        ImWchar* data = nullptr;
    };

    void update();

    void add(const std::string& name, const void* font_data, size_t data_size, float size, font_ranges ranges = {}, bool mono = false);

    ImFont* get(const std::string& name);

private:
    struct font_data
    {
        const void* data;
        size_t data_size;  // Size of compressed font data
        float size;
        ImFont* out_font;
        font_ranges ranges;
        bool is_mono;

        bool operator==(const font_data& other) const
        {
            return data == other.data && size == other.size;
        }
    };

    std::unordered_map<std::string, font_data> fonts_map;
};

inline std::unique_ptr<c_font> font = std::make_unique<c_font>();