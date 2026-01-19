#include "ui_system.hpp"

#include <algorithm>
#include <SFML/Window/Event.hpp>

namespace client::systems {

void UISystem::update(rtype::ecs::registry& reg, sf::WindowBase& window) {
    const auto mouse_pos = sf::Mouse::getPosition(window);
    const bool mouse_down = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    const bool mouse_pressed = mouse_down && !last_mouse_down_;
    const bool mouse_released = !mouse_down && last_mouse_down_;

    reg.view<engine::game::components::UITransform, engine::game::components::UIButton>(
        [&](std::size_t /*eid*/, auto& transform, auto& button) {
            if (button.id == "panel" || button.id == "overlay") {
                button.hovered = false;
                button.pressed = false;
                return;
            }
            const float mx = static_cast<float>(mouse_pos.x);
            const float my = static_cast<float>(mouse_pos.y);
            const bool inside =
                mx >= transform.x &&
                mx <= transform.x + transform.width &&
                my >= transform.y &&
                my <= transform.y + transform.height;
            button.hovered = inside;
            if (mouse_pressed && inside) {
                button.pressed = true;
            }
            if (mouse_down && button.pressed && !inside) {
                button.pressed = false;
            }
            if (mouse_released) {
                if (button.pressed && inside) {
                    pressed_ids_.push_back(button.id);
                }
                button.pressed = false;
            }
        });

    last_mouse_down_ = mouse_down;
}

std::vector<std::string> UISystem::consume_pressed() {
    auto out = pressed_ids_;
    pressed_ids_.clear();
    return out;
}

}  // namespace client::systems
