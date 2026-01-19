#include "settings_ui_system.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include "engine/game/components/ui/ui_action.hpp"
#include "engine/game/components/ui/ui_dropdown.hpp"
#include "engine/game/components/ui/ui_slider.hpp"
#include "engine/game/components/ui/ui_tab.hpp"
#include "engine/game/components/ui/ui_text.hpp"
#include "engine/game/components/ui/ui_text_input.hpp"
#include "engine/game/components/ui/ui_transform.hpp"
#include "engine/game/components/ui/ui_button.hpp"
#include "engine/game/components/ui/ui_setting_key.hpp"
#include "components/ui_tab_owner.hpp"
#include "components/ui_visibility.hpp"
#include "settings_ui_constants.hpp"
#include "localization.hpp"

namespace client::systems {

namespace {

void clear_settings_ui(rtype::ecs::registry& reg) {
    reg = rtype::ecs::registry{};
}

void add_back_button(rtype::ecs::registry& reg, float x, float y, float size) {
    auto entity = reg.spawn_entity();
    engine::game::components::UIAction action{};
    action.type = engine::game::components::ActionType::Back;
    action.x = x;
    action.y = y;
    action.width = size;
    action.height = size;
    reg.emplace_component<engine::game::components::UIAction>(entity, action);
}

engine::game::components::TabId find_active_tab(rtype::ecs::registry& reg) {
    using engine::game::components::TabId;
    using engine::game::components::UITab;
    TabId active = TabId::Audio;
    reg.view<UITab>([&](std::size_t /*eid*/, auto& tab) {
        if (tab.active) {
            active = tab.id;
        }
    });
    return active;
}

void add_label(rtype::ecs::registry& reg,
               engine::game::components::TabId tab,
               bool visible,
               const std::string& text,
               float x,
               float y) {
    auto entity = reg.spawn_entity();
    reg.emplace_component<engine::game::components::UITransform>(entity, x, y, 0.f, 0.f);
    engine::game::components::UIText label{};
    label.text = text;
    label.font_size = 20;
    label.color = 0xE6EAF2FF;
    reg.emplace_component<engine::game::components::UIText>(entity, label);
    reg.emplace_component<client::components::UITabOwner>(entity, client::components::UITabOwner{tab});
    reg.emplace_component<client::components::UIVisibility>(entity, client::components::UIVisibility{visible});
}

void add_slider(rtype::ecs::registry& reg,
                engine::game::components::TabId tab,
                bool visible,
                const std::string& label,
                float label_x,
                float y,
                float slider_x,
                float slider_w,
                float slider_h,
                engine::game::components::SettingKey key,
                float min_value,
                float max_value,
                float value) {
    add_label(reg, tab, visible, label, label_x, y + 2.f);  // Changed from y - 2.f to y + 2.f for better alignment

    auto entity = reg.spawn_entity();
    engine::game::components::UISlider slider{};
    slider.x = slider_x;
    slider.y = y;
    slider.width = slider_w;
    slider.height = slider_h;
    slider.min_value = min_value;
    slider.max_value = max_value;
    slider.value = value;
    slider.key = key;
    slider.dragging = false;
    reg.emplace_component<engine::game::components::UISlider>(entity, slider);
    reg.emplace_component<client::components::UITabOwner>(entity, client::components::UITabOwner{tab});
    reg.emplace_component<client::components::UIVisibility>(entity, client::components::UIVisibility{visible});
}

void add_dropdown(rtype::ecs::registry& reg,
                  engine::game::components::TabId tab,
                  bool visible,
                  const std::string& label,
                  float label_x,
                  float y,
                  float drop_x,
                  float drop_w,
                  float drop_h,
                  engine::game::components::SettingKey key,
                  std::vector<std::string> options,
                  int selected_index) {
    add_label(reg, tab, visible, label, label_x, y + 2.f);  // Changed from y - 2.f to y + 2.f for better alignment

    auto entity = reg.spawn_entity();
    engine::game::components::UIDropdown dropdown{};
    dropdown.x = drop_x;
    dropdown.y = y;
    dropdown.width = drop_w;
    dropdown.height = drop_h;
    dropdown.options = std::move(options);
    dropdown.selected_index = selected_index;
    dropdown.key = key;
    dropdown.open = false;
    reg.emplace_component<engine::game::components::UIDropdown>(entity, dropdown);
    reg.emplace_component<client::components::UITabOwner>(entity, client::components::UITabOwner{tab});
    reg.emplace_component<client::components::UIVisibility>(entity, client::components::UIVisibility{visible});
}

void add_text_input(rtype::ecs::registry& reg,
                    engine::game::components::TabId tab,
                    bool visible,
                    const std::string& label,
                    float label_x,
                    float y,
                    float input_x,
                    float input_w,
                    float input_h,
                    engine::game::components::SettingKey key,
                    std::string text) {
    add_label(reg, tab, visible, label, label_x, y + 2.f);  // Changed from y - 2.f to y + 2.f for better alignment

    auto entity = reg.spawn_entity();
    engine::game::components::UITextInput input{};
    input.x = input_x;
    input.y = y;
    input.width = input_w;
    input.height = input_h;
    input.text = std::move(text);
    input.key = key;
    input.focused = false;
    input.cursor_index = input.text.size();
    reg.emplace_component<engine::game::components::UITextInput>(entity, input);
    reg.emplace_component<client::components::UITabOwner>(entity, client::components::UITabOwner{tab});
    reg.emplace_component<client::components::UIVisibility>(entity, client::components::UIVisibility{visible});
}

}  // namespace

void SettingsUISystem::build_ui(rtype::ecs::registry& reg,
                                sf::Vector2u window_size,
                                const GameSettings& settings) {
    using engine::game::components::ActionType;
    using engine::game::components::SettingKey;
    using engine::game::components::TabId;
    using namespace client::systems::settings_ui;

    const TabId active_tab = find_active_tab(reg);
    clear_settings_ui(reg);

    // Add back button in top-left corner
    constexpr float back_button_size = 50.f;
    constexpr float back_button_margin = 20.f;
    add_back_button(reg, back_button_margin, back_button_margin, back_button_size);

    const float width = static_cast<float>(window_size.x);
    const float height = static_cast<float>(window_size.y);
    const float panel_width = width;  // Full screen width
    const float panel_height = height; // Full screen height
    const float panel_x = 0.f;  // Start at left edge
    const float panel_y = 0.f;  // Start at top edge
    const float center_x = width * 0.5f;
    const float title_y = height * 0.08f;  // Title position from top
    const float tabs_y = height * 0.18f;   // Tabs position from top
    const float content_y = height * 0.28f; // Content starts here
    const float label_x = width * 0.15f;   // Labels with margin from left
    const float control_x = width * 0.55f; // Controls positioned to the right
    const float row_gap = 85.f;  // Increased from 70 to avoid text overlap with font scaling
    const float control_width = std::clamp(width * 0.30f, 260.f, 440.f); // Responsive width
    const float control_height = 36.f;  // Increased to 36 for better dropdown visibility

    // Overlay
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, 0.f, 0.f, width, height);
        reg.emplace_component<engine::game::components::UIButton>(entity,
            engine::game::components::UIButton{"overlay", false, false});
    }

    // Panel removed - using full screen instead

    // Title
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, center_x - 120.f, title_y, 0.f, 0.f);
        engine::game::components::UIText txt{};
        txt.text = Localization::get("settings");
        txt.font_size = 44;
        txt.color = 0xFFFFFFFF;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    // Tabs
    {
        struct TabInfo {
            TabId id;
            const char* localization_key;
        };
        const std::array<TabInfo, 5> tabs = {{
            {TabId::Audio, "audio"},
            {TabId::Gameplay, "gameplay"},
            {TabId::Graphics, "graphics"},
            {TabId::Network, "network"},
            {TabId::Accessibility, "accessibility"}
        }};
        const float tab_gap = 8.f;
        const float tab_width = (panel_width - tab_gap * 4.f) / 5.f;
        const float tab_height = 36.f;
        float tab_x = panel_x;
        for (const auto& tab : tabs) {
            auto entity = reg.spawn_entity();
            engine::game::components::UITab ui_tab{};
            ui_tab.name = Localization::get(tab.localization_key);
            ui_tab.x = tab_x;
            ui_tab.y = tabs_y;
            ui_tab.width = tab_width;
            ui_tab.height = tab_height;
            ui_tab.id = tab.id;
            ui_tab.active = (tab.id == active_tab);
            reg.emplace_component<engine::game::components::UITab>(entity, ui_tab);
            tab_x += tab_width + tab_gap;
        }
    }

    // Audio tab
    {
        const bool visible = (active_tab == TabId::Audio);
        float y = content_y;
        add_slider(reg, TabId::Audio, visible, Localization::get("master_volume"), label_x, y,
                   control_x, control_width, control_height, SettingKey::MasterVolume,
                   kVolumeMin, kVolumeMax, settings.master_volume);
        y += row_gap;
        add_slider(reg, TabId::Audio, visible, Localization::get("music_volume"), label_x, y,
                   control_x, control_width, control_height, SettingKey::MusicVolume,
                   kVolumeMin, kVolumeMax, settings.music_volume);
        y += row_gap;
        add_slider(reg, TabId::Audio, visible, Localization::get("sfx_volume"), label_x, y,
                   control_x, control_width, control_height, SettingKey::SfxVolume,
                   kVolumeMin, kVolumeMax, settings.sfx_volume);
    }

    // Gameplay tab
    {
        const bool visible = (active_tab == TabId::Gameplay);
        float y = content_y;
        add_dropdown(reg, TabId::Gameplay, visible, Localization::get("difficulty"), label_x, y,
                     control_x, control_width, control_height, SettingKey::Difficulty,
                     {Localization::get("difficulty_easy"), Localization::get("difficulty_normal"), Localization::get("difficulty_hard"), Localization::get("difficulty_hardcore")}, settings.difficulty);
        y += row_gap;
        add_slider(reg, TabId::Gameplay, visible, Localization::get("player_lives"), label_x, y,
                   control_x, control_width, control_height, SettingKey::PlayerLives,
                   static_cast<float>(kPlayerLivesMin), static_cast<float>(kPlayerLivesMax),
                   static_cast<float>(settings.player_lives));
        y += row_gap;
        add_slider(reg, TabId::Gameplay, visible, Localization::get("enemies_per_wave"), label_x, y,
                   control_x, control_width, control_height, SettingKey::EnemiesPerWave,
                   static_cast<float>(kEnemiesPerWaveMin), static_cast<float>(kEnemiesPerWaveMax),
                   static_cast<float>(settings.enemies_per_wave));
        y += row_gap;
        add_slider(reg, TabId::Gameplay, visible, Localization::get("kills_per_wave"), label_x, y,
                   control_x, control_width, control_height, SettingKey::KillsPerWave,
                   static_cast<float>(kKillsPerWaveMin), static_cast<float>(kKillsPerWaveMax),
                   static_cast<float>(settings.kills_per_wave));
        y += row_gap;
        add_dropdown(reg, TabId::Gameplay, visible, Localization::get("infinite_lives"), label_x, y,
                     control_x, control_width, control_height, SettingKey::InfiniteLives,
                     {Localization::get("off"), Localization::get("on")}, settings.infinite_lives ? 1 : 0);
    }

    // Graphics tab
    {
        const bool visible = (active_tab == TabId::Graphics);
        float y = content_y;
        add_dropdown(reg, TabId::Graphics, visible, Localization::get("fullscreen"), label_x, y,
                     control_x, control_width, control_height, SettingKey::Fullscreen,
                     {Localization::get("off"), Localization::get("on")}, settings.fullscreen ? 1 : 0);
        y += row_gap;
        add_dropdown(reg, TabId::Graphics, visible, Localization::get("vsync"), label_x, y,
                     control_x, control_width, control_height, SettingKey::Vsync,
                     {Localization::get("off"), Localization::get("on")}, settings.vsync ? 1 : 0);
        y += row_gap;
        add_dropdown(reg, TabId::Graphics, visible, Localization::get("show_fps"), label_x, y,
                     control_x, control_width, control_height, SettingKey::ShowFps,
                     {Localization::get("off"), Localization::get("on")}, settings.show_fps ? 1 : 0);
        y += row_gap;
        add_dropdown(reg, TabId::Graphics, visible, Localization::get("screen_shake"), label_x, y,
                     control_x, control_width, control_height, SettingKey::ScreenShake,
                     {Localization::get("off"), Localization::get("on")}, settings.screen_shake ? 1 : 0);
        y += row_gap;
        add_slider(reg, TabId::Graphics, visible, Localization::get("target_fps"), label_x, y,
                   control_x, control_width, control_height, SettingKey::TargetFPS,
                   static_cast<float>(settings_ui::kTargetFPSMin), static_cast<float>(settings_ui::kTargetFPSMax),
                   static_cast<float>(settings.target_fps));
    }

    // Network tab
    {
        const bool visible = (active_tab == TabId::Network);
        float y = content_y;
        add_text_input(reg, TabId::Network, visible, Localization::get("player_name"), label_x, y,
                       control_x, control_width, control_height, SettingKey::PlayerName,
                       settings.player_name);
        y += row_gap;
        add_text_input(reg, TabId::Network, visible, Localization::get("server_ip"), label_x, y,
                       control_x, control_width, control_height, SettingKey::DefaultServerIp,
                       settings.default_server_ip);
        y += row_gap;
        add_text_input(reg, TabId::Network, visible, Localization::get("server_port"), label_x, y,
                       control_x, control_width, control_height, SettingKey::DefaultPort,
                       std::to_string(settings.default_port));
    }

    // Accessibility tab
    {
        const bool visible = (active_tab == TabId::Accessibility);
        float y = content_y;
        add_dropdown(reg, TabId::Accessibility, visible, Localization::get("high_contrast"), label_x, y,
                     control_x, control_width, control_height, SettingKey::HighContrast,
                     {Localization::get("off"), Localization::get("on")}, settings.high_contrast ? 1 : 0);
        y += row_gap;
        add_dropdown(reg, TabId::Accessibility, visible, Localization::get("font_scale"), label_x, y,
                     control_x, control_width, control_height, SettingKey::FontScale,
                     {"1.0x", "1.25x", "1.5x"}, settings.font_scale_index);
        y += row_gap;
        add_dropdown(reg, TabId::Accessibility, visible, Localization::get("language"), label_x, y,
                     control_x, control_width, control_height, SettingKey::Language,
                     {Localization::get("language_english"), Localization::get("language_spanish"), Localization::get("language_french")}, settings.language);
    }

    // Action buttons
    {
        const float action_y = height - 90.f;  // Fixed position from bottom
        const float action_w = 160.f;
        const float action_h = 44.f;
        const float action_gap = 18.f;
        const float total_w = action_w * 3.f + action_gap * 2.f;
        float action_x = center_x - total_w * 0.5f;

        auto spawn_action = [&](ActionType type) {
            auto entity = reg.spawn_entity();
            engine::game::components::UIAction action{};
            action.type = type;
            action.x = action_x;
            action.y = action_y;
            action.width = action_w;
            action.height = action_h;
            reg.emplace_component<engine::game::components::UIAction>(entity, action);
            action_x += action_w + action_gap;
        };

        spawn_action(ActionType::Apply);
        spawn_action(ActionType::Save);
        spawn_action(ActionType::Cancel);
    }
}

}  // namespace client::systems
