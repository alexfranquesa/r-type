#include "death_menu_ui_system.hpp"

#include <algorithm>
#include <sstream>
#include <iomanip>

#include "engine/game/components/ui/ui_button.hpp"
#include "engine/game/components/ui/ui_text.hpp"
#include "engine/game/components/ui/ui_transform.hpp"

namespace client::systems {

void clear_death_menu_ui(rtype::ecs::registry& reg) {
    reg = rtype::ecs::registry{};
}

void build_death_menu_ui(rtype::ecs::registry& reg, sf::Vector2u window_size, bool has_alive_players, std::uint32_t score, std::uint16_t level) {
    clear_death_menu_ui(reg);

    const float width = static_cast<float>(window_size.x);
    const float height = static_cast<float>(window_size.y);
    const float center_x = width * 0.5f;
    
    // Button dimensions
    const float button_width = 320.f;
    const float button_height = 56.f;
    const float button_x = (width - button_width) * 0.5f;
    
    // Layout positions
    const float title_y = 200.f;
    const float stats_y = 300.f;
    float button_y = has_alive_players ? 420.f : 400.f;  // Center button if only one option

    // Dark overlay
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, 0.f, 0.f, width, height);
        reg.emplace_component<engine::game::components::UIButton>(entity, engine::game::components::UIButton{"overlay", false, false});
    }

    // "YOU DIED" title
    {
        auto entity = reg.spawn_entity();
        // Position will be adjusted by render system based on text bounds
        reg.emplace_component<engine::game::components::UITransform>(entity, center_x - 150.f, title_y, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = "YOU DIED";
        txt.font_size = 72;
        txt.color = 0xFF0000FF;  // Red
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    // Score display
    {
        auto entity = reg.spawn_entity();
        std::ostringstream oss;
        oss << "SCORE: " << score;
        reg.emplace_component<engine::game::components::UITransform>(entity, center_x - 100.f, stats_y, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = oss.str();
        txt.font_size = 32;
        txt.color = 0xFFD700FF;  // Gold
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    // Level reached display
    {
        auto entity = reg.spawn_entity();
        std::ostringstream oss;
        oss << "LEVEL REACHED: " << level;
        reg.emplace_component<engine::game::components::UITransform>(entity, center_x - 120.f, stats_y + 50.f, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = oss.str();
        txt.font_size = 32;
        txt.color = 0x00BFFFFF;  // Cyan
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    // "SPECTATE" button - only if there are alive players
    if (has_alive_players) {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, button_x, button_y, button_width, button_height);
        reg.emplace_component<engine::game::components::UIButton>(entity, engine::game::components::UIButton{"spectate", false, false});
        engine::game::components::UIText txt{};
        txt.text = "SPECTATE";
        txt.font_size = 22;
        txt.color = 0xF3F6FFFF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
        button_y += 80.f;  // Move next button down
    }

    // "GO BACK TO MENU" button
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, button_x, button_y, button_width, button_height);
        reg.emplace_component<engine::game::components::UIButton>(entity, engine::game::components::UIButton{"go_back_menu", false, false});
        engine::game::components::UIText txt{};
        txt.text = "GO BACK TO MENU";
        txt.font_size = 22;
        txt.color = 0xF3F6FFFF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }
}

}  // namespace client::systems
