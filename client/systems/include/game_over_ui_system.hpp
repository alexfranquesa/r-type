#pragma once

#include <SFML/System/Vector2.hpp>

#include "engine/core/registry.hpp"

namespace client::systems {

void build_game_over_ui(rtype::ecs::registry& reg, sf::Vector2u window_size);
void clear_game_over_ui(rtype::ecs::registry& reg);

}  // namespace client::systems
