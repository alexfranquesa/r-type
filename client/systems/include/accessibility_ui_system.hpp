#pragma once

#include <SFML/System/Vector2.hpp>

#include "engine/core/registry.hpp"

namespace client::systems {

/**
 * @brief Builds the accessibility settings menu UI using ECS.
 * Creates UI entities with buttons for toggling high contrast and font scale.
 * Also includes an AccessibilityConfig component to track current settings.
 * 
 * @param reg The ECS registry to populate with UI entities
 * @param window_size The size of the window for layout calculations
 * @param high_contrast Current high contrast setting
 * @param font_scale_index Current font scale index
 */
void build_accessibility_ui(rtype::ecs::registry& reg, sf::Vector2u window_size, 
                            bool high_contrast, std::size_t font_scale_index);

/**
 * @brief Clears all entities from the accessibility UI registry.
 * 
 * @param reg The registry to clear
 */
void clear_accessibility_ui(rtype::ecs::registry& reg);

}  // namespace client::systems
