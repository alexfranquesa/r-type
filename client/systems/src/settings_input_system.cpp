#include "settings_input_system.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>

#include "engine/game/components/ui/ui_action.hpp"
#include "engine/game/components/ui/ui_dropdown.hpp"
#include "engine/game/components/ui/ui_slider.hpp"
#include "engine/game/components/ui/ui_tab.hpp"
#include "engine/game/components/ui/ui_text_input.hpp"
#include "engine/game/components/ui/ui_setting_key.hpp"
#include "components/ui_tab_owner.hpp"
#include "components/ui_visibility.hpp"
#include "settings_ui_constants.hpp"
#include "localization.hpp"

namespace client::systems {

namespace {

bool hit_test(float x, float y, float w, float h, const sf::Vector2f& mouse) {
    return mouse.x >= x && mouse.x <= x + w && mouse.y >= y && mouse.y <= y + h;
}

bool is_printable(char32_t unicode) {
    if (unicode < 32 || unicode >= 127) {
        return false;
    }
    return std::isprint(static_cast<unsigned char>(unicode)) != 0;
}

bool slider_is_integer(engine::game::components::SettingKey key) {
    using engine::game::components::SettingKey;
    switch (key) {
        case SettingKey::PlayerLives:
        case SettingKey::EnemiesPerWave:
        case SettingKey::KillsPerWave:
        case SettingKey::TargetFPS:
            return true;
        default:
            return false;
    }
}

void set_setting_from_slider(GameSettings& settings,
                             engine::game::components::SettingKey key,
                             float value) {
    using engine::game::components::SettingKey;
    using namespace client::systems::settings_ui;
    switch (key) {
        case SettingKey::MasterVolume:
            settings.master_volume = std::clamp(value, kVolumeMin, kVolumeMax);
            break;
        case SettingKey::MusicVolume:
            settings.music_volume = std::clamp(value, kVolumeMin, kVolumeMax);
            break;
        case SettingKey::SfxVolume:
            settings.sfx_volume = std::clamp(value, kVolumeMin, kVolumeMax);
            break;
        case SettingKey::PlayerLives:
            settings.player_lives = std::clamp(static_cast<int>(value), kPlayerLivesMin, kPlayerLivesMax);
            break;
        case SettingKey::EnemiesPerWave:
            settings.enemies_per_wave = std::clamp(static_cast<int>(value), kEnemiesPerWaveMin, kEnemiesPerWaveMax);
            break;
        case SettingKey::KillsPerWave:
            settings.kills_per_wave = std::clamp(static_cast<int>(value), kKillsPerWaveMin, kKillsPerWaveMax);
            break;
        case SettingKey::EnemySpawnRate:
            settings.enemy_spawn_rate = std::clamp(value, kEnemySpawnRateMin, kEnemySpawnRateMax);
            break;
        case SettingKey::TargetFPS:
            settings.target_fps = std::clamp(static_cast<int>(value), kTargetFPSMin, kTargetFPSMax);
            break;
        default:
            break;
    }
}

void set_setting_from_dropdown(GameSettings& settings,
                               engine::game::components::SettingKey key,
                               int selected_index) {
    using engine::game::components::SettingKey;
    using namespace client::systems::settings_ui;
    const bool enabled = (selected_index > 0);
    switch (key) {
        case SettingKey::Difficulty:
            settings.difficulty = std::clamp(selected_index, kDifficultyMin, kDifficultyMax);
            break;
        case SettingKey::InfiniteLives:
            settings.infinite_lives = enabled;
            break;
        case SettingKey::Fullscreen:
            settings.fullscreen = enabled;
            break;
        case SettingKey::Vsync:
            settings.vsync = enabled;
            break;
        case SettingKey::ShowFps:
            settings.show_fps = enabled;
            break;
        case SettingKey::ScreenShake:
            settings.screen_shake = enabled;
            break;
        case SettingKey::MusicEnabled:
            settings.music_enabled = enabled;
            break;
        case SettingKey::AutoConnect:
            settings.auto_connect = enabled;
            break;
        case SettingKey::HighContrast:
            settings.high_contrast = enabled;
            break;
        case SettingKey::FontScale:
            settings.font_scale_index = std::clamp(selected_index, 0, 2);
            break;
        case SettingKey::Language:
            settings.language = std::clamp(selected_index, 0, 2);
            break;
        default:
            break;
    }
}

void set_setting_from_text(GameSettings& settings,
                           engine::game::components::SettingKey key,
                           const std::string& text) {
    using engine::game::components::SettingKey;
    using namespace client::systems::settings_ui;
    switch (key) {
        case SettingKey::PlayerName:
            settings.player_name = text;
            break;
        case SettingKey::DefaultServerIp:
            settings.default_server_ip = text;
            break;
        case SettingKey::DefaultPort: {
            bool digits = !text.empty();
            for (char c : text) {
                if (!std::isdigit(static_cast<unsigned char>(c))) {
                    digits = false;
                    break;
                }
            }
            if (digits) {
                try {
                    settings.default_port = std::clamp(std::stoi(text), kDefaultPortMin, kDefaultPortMax);
                } catch (...) {
                }
            }
            break;
        }
        default:
            break;
    }
}

engine::game::components::TabId current_active_tab(rtype::ecs::registry& reg) {
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

void reset_tab_interaction(rtype::ecs::registry& reg, engine::game::components::TabId active_tab) {
    reg.view<engine::game::components::UITextInput, client::components::UITabOwner, client::components::UIVisibility>(
        [&](std::size_t /*eid*/, auto& input, auto& owner, auto& visibility) {
            if (owner.tab != active_tab) {
                input.focused = false;
                visibility.visible = false;
            }
        });
    reg.view<engine::game::components::UISlider, client::components::UITabOwner, client::components::UIVisibility>(
        [&](std::size_t /*eid*/, auto& slider, auto& owner, auto& visibility) {
            slider.dragging = false;
            visibility.visible = (owner.tab == active_tab);
        });
    reg.view<engine::game::components::UIDropdown, client::components::UITabOwner, client::components::UIVisibility>(
        [&](std::size_t /*eid*/, auto& dropdown, auto& owner, auto& visibility) {
            dropdown.open = false;
            visibility.visible = (owner.tab == active_tab);
        });
    reg.view<client::components::UITabOwner, client::components::UIVisibility>(
        [&](std::size_t /*eid*/, auto& owner, auto& visibility) {
            visibility.visible = (owner.tab == active_tab);
        });
}

}  // namespace

SettingsMenuAction SettingsInputSystem::update(rtype::ecs::registry& reg,
                                               GameSettings& settings,
                                               const SettingsInputState& input) {
    using engine::game::components::ActionType;
    using engine::game::components::TabId;

    SettingsMenuAction action = SettingsMenuAction::None;
    if (input.escape) {
        settings.load_from_file();
        return SettingsMenuAction::Cancel;
    }

    // Tabs
    if (input.mouse_pressed) {
        bool tab_clicked = false;
        TabId new_tab = current_active_tab(reg);
        reg.view<engine::game::components::UITab>(
            [&](std::size_t /*eid*/, auto& tab) {
                if (tab_clicked) {
                    return;
                }
                if (hit_test(tab.x, tab.y, tab.width, tab.height, input.mouse_pos)) {
                    new_tab = tab.id;
                    tab_clicked = true;
                }
            });
        if (tab_clicked) {
            reg.view<engine::game::components::UITab>(
                [&](std::size_t /*eid*/, auto& tab) {
                    tab.active = (tab.id == new_tab);
                });
            reset_tab_interaction(reg, new_tab);
        }
    }

    // Sliders
    if (input.mouse_pressed) {
        bool dragging_set = false;
        reg.view<engine::game::components::UISlider, client::components::UIVisibility>(
            [&](std::size_t /*eid*/, auto& slider, auto& visibility) {
                if (!visibility.visible) {
                    slider.dragging = false;
                    return;
                }
                if (hit_test(slider.x, slider.y, slider.width, slider.height, input.mouse_pos)) {
                    slider.dragging = true;
                    dragging_set = true;
                } else {
                    slider.dragging = false;
                }
            });
        if (!dragging_set) {
            reg.view<engine::game::components::UISlider>([&](std::size_t /*eid*/, auto& slider) {
                slider.dragging = false;
            });
        }
    }

    if (input.mouse_released) {
        reg.view<engine::game::components::UISlider>(
            [&](std::size_t /*eid*/, auto& slider) {
                slider.dragging = false;
            });
    }

    if (input.mouse_down) {
        reg.view<engine::game::components::UISlider, client::components::UIVisibility>(
            [&](std::size_t /*eid*/, auto& slider, auto& visibility) {
                if (!visibility.visible || !slider.dragging) {
                    return;
                }
                const float t = std::clamp((input.mouse_pos.x - slider.x) / slider.width, 0.0f, 1.0f);
                float value = slider.min_value + t * (slider.max_value - slider.min_value);
                if (slider_is_integer(slider.key)) {
                    value = std::round(value);
                }
                if (value != slider.value) {
                    slider.value = value;
                    set_setting_from_slider(settings, slider.key, value);
                }
            });
    }

    // Dropdowns
    if (input.mouse_pressed) {
        bool click_consumed = false;
        reg.view<engine::game::components::UIDropdown, client::components::UIVisibility>(
            [&](std::size_t /*eid*/, auto& dropdown, auto& visibility) {
                if (!visibility.visible) {
                    dropdown.open = false;
                    return;
                }
                const bool in_box = hit_test(dropdown.x, dropdown.y, dropdown.width, dropdown.height, input.mouse_pos);
                if (!dropdown.open) {
                    if (in_box) {
                        dropdown.open = true;
                        click_consumed = true;
                    }
                    return;
                }

                const float list_y = dropdown.y + dropdown.height + 2.f;  // Match render system gap
                const float option_height = std::max(dropdown.height, 38.f);  // Increased from 32 to 38
                const float list_h = option_height * static_cast<float>(dropdown.options.size());
                const bool in_list = hit_test(dropdown.x, list_y, dropdown.width, list_h, input.mouse_pos);
                if (in_list) {
                    const int index = static_cast<int>((input.mouse_pos.y - list_y) / option_height);
                    if (index >= 0 && index < static_cast<int>(dropdown.options.size())) {
                        dropdown.selected_index = index;
                        const int old_language = settings.language;
                        set_setting_from_dropdown(settings, dropdown.key, dropdown.selected_index);
                        // If language changed, update localization and mark for UI rebuild
                        if (dropdown.key == engine::game::components::SettingKey::Language && old_language != settings.language) {
                            Localization::setLanguage(static_cast<Language>(settings.language));
                            action = SettingsMenuAction::Apply;  // Trigger UI rebuild
                        }
                    }
                    dropdown.open = false;
                    click_consumed = true;
                } else if (in_box) {
                    dropdown.open = true;
                    click_consumed = true;
                } else {
                    // Click outside - close dropdown but don't consume the click
                    dropdown.open = false;
                }
            });
    }

    // Text inputs focus
    if (input.mouse_pressed) {
        bool focused_any = false;
        reg.view<engine::game::components::UITextInput, client::components::UIVisibility>(
            [&](std::size_t /*eid*/, auto& field, auto& visibility) {
                if (!visibility.visible) {
                    field.focused = false;
                    return;
                }
                const bool inside = hit_test(field.x, field.y, field.width, field.height, input.mouse_pos);
                if (!focused_any && inside) {
                    field.focused = true;
                    focused_any = true;
                    field.cursor_index = std::min(field.cursor_index, field.text.size());
                } else {
                    field.focused = false;
                }
            });
        if (!focused_any) {
            reg.view<engine::game::components::UITextInput>(
                [&](std::size_t /*eid*/, auto& field) {
                    field.focused = false;
                });
        }
    }

    // Text input editing
    reg.view<engine::game::components::UITextInput, client::components::UIVisibility>(
        [&](std::size_t /*eid*/, auto& field, auto& visibility) {
            if (!visibility.visible || !field.focused) {
                return;
            }
            bool changed = false;
            for (char32_t unicode : input.text_entered) {
                if (!is_printable(unicode)) {
                    continue;
                }
                field.text.insert(field.cursor_index, 1, static_cast<char>(unicode));
                field.cursor_index++;
                changed = true;
            }
            if (input.backspace && field.cursor_index > 0 && !field.text.empty()) {
                field.text.erase(field.cursor_index - 1, 1);
                field.cursor_index--;
                changed = true;
            }
            if (input.left && field.cursor_index > 0) {
                field.cursor_index--;
            }
            if (input.right && field.cursor_index < field.text.size()) {
                field.cursor_index++;
            }
            if (input.enter) {
                field.focused = false;
            }
            if (changed) {
                set_setting_from_text(settings, field.key, field.text);
            }
        });

    // Action buttons
    if (input.mouse_pressed || input.mouse_down || input.mouse_released) {
        reg.view<engine::game::components::UIAction>(
            [&](std::size_t /*eid*/, auto& button) {
                const bool inside = hit_test(button.x, button.y, button.width, button.height, input.mouse_pos);
                if (input.mouse_pressed && inside) {
                    button.pressed = true;
                }
                if (input.mouse_down && button.pressed && !inside) {
                    button.pressed = false;
                }
                if (input.mouse_released) {
                    if (button.pressed && inside) {
                        if (button.type == ActionType::Apply) {
                            settings.default_port = std::clamp(settings.default_port,
                                                               settings_ui::kDefaultPortMin,
                                                               settings_ui::kDefaultPortMax);
                            action = SettingsMenuAction::Apply;
                        } else if (button.type == ActionType::Save) {
                            settings.default_port = std::clamp(settings.default_port,
                                                               settings_ui::kDefaultPortMin,
                                                               settings_ui::kDefaultPortMax);
                            settings.save_to_file();
                        } else if (button.type == ActionType::Cancel) {
                            settings.load_from_file();
                            action = SettingsMenuAction::Cancel;
                        } else if (button.type == ActionType::Back) {
                            action = SettingsMenuAction::Cancel;
                        }
                    }
                    button.pressed = false;
                }
            });
    }

    return action;
}

}  // namespace client::systems
