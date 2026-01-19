#include "client/systems/input_system.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <iostream>

namespace client::systems {

void InputSystem::handle_event(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::P) {
            paused_ = !paused_;
            std::cout << (paused_ ? "[client] Paused\n" : "[client] Resumed\n");
        }
    }
}

}
