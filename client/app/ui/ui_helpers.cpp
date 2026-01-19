#include "ui_helpers.hpp"

#include <algorithm>
#include <array>

#include "engine/game/components/ui/ui_button.hpp"
#include "engine/game/components/ui/ui_text.hpp"
#include "engine/game/components/ui/ui_transform.hpp"
#include "localization.hpp"

namespace client::app {

std::filesystem::path find_font() {
    const std::array<std::filesystem::path, 3> candidates = {
        "client/assets/fonts/Roboto-Regular.ttf",
        "assets/fonts/Roboto-Regular.ttf",
        "fonts/Roboto-Regular.ttf",
    };
    for (const auto& p : candidates) {
        if (std::filesystem::exists(p)) {
            return p;
        }
    }
    return {};
}

std::uint16_t key_to_mask_bit(sf::Keyboard::Key key) {
    switch (key) {
        case sf::Keyboard::Key::Up:    return 1u << 0;
        case sf::Keyboard::Key::Down:  return 1u << 1;
        case sf::Keyboard::Key::Left:  return 1u << 2;
        case sf::Keyboard::Key::Right: return 1u << 3;
        case sf::Keyboard::Key::Space: return 1u << 4;
        case sf::Keyboard::Key::P:     return 1u << 5;  // Pause
        case sf::Keyboard::Key::E:     return 1u << 6;  // Ultimate
        default:                       return 0;
    }
}

void clear_ui_registry(rtype::ecs::registry& reg) {
    reg = rtype::ecs::registry{};
}

void build_connect_ui(rtype::ecs::registry& reg, sf::Vector2u window_size, const LobbyData& data) {
    clear_ui_registry(reg);

    const float width = static_cast<float>(window_size.x);
    const float height = static_cast<float>(window_size.y);
    const float panel_width = std::min(width * 0.6f, 780.f);
    const float panel_height = std::min(height * 0.70f, 560.f);
    const float panel_x = (width - panel_width) * 0.5f;
    const float panel_y = (height - panel_height) * 0.5f;
    const float center_x = width * 0.5f;
    const float title_y = panel_y + panel_height * 0.10f;
    const float info_y = panel_y + panel_height * 0.22f;
    const float level_label_y = panel_y + panel_height * 0.34f;
    const float level_buttons_y = panel_y + panel_height * 0.44f;
    const float button_width = std::min(panel_width * 0.45f, 360.f);
    const float button_height = 52.f;
    const float button_x = panel_x + (panel_width - button_width) * 0.5f;
    const float connect_y = panel_y + panel_height * 0.58f;
    const float back_y = panel_y + panel_height * 0.72f;

    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, 0.f, 0.f, width, height);
        reg.emplace_component<engine::game::components::UIButton>(entity, engine::game::components::UIButton{"overlay", false, false});
    }
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, panel_x, panel_y, panel_width, panel_height);
        reg.emplace_component<engine::game::components::UIButton>(entity, engine::game::components::UIButton{"panel", false, false});
    }
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, center_x - 85.f, title_y, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = client::systems::Localization::get("connect");
        txt.font_size = 36;
        txt.color = 0xFFFFFFFF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, panel_x + 60.f, info_y, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = client::systems::Localization::get("host") + ": " + data.host + "  " + client::systems::Localization::get("port") + ": " + std::to_string(data.port);
        txt.font_size = 18;
        txt.color = 0xB5BBC5FF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, center_x - 100.f, level_label_y, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = client::systems::Localization::get("select_level");
        txt.font_size = 20;
        txt.color = 0xB5BBC5FF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    const float level_btn_width = 60.f;
    const float level_btn_height = 50.f;
    const float level_btn_spacing = 20.f;
    const float total_level_width = 5 * level_btn_width + 4 * level_btn_spacing;
    const float level_start_x = center_x - total_level_width * 0.5f;
    const char* level_ids[] = {kLevel1Id, kLevel2Id, kLevel3Id, kLevel4Id, kLevel5Id};
    for (int i = 0; i < 5; ++i) {
        auto entity = reg.spawn_entity();
        float btn_x = level_start_x + static_cast<float>(i) * (level_btn_width + level_btn_spacing);
        reg.emplace_component<engine::game::components::UITransform>(entity, btn_x, level_buttons_y, level_btn_width, level_btn_height);

        bool is_selected = (data.selected_level == static_cast<std::uint16_t>(i + 1));
        reg.emplace_component<engine::game::components::UIButton>(entity, engine::game::components::UIButton{level_ids[i], false, is_selected});

        engine::game::components::UIText txt{};
        txt.text = std::to_string(i + 1);
        txt.font_size = 24;
        txt.color = is_selected ? 0x00FF00FF : 0xF3F6FFFF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, button_x, connect_y, button_width, button_height);
        reg.emplace_component<engine::game::components::UIButton>(entity, engine::game::components::UIButton{kConnectId, false, false});
        engine::game::components::UIText txt{};
        txt.text = "CONNECT";
        txt.font_size = 20;
        txt.color = 0xF3F6FFFF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, button_x, back_y, button_width, button_height);
        reg.emplace_component<engine::game::components::UIButton>(entity, engine::game::components::UIButton{kBackId, false, false});
        engine::game::components::UIText txt{};
        txt.text = "BACK";
        txt.font_size = 20;
        txt.color = 0xF3F6FFFF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }
}

void set_main_menu_selection(rtype::ecs::registry& reg, int selected_index, bool has_mouse_hover) {
    if (has_mouse_hover) return;
    const char* selected_id = kMainMenuPlayId;
    if (selected_index == 1) {
        selected_id = kMainMenuSettingsId;
    } else if (selected_index == 2) {
        selected_id = kMainMenuQuitId;
    }
    reg.view<engine::game::components::UIButton>(
        [&](std::size_t /*eid*/, auto& button) {
            if (button.id == kMainMenuPlayId || button.id == kMainMenuSettingsId || button.id == kMainMenuQuitId) {
                button.hovered = (button.id == selected_id);
            }
        });
}

std::optional<int> hovered_main_menu_index(rtype::ecs::registry& reg) {
    std::optional<int> hovered;
    reg.view<engine::game::components::UIButton>(
        [&](std::size_t /*eid*/, auto& button) {
            if (!button.hovered) return;
            if (button.id == kMainMenuPlayId) hovered = 0;
            else if (button.id == kMainMenuSettingsId) hovered = 1;
            else if (button.id == kMainMenuQuitId) hovered = 2;
        });
    return hovered;
}

}  // namespace client::app
