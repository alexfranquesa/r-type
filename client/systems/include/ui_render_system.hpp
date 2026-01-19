#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

#include "engine/core/registry.hpp"
#include "engine/game/components/ui/ui_transform.hpp"
#include "engine/game/components/ui/ui_button.hpp"
#include "engine/game/components/ui/ui_text.hpp"

namespace client::systems {

class UIRenderSystem {
public:
    explicit UIRenderSystem(const sf::Font& font);

    void draw(rtype::ecs::registry& reg, sf::RenderWindow& window);

private:
    const sf::Font& font_;
};

}  // namespace client::systems
