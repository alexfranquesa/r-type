/*
** EPITECH PROJECT, 2025
** G-CPP-500-BAR-5-2-rtype-2
** File description:
** ui_text
*/

#pragma once
#include <string>

namespace engine::game::components {

    struct UIText {
        std::string text{};
        std::string font_path{"assets/fonts/arial.ttf"};
        unsigned int font_size{24};
        uint32_t color{0xFFFFFFFF}; // White by default
    };

}
