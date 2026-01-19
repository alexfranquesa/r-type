#pragma once

#include "engine/core/registry.hpp"
#include <SFML/Graphics.hpp>

namespace client::systems {

void build_help_ui(rtype::ecs::registry& reg, sf::Vector2u window_size);

}  // namespace client::systems
