#pragma once

#include <SFML/Window/Event.hpp>

namespace client::systems {

class InputSystem {
public:
    void handle_event(const sf::Event& event);

    bool is_paused() const { return paused_; }

private:
    bool paused_ = false;
};

}
