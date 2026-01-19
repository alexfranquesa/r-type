#pragma once

#include <SFML/System/Vector2.hpp>

#include "engine/core/registry.hpp"
#include "game_settings.hpp"

namespace client::systems {

class SettingsUISystem {
public:
    void build_ui(rtype::ecs::registry& reg, sf::Vector2u window_size, const GameSettings& settings);
};

}  // namespace client::systems
