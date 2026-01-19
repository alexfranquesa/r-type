#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics.hpp>
#include <memory>

#include "engine/core/registry.hpp"

namespace client::systems {

void build_main_menu_ui(rtype::ecs::registry& reg, sf::Vector2u window_size);
void clear_main_menu_ui(rtype::ecs::registry& reg);

// Render the title logo on top of everything
void render_title_logo(sf::RenderWindow& window);

}  // namespace client::systems
