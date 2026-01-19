#pragma once

#include <vector>
#include <string>
#include <SFML/Window/Window.hpp>
#include <SFML/Window/Mouse.hpp>

#include "engine/core/registry.hpp"
#include "engine/game/components/ui/ui_transform.hpp"
#include "engine/game/components/ui/ui_button.hpp"

namespace client::systems {

class UISystem {
public:
    // Update hover/pressed state based on mouse position/button.
    void update(rtype::ecs::registry& reg, sf::WindowBase& window);

    // Retrieve and clear buttons that were pressed this frame.
    std::vector<std::string> consume_pressed();

private:
    std::vector<std::string> pressed_ids_;
    bool last_mouse_down_{false};
};

}  // namespace client::systems
