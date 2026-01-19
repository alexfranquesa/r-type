#include "ultimate_display_system.hpp"

#include <algorithm>

#include "engine/game/components/gameplay/ultimate_charge.hpp"
#include "engine/game/components/network/owner.hpp"

namespace client::systems {

void UltimateDisplaySystem::draw(
    engine::render::IRenderer& renderer,
    rtype::ecs::registry& game_reg,
    sf::RenderWindow& window,
    std::uint16_t local_player_id
) {
    if (local_player_id == 0) {
        return;
    }

    std::uint8_t frame = 0;
    bool found = false;
    game_reg.view<engine::game::components::Owner, engine::game::components::UltimateCharge>(
        [&](rtype::ecs::entity_t /*entity*/, auto& owner, auto& charge) {
            if (owner.player_id != local_player_id) {
                return;
            }
            frame = charge.ui_frame;
            found = true;
        });

    if (!found) {
        return;
    }

    const float window_width = static_cast<float>(window.getSize().x);
    const float pause_right =
        (window_width - kPauseButtonWidth) * 0.5f + kPauseButtonWidth;
    const float panel_left = window_width - kPanelWidth - kPanelMargin;
    const float sprite_half = (static_cast<float>(kFrameWidth) * kScale) * 0.5f;
    const float min_x = pause_right + kGapMargin + sprite_half;
    const float max_x = panel_left - kGapMargin - sprite_half;
    float indicator_x = (min_x + max_x) * 0.5f;
    if (min_x > max_x) {
        indicator_x = max_x;
    }
    const float indicator_y = kPanelMargin + kPauseButtonHeight + kYOffset;

    const int clamped_frame = std::clamp<int>(static_cast<int>(frame), 1, kFrameCount);
    engine::render::SpriteView sprite{};
    sprite.texture_id = "ultimate";
    sprite.rect_left = (clamped_frame - 1) * kFrameWidth;
    sprite.rect_top = 0;
    sprite.rect_width = kFrameWidth;
    sprite.rect_height = kFrameHeight;
    sprite.x = indicator_x;
    sprite.y = indicator_y;
    sprite.scale_x = kScale;
    sprite.scale_y = kScale;
    sprite.origin_x = static_cast<float>(kFrameWidth) * 0.5f;
    sprite.origin_y = static_cast<float>(kFrameHeight) * 0.5f;
    sprite.z_index = 95.0f;
    renderer.draw(sprite);
}

}  // namespace client::systems
