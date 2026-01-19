/*
** EPITECH PROJECT, 2025
** G-CPP-500-BAR-5-2-rtype-2
** File description:
** accessibility_config - ECS component for accessibility settings
*/

#pragma once

#include <cstddef>

namespace engine::game::components {

/**
 * @brief Accessibility configuration component for UI elements.
 * This component stores global accessibility settings that affect UI rendering.
 * Typically attached to a singleton entity in the menu registry.
 */
struct AccessibilityConfig {
    bool high_contrast{false};         // Enable high contrast mode (black bg, yellow text)
    std::size_t font_scale_index{0};   // Index into available font scales (0=1.0x, 1=1.25x, 2=1.5x)
    
    // Available font scales
    static constexpr std::size_t font_scale_count = 3;
    static constexpr float font_scales[font_scale_count] = {1.0F, 1.25F, 1.5F};
    
    /**
     * @brief Get the current font scale multiplier
     * @return Font scale factor (1.0, 1.25, or 1.5)
     */
    float get_font_scale() const {
        if (font_scale_index < font_scale_count) {
            return font_scales[font_scale_index];
        }
        return font_scales[0];
    }
    
    /**
     * @brief Cycle to the next font scale
     */
    void cycle_font_scale() {
        font_scale_index = (font_scale_index + 1) % font_scale_count;
    }
    
    /**
     * @brief Set font scale index with bounds checking
     */
    void set_font_scale_index(std::size_t idx) {
        if (idx < font_scale_count) {
            font_scale_index = idx;
        }
    }
};

}  // namespace engine::game::components
