#include "game_over_ui_system.hpp"

#include <algorithm>

#include "engine/game/components/ui/ui_button.hpp"
#include "engine/game/components/ui/ui_text.hpp"
#include "engine/game/components/ui/ui_transform.hpp"

namespace client::systems {

void clear_game_over_ui(rtype::ecs::registry& reg) {
    reg = rtype::ecs::registry{};
}

void build_game_over_ui(rtype::ecs::registry& reg, sf::Vector2u window_size) {
    clear_game_over_ui(reg);

    const float width = static_cast<float>(window_size.x);
    const float height = static_cast<float>(window_size.y);
    const float center_x = width * 0.5f;
    
    // Button dimensions
    const float button_width = 320.f;
    const float button_height = 56.f;
    const float button_x = (width - button_width) * 0.5f;
    
    // Layout positions
    const float title_y = 250.f;
    const float play_again_y = 380.f;
    const float go_back_y = 460.f;

    // Dark overlay
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, 0.f, 0.f, width, height);
        reg.emplace_component<engine::game::components::UIButton>(entity, engine::game::components::UIButton{"overlay", false, false});
    }

    // "GAME OVER" title
    {
        auto entity = reg.spawn_entity();
        // Position will be adjusted by render system based on text bounds
        reg.emplace_component<engine::game::components::UITransform>(entity, center_x - 150.f, title_y, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = "GAME OVER";
        txt.font_size = 72;
        txt.color = 0xFF0000FF;  // Red
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    // "PLAY AGAIN" button
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, button_x, play_again_y, button_width, button_height);
        reg.emplace_component<engine::game::components::UIButton>(entity, engine::game::components::UIButton{"play_again", false, false});
        engine::game::components::UIText txt{};
        txt.text = "PLAY AGAIN";
        txt.font_size = 22;
        txt.color = 0xF3F6FFFF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    // "GO BACK TO MENU" button
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, button_x, go_back_y, button_width, button_height);
        reg.emplace_component<engine::game::components::UIButton>(entity, engine::game::components::UIButton{"go_back_menu", false, false});
        engine::game::components::UIText txt{};
        txt.text = "GO BACK TO MENU";
        txt.font_size = 22;
        txt.color = 0xF3F6FFFF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }
}

}  // namespace client::systems
