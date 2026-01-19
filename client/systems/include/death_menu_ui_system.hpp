#pragma once

#include <SFML/System/Vector2.hpp>

#include "engine/core/registry.hpp"

namespace client::systems {

void build_death_menu_ui(rtype::ecs::registry& reg, sf::Vector2u window_size, bool has_alive_players = true, std::uint32_t score = 0, std::uint16_t level = 1);
void clear_death_menu_ui(rtype::ecs::registry& reg);

}  // namespace client::systems
