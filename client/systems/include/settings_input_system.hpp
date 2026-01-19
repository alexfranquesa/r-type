#pragma once

#include <vector>

#include <SFML/System/Vector2.hpp>

#include "engine/core/registry.hpp"
#include "game_settings.hpp"

namespace client::systems {

struct SettingsInputState {
    sf::Vector2f mouse_pos{};
    bool mouse_down{false};
    bool mouse_pressed{false};
    bool mouse_released{false};
    bool backspace{false};
    bool left{false};
    bool right{false};
    bool enter{false};
    bool escape{false};
    std::vector<char32_t> text_entered{};
};

enum class SettingsMenuAction {
    None,
    Apply,
    Cancel
};

class SettingsInputSystem {
public:
    SettingsMenuAction update(rtype::ecs::registry& reg,
                              GameSettings& settings,
                              const SettingsInputState& input);
};

}  // namespace client::systems
