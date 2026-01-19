#include "main_menu_ui_system.hpp"

#include <algorithm>

#include "engine/game/components/ui/ui_button.hpp"
#include "engine/game/components/ui/ui_text.hpp"
#include "engine/game/components/ui/ui_transform.hpp"
#include "localization.hpp"

namespace client::systems {

void clear_main_menu_ui(rtype::ecs::registry& reg) {
    reg = rtype::ecs::registry{};
}

void build_main_menu_ui(rtype::ecs::registry& reg, sf::Vector2u window_size) {
    clear_main_menu_ui(reg);

    const float width = static_cast<float>(window_size.x);
    const float height = static_cast<float>(window_size.y);
    const float panel_width = width;
    const float panel_height = height;
    const float panel_x = 0.f;
    const float panel_y = 0.f;
    const float center_x = width * 0.5f;
    const float logo_y = panel_y + panel_height * 0.08f;  // Logo position
    const float title_y = panel_y + panel_height * 0.25f;  // Title position
    const float subtitle_y = panel_y + panel_height * 0.40f;  // More space between title and subtitle
    const float button_width = std::clamp(panel_width * 0.35f, 280.f, 380.f);
    const float button_height = 60.f;  // Slightly taller
    const float play_y = panel_y + panel_height * 0.44f;
    const float scores_y = panel_y + panel_height * 0.54f;
    const float help_y = panel_y + panel_height * 0.64f;
    const float settings_y = panel_y + panel_height * 0.74f;
    const float quit_y = panel_y + panel_height * 0.84f;
    const float button_x = panel_x + (panel_width - button_width) * 0.5f;
    const float footer_y = panel_y + panel_height - 36.f;

    // Title image will be rendered separately via render_title_logo()

    // PLAY button - Bright green arcade style
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, button_x, play_y, button_width, button_height);
        reg.emplace_component<engine::game::components::UIButton>(entity, engine::game::components::UIButton{"play", false, false});
        engine::game::components::UIText txt{};
        txt.text = Localization::get("play");
        txt.font_size = 28;
        txt.color = 0x00FF00FF;  // Bright green - classic arcade
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    // SCORES button - Gold arcade style
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, button_x, scores_y, button_width, button_height);
        reg.emplace_component<engine::game::components::UIButton>(entity, engine::game::components::UIButton{"scores", false, false});
        engine::game::components::UIText txt{};
        txt.text = Localization::get("scores");
        txt.font_size = 28;
        txt.color = 0xFFD700FF;  // Gold - arcade style
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    // HELP button - Orange arcade style
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, button_x, help_y, button_width, button_height);
        reg.emplace_component<engine::game::components::UIButton>(entity, engine::game::components::UIButton{"help", false, false});
        engine::game::components::UIText txt{};
        txt.text = Localization::get("help");
        txt.font_size = 28;
        txt.color = 0xFFA500FF;  // Orange - arcade style
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    // SETTINGS button - Cyan arcade style
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, button_x, settings_y, button_width, button_height);
        reg.emplace_component<engine::game::components::UIButton>(entity, engine::game::components::UIButton{"settings", false, false});
        engine::game::components::UIText txt{};
        txt.text = Localization::get("settings");
        txt.font_size = 28;
        txt.color = 0x00FFFFFF;  // Cyan - arcade style
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    // QUIT button - Magenta/pink arcade style
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, button_x, quit_y, button_width, button_height);
        reg.emplace_component<engine::game::components::UIButton>(entity, engine::game::components::UIButton{"quit", false, false});
        engine::game::components::UIText txt{};
        txt.text = Localization::get("quit");
        txt.font_size = 28;
        txt.color = 0xFF00FFFF;  // Magenta - arcade style
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    // Footer
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, center_x - 10.f, footer_y, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = "alpha";
        txt.font_size = 16;
        txt.color = 0x8C94A0FF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }
}

void render_title_logo(sf::RenderWindow& window) {
    static sf::Texture title_texture;
    static std::unique_ptr<sf::Sprite> title_sprite;
    static bool loaded = false;
    
    if (!loaded) {
        if (title_texture.loadFromFile("sprites/title.png")) {
            title_sprite = std::make_unique<sf::Sprite>(title_texture);
            
            // Center the logo horizontally and position it at the top
            auto tex_size = title_texture.getSize();
            float scale = 400.0f / static_cast<float>(tex_size.x);  // Scale to 400px width
            title_sprite->setScale(sf::Vector2f(scale, scale));
            
            float x = (1280.0f - (static_cast<float>(tex_size.x) * scale)) * 0.5f;
            float y = 120.0f;  // Position from top
            title_sprite->setPosition(sf::Vector2f(x, y));
            
            loaded = true;
            std::cout << "[MainMenu] Title logo loaded from sprites/title.png" << std::endl;
        } else {
            std::cerr << "[MainMenu] Failed to load title logo" << std::endl;
            loaded = true;  // Don't try again
        }
    }
    
    if (title_sprite && title_texture.getNativeHandle() != 0) {
        window.draw(*title_sprite);
    }
}

}  // namespace client::systems
