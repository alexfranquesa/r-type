#include "help_ui_system.hpp"

#include "engine/game/components/ui/ui_button.hpp"
#include "engine/game/components/ui/ui_text.hpp"
#include "engine/game/components/ui/ui_transform.hpp"
#include "localization.hpp"

namespace client::systems {

void build_help_ui(rtype::ecs::registry& reg, sf::Vector2u window_size) {
    reg = rtype::ecs::registry{};

    const float width = static_cast<float>(window_size.x);
    const float height = static_cast<float>(window_size.y);
    const float center_x = width * 0.5f;
    const float content_start_y = 80.f;
    const float line_spacing = 30.f;
    float current_y = content_start_y;

    // Title
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, center_x - 150.f, 30.f, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = Localization::get("help_title");
        txt.font_size = 36;
        txt.color = 0xFFFFFFFF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    // CONTROLS Section
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, 100.f, current_y, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = Localization::get("help_controls");
        txt.font_size = 24;
        txt.color = 0x00FF00FF;  // Green
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }
    current_y += line_spacing;

    // Controls list
    const std::array<const char*, 3> control_keys = {
        "help_movement",
        "help_shoot",
        "help_ultimate"
    };

    for (const auto& key : control_keys) {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, 120.f, current_y, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = Localization::get(key);
        txt.font_size = 18;
        txt.color = 0xE6EAF2FF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
        current_y += line_spacing;
    }

    current_y += 20.f;

    // SETTINGS Section
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, 100.f, current_y, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = Localization::get("help_settings_title");
        txt.font_size = 24;
        txt.color = 0xFFD700FF;  // Gold
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }
    current_y += line_spacing;

    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, 120.f, current_y, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = Localization::get("help_settings_desc");
        txt.font_size = 16;
        txt.color = 0xE6EAF2FF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }
    current_y += line_spacing * 2;

    // ULTIMATE Section
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, 100.f, current_y, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = Localization::get("help_ultimate_title");
        txt.font_size = 24;
        txt.color = 0xFF00FFFF;  // Magenta
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }
    current_y += line_spacing;

    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, 120.f, current_y, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = Localization::get("help_ultimate_desc");
        txt.font_size = 16;
        txt.color = 0xE6EAF2FF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }
    current_y += line_spacing * 2;

    // LEVELS Section
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, 100.f, current_y, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = Localization::get("help_levels_title");
        txt.font_size = 24;
        txt.color = 0x00FFFFFF;  // Cyan
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }
    current_y += line_spacing;

    // Levels list
    const std::array<const char*, 5> level_keys = {
        "help_level1",
        "help_level2",
        "help_level3",
        "help_level4",
        "help_level5"
    };

    for (const auto& key : level_keys) {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, 120.f, current_y, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = Localization::get(key);
        txt.font_size = 18;
        txt.color = 0xE6EAF2FF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
        current_y += line_spacing;
    }

    current_y += 20.f;

    // TIPS Section
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, 100.f, current_y, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = Localization::get("help_tips_title");
        txt.font_size = 24;
        txt.color = 0xFFA500FF;  // Orange
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }
    current_y += line_spacing;

    // Tips list
    const std::array<const char*, 3> tip_keys = {
        "help_tip1",
        "help_tip2",
        "help_tip3"
    };

    for (const auto& key : tip_keys) {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, 120.f, current_y, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = Localization::get(key);
        txt.font_size = 18;
        txt.color = 0xE6EAF2FF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
        current_y += line_spacing;
    }

    current_y += 30.f;

    // Footer text
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, center_x - 180.f, current_y, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = Localization::get("press_back");
        txt.font_size = 20;
        txt.color = 0x8C94A0FF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    // BACK button
    {
        const float button_width = 320.f;
        const float button_height = 56.f;
        const float button_x = center_x - button_width * 0.5f;
        const float button_y = height - 80.f;

        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, button_x, button_y, button_width, button_height);
        reg.emplace_component<engine::game::components::UIButton>(entity, engine::game::components::UIButton{"back_to_menu", false, false});
        engine::game::components::UIText txt{};
        txt.text = Localization::get("back_to_menu");
        txt.font_size = 28;
        txt.color = 0xFFFFFFFF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }
}

}  // namespace client::systems
