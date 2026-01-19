#pragma once

#include <cstdint>
#include <SFML/Graphics/RenderWindow.hpp>

#include "engine/core/registry.hpp"
#include "engine/render/renderer_interface.hpp"

namespace client::systems {

class UltimateDisplaySystem {
public:
    void draw(
        engine::render::IRenderer& renderer,
        rtype::ecs::registry& game_reg,
        sf::RenderWindow& window,
        std::uint16_t local_player_id
    );

private:
    static constexpr float kScale = 3.5f;
    static constexpr int kFrameCount = 4;
    static constexpr int kFrameWidth = 32;
    static constexpr int kFrameHeight = 32;
    static constexpr float kPauseButtonWidth = 100.0f;
    static constexpr float kPauseButtonHeight = 40.0f;
    static constexpr float kPanelWidth = 200.0f;
    static constexpr float kPanelMargin = 10.0f;
    static constexpr float kGapMargin = 12.0f;
    static constexpr float kYOffset = 24.0f;
};

}  // namespace client::systems
