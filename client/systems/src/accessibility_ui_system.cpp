#include "accessibility_ui_system.hpp"

#include <algorithm>
#include <string>

#include "engine/game/components/ui/ui_button.hpp"
#include "engine/game/components/ui/ui_text.hpp"
#include "engine/game/components/ui/ui_transform.hpp"
#include "engine/game/components/ui/accessibility_config.hpp"

namespace client::systems {

void clear_accessibility_ui(rtype::ecs::registry& reg) {
    reg = rtype::ecs::registry{};
}

void build_accessibility_ui(rtype::ecs::registry& reg, sf::Vector2u window_size,
                            bool high_contrast, std::size_t font_scale_index) {
    clear_accessibility_ui(reg);

    const float width = static_cast<float>(window_size.x);
    const float height = static_cast<float>(window_size.y);
    const float panel_width = std::clamp(width * 0.7f, 520.f, 920.f);
    const float panel_height = std::clamp(height * 0.7f, 420.f, 640.f);
    const float panel_x = (width - panel_width) * 0.5f;
    const float panel_y = (height - panel_height) * 0.5f;
    const float center_x = width * 0.5f;
    const float title_y = panel_y + panel_height * 0.15f;
    const float button_width = std::clamp(panel_width * 0.70f, 320.f, 500.f);
    const float button_height = 56.f;
    const float button_x = panel_x + (panel_width - button_width) * 0.5f;
    
    // Button positions
    const float contrast_y = panel_y + panel_height * 0.35f;
    const float font_scale_y = panel_y + panel_height * 0.50f;
    const float back_y = panel_y + panel_height * 0.75f;

    // Store accessibility config in an entity
    {
        auto entity = reg.spawn_entity();
        engine::game::components::AccessibilityConfig config;
        config.high_contrast = high_contrast;
        config.font_scale_index = font_scale_index;
        reg.emplace_component<engine::game::components::AccessibilityConfig>(entity, config);
    }

    // Overlay
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, 0.f, 0.f, width, height);
        reg.emplace_component<engine::game::components::UIButton>(entity, 
            engine::game::components::UIButton{"overlay", false, false});
    }

    // Panel
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, panel_x, panel_y, panel_width, panel_height);
        reg.emplace_component<engine::game::components::UIButton>(entity, 
            engine::game::components::UIButton{"panel", false, false});
    }

    // Title
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, center_x - 160.f, title_y, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = "ACCESSIBILITY";
        txt.font_size = 48;
        txt.color = 0xFFFFFFFF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    // High Contrast button
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, button_x, contrast_y, button_width, button_height);
        reg.emplace_component<engine::game::components::UIButton>(entity, 
            engine::game::components::UIButton{"toggle_contrast", false, false});
        engine::game::components::UIText txt{};
        txt.text = "High Contrast: " + std::string(high_contrast ? "ON" : "OFF");
        txt.font_size = 22;
        txt.color = 0xF3F6FFFF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    // Font Scale button
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, button_x, font_scale_y, button_width, button_height);
        reg.emplace_component<engine::game::components::UIButton>(entity, 
            engine::game::components::UIButton{"cycle_font_scale", false, false});
        engine::game::components::UIText txt{};
        const float scale = engine::game::components::AccessibilityConfig::font_scales[
            font_scale_index < engine::game::components::AccessibilityConfig::font_scale_count 
            ? font_scale_index : 0
        ];
        txt.text = "Font Scale: " + std::to_string(scale).substr(0, 4) + "x";
        txt.font_size = 22;
        txt.color = 0xF3F6FFFF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    // BACK button
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, button_x, back_y, button_width, button_height);
        reg.emplace_component<engine::game::components::UIButton>(entity, 
            engine::game::components::UIButton{"back", false, false});
        engine::game::components::UIText txt{};
        txt.text = "BACK";
        txt.font_size = 22;
        txt.color = 0xF3F6FFFF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    // Hint text
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, panel_x + 60.f, panel_y + panel_height * 0.88f, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = "Use Up/Down + Enter or click buttons";
        txt.font_size = 16;
        txt.color = 0xB5BBC5FF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }
}

}  // namespace client::systems
