#include "ui_render_system.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Mouse.hpp>

#include "engine/game/components/ui/accessibility_config.hpp"
#include "engine/game/components/ui/ui_action.hpp"
#include "engine/game/components/ui/ui_dropdown.hpp"
#include "engine/game/components/ui/ui_slider.hpp"
#include "engine/game/components/ui/ui_tab.hpp"
#include "engine/game/components/ui/ui_text_input.hpp"
#include "engine/game/components/ui/ui_setting_key.hpp"
#include "components/ui_visibility.hpp"

namespace client::systems {

UIRenderSystem::UIRenderSystem(const sf::Font& font) : font_(font) {}

namespace {

struct ButtonAnimState {
    float scale = 1.0f;
    float alpha = 1.0f;
};

sf::Color color_from_rgba(std::uint32_t rgba) {
    return sf::Color(
        static_cast<std::uint8_t>((rgba >> 24) & 0xFF),
        static_cast<std::uint8_t>((rgba >> 16) & 0xFF),
        static_cast<std::uint8_t>((rgba >> 8) & 0xFF),
        static_cast<std::uint8_t>(rgba & 0xFF));
}

bool is_visible(rtype::ecs::registry& reg, rtype::ecs::entity_t entity) {
    if (auto* visibility = reg.try_get<client::components::UIVisibility>(entity)) {
        return visibility->visible;
    }
    return true;
}

std::string slider_value_label(const engine::game::components::UISlider& slider) {
    using engine::game::components::SettingKey;
    if (slider.key == SettingKey::MasterVolume ||
        slider.key == SettingKey::MusicVolume ||
        slider.key == SettingKey::SfxVolume) {
        const int percent = static_cast<int>(slider.value * 100.0f + 0.5f);
        return std::to_string(percent) + "%";
    }
    if (slider.key == SettingKey::EnemySpawnRate) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << slider.value;
        return oss.str();
    }
    return std::to_string(static_cast<int>(slider.value));
}

const char* action_label(engine::game::components::ActionType type) {
    using engine::game::components::ActionType;
    switch (type) {
        case ActionType::Apply: return "APPLY";
        case ActionType::Save: return "SAVE";
        case ActionType::Cancel: return "CANCEL";
        case ActionType::Back: return "<";
        default: return "";
    }
}

}  // namespace

void UIRenderSystem::draw(rtype::ecs::registry& reg, sf::RenderWindow& window) {
    // Check for accessibility settings in the registry
    std::optional<engine::game::components::AccessibilityConfig> accessibility_config;
    reg.view<engine::game::components::AccessibilityConfig>(
        [&](std::size_t /*eid*/, auto& config) {
            accessibility_config = config;
        });

    const bool high_contrast = accessibility_config.has_value() && accessibility_config->high_contrast;
    const float font_scale = accessibility_config.has_value() ? accessibility_config->get_font_scale() : 1.0f;
    const auto mouse_pos = sf::Mouse::getPosition(window);
    const sf::Vector2f mouse = {static_cast<float>(mouse_pos.x), static_cast<float>(mouse_pos.y)};
    static sf::Clock cursor_clock;

    // Draw background elements (overlay, panels without text)
    reg.view<engine::game::components::UITransform, engine::game::components::UIButton>(
        [&](std::size_t eid, auto& transform, auto& button) {
            const auto entity = rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(eid)};
            if (!is_visible(reg, entity)) {
                return;
            }
            if (reg.try_get<engine::game::components::UIText>(entity)) {
                return;
            }

            sf::RectangleShape box;
            box.setPosition({transform.x, transform.y});
            box.setSize({transform.width, transform.height});

            if (button.id == "overlay") {
                box.setFillColor(high_contrast ? sf::Color(0, 0, 0, 200) : sf::Color(0, 0, 0, 120));
                box.setOutlineThickness(0.0f);
            } else {
                if (high_contrast) {
                    box.setFillColor(sf::Color(0, 0, 0, 240));
                    box.setOutlineThickness(2.0f);
                    box.setOutlineColor(sf::Color(255, 255, 255, 255));
                } else {
                    box.setFillColor(sf::Color(18, 22, 30, 220));
                    box.setOutlineThickness(1.0f);
                    box.setOutlineColor(sf::Color(70, 80, 100, 180));
                }
            }

            window.draw(box);
        });

    static std::unordered_map<rtype::ecs::entity_id_t, ButtonAnimState> anim_states;
    std::unordered_set<rtype::ecs::entity_id_t> seen;

    // Draw buttons with text
    reg.view<engine::game::components::UITransform, engine::game::components::UIButton, engine::game::components::UIText>(
        [&](std::size_t eid, auto& transform, auto& button, auto& text) {
            const auto entity = rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(eid)};
            if (!is_visible(reg, entity)) {
                return;
            }
            seen.insert(static_cast<rtype::ecs::entity_id_t>(eid));
            auto& anim = anim_states[static_cast<rtype::ecs::entity_id_t>(eid)];

            const float target_scale = button.hovered ? 1.08f : 1.0f;  // More dramatic scale
            const float target_alpha = button.hovered ? 1.0f : 0.75f;
            anim.scale += (target_scale - anim.scale) * 0.2f;  // Faster animation
            anim.alpha += (target_alpha - anim.alpha) * 0.2f;

            // Arcade-style glow effect when hovered
            static sf::Clock glow_clock;
            const float glow_pulse = std::abs(std::sin(glow_clock.getElapsedTime().asSeconds() * 3.0f));

            sf::RectangleShape box;
            const float scaled_w = transform.width * anim.scale;
            const float scaled_h = transform.height * anim.scale;
            box.setSize({scaled_w, scaled_h});
            box.setOrigin({scaled_w * 0.5f, scaled_h * 0.5f});
            box.setPosition({transform.x + transform.width * 0.5f, transform.y + transform.height * 0.5f});

            // Arcade retro style: dark background with bright colored borders
            if (high_contrast) {
                box.setFillColor(button.hovered ? sf::Color(20, 20, 40, 255)
                                                : sf::Color(0, 0, 0, 240));
                box.setOutlineThickness(3.0f);
                box.setOutlineColor(button.hovered ? sf::Color(255, 255, 0, 255) : sf::Color(200, 200, 200, 255));
            } else {
                // Dark semi-transparent background - more transparent
                box.setFillColor(sf::Color(10, 10, 30, static_cast<std::uint8_t>(80 * anim.alpha)));

                // Bright arcade borders that pulse when hovered
                box.setOutlineThickness(button.hovered ? 3.0f : 2.0f);
                if (button.hovered) {
                    const std::uint8_t pulse_alpha = static_cast<std::uint8_t>(180 + 75 * glow_pulse);
                    // Get button color from text to match
                    sf::Color border_color;
                    if (button.id == "play") {
                        border_color = sf::Color(0, 255, 0, pulse_alpha);  // Green
                    } else if (button.id == "settings") {
                        border_color = sf::Color(0, 255, 255, pulse_alpha);  // Cyan
                    } else if (button.id == "quit") {
                        border_color = sf::Color(255, 0, 255, pulse_alpha);  // Magenta
                    } else {
                        border_color = sf::Color(255, 255, 0, pulse_alpha);  // Yellow default
                    }
                    box.setOutlineColor(border_color);
                } else {
                    box.setOutlineColor(sf::Color(50, 50, 80, 150));  // Dim when not hovered
                }
            }
            window.draw(box);

            const unsigned scaled_font_size = static_cast<unsigned>(text.font_size * font_scale);
            sf::Text label(font_, text.text, scaled_font_size);

            if (high_contrast) {
                label.setFillColor(button.hovered ? sf::Color(255, 255, 0) : sf::Color(255, 255, 255));
            } else {
                sf::Color text_color = color_from_rgba(text.color);
                // Add pulsing brightness effect when hovered
                if (button.hovered) {
                    const std::uint8_t pulse_brightness = static_cast<std::uint8_t>(200 + 55 * glow_pulse);
                    text_color.r = std::min(255, static_cast<int>(text_color.r) * pulse_brightness / 255);
                    text_color.g = std::min(255, static_cast<int>(text_color.g) * pulse_brightness / 255);
                    text_color.b = std::min(255, static_cast<int>(text_color.b) * pulse_brightness / 255);
                }
                label.setFillColor(text_color);
            }

            auto bounds = label.getLocalBounds();
            label.setPosition({
                transform.x + (transform.width - bounds.size.x) * 0.5f - bounds.position.x,
                transform.y + (transform.height - bounds.size.y) * 0.5f - bounds.position.y
            });
            window.draw(label);
        });

    for (auto it = anim_states.begin(); it != anim_states.end(); ) {
        if (seen.find(it->first) == seen.end()) {
            it = anim_states.erase(it);
        } else {
            ++it;
        }
    }

    const sf::Color control_fill = high_contrast ? sf::Color(0, 0, 0, 235) : sf::Color(26, 30, 38, 220);
    const sf::Color control_outline = high_contrast ? sf::Color(255, 255, 255, 255) : sf::Color(80, 90, 110, 190);
    const sf::Color control_text = high_contrast ? sf::Color(255, 255, 255) : sf::Color(230, 235, 245);
    const sf::Color accent = high_contrast ? sf::Color(255, 255, 0) : sf::Color(120, 170, 230);

    // Tabs
    reg.view<engine::game::components::UITab>(
        [&](std::size_t eid, auto& tab) {
            const auto entity = rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(eid)};
            if (!is_visible(reg, entity)) {
                return;
            }
            sf::RectangleShape box;
            box.setPosition({tab.x, tab.y});
            box.setSize({tab.width, tab.height});
            if (tab.active) {
                box.setFillColor(high_contrast ? sf::Color(20, 20, 20, 255) : sf::Color(40, 56, 80, 230));
                box.setOutlineThickness(2.0f);
                box.setOutlineColor(accent);
            } else {
                box.setFillColor(control_fill);
                box.setOutlineThickness(1.0f);
                box.setOutlineColor(control_outline);
            }
            window.draw(box);

            const unsigned scaled_font_size = static_cast<unsigned>(18 * font_scale);
            sf::Text label(font_, tab.name, scaled_font_size);
            label.setFillColor(tab.active ? accent : control_text);
            auto bounds = label.getLocalBounds();
            label.setPosition({
                tab.x + (tab.width - bounds.size.x) * 0.5f - bounds.position.x,
                tab.y + (tab.height - bounds.size.y) * 0.5f - bounds.position.y
            });
            window.draw(label);
        });

    // Sliders
    reg.view<engine::game::components::UISlider>(
        [&](std::size_t eid, auto& slider) {
            const auto entity = rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(eid)};
            if (!is_visible(reg, entity)) {
                return;
            }
            sf::RectangleShape track;
            const float track_h = std::max(4.0f, slider.height * 0.25f);
            track.setPosition({slider.x, slider.y + slider.height * 0.5f - track_h * 0.5f});
            track.setSize({slider.width, track_h});
            track.setFillColor(high_contrast ? sf::Color(255, 255, 255, 80) : sf::Color(90, 100, 120, 160));
            window.draw(track);

            const float range = std::max(0.001f, slider.max_value - slider.min_value);
            const float t = std::clamp((slider.value - slider.min_value) / range, 0.0f, 1.0f);
            const float handle_w = 12.f;
            const float handle_h = slider.height + 6.f;
            sf::RectangleShape handle;
            handle.setSize({handle_w, handle_h});
            handle.setOrigin({handle_w * 0.5f, handle_h * 0.5f});
            handle.setPosition({slider.x + t * slider.width, slider.y + slider.height * 0.5f});
            handle.setFillColor(slider.dragging ? accent : control_outline);
            window.draw(handle);

            const unsigned scaled_font_size = static_cast<unsigned>(16 * font_scale);
            sf::Text value_text(font_, slider_value_label(slider), scaled_font_size);
            value_text.setFillColor(control_text);
            auto bounds = value_text.getLocalBounds();
            value_text.setPosition({
                slider.x + slider.width + 12.f,
                slider.y + slider.height * 0.5f - bounds.size.y * 0.5f - bounds.position.y
            });
            window.draw(value_text);
        });

    // Dropdowns
    reg.view<engine::game::components::UIDropdown>(
        [&](std::size_t eid, auto& dropdown) {
            const auto entity = rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(eid)};
            if (!is_visible(reg, entity)) {
                return;
            }
            sf::RectangleShape box;
            box.setPosition({dropdown.x, dropdown.y});
            box.setSize({dropdown.width, dropdown.height});
            box.setFillColor(control_fill);
            box.setOutlineThickness(1.0f);
            box.setOutlineColor(control_outline);
            window.draw(box);

            std::string selected = "";
            if (!dropdown.options.empty() &&
                dropdown.selected_index >= 0 &&
                dropdown.selected_index < static_cast<int>(dropdown.options.size())) {
                selected = dropdown.options[dropdown.selected_index];
            }
            const unsigned scaled_font_size = static_cast<unsigned>(16 * font_scale);
            sf::Text label(font_, selected, scaled_font_size);
            label.setFillColor(control_text);
            auto bounds = label.getLocalBounds();
            label.setPosition({
                dropdown.x + 10.f - bounds.position.x,
                dropdown.y + (dropdown.height - bounds.size.y) * 0.5f - bounds.position.y
            });
            window.draw(label);

            if (dropdown.open) {
                const float list_y = dropdown.y + dropdown.height + 2.f;  // Small gap below the control
                const float option_height = std::max(dropdown.height, 38.f);  // Increased from 32 to 38 for better visibility
                sf::RectangleShape list_box;
                list_box.setPosition({dropdown.x, list_y});
                list_box.setSize({dropdown.width, option_height * dropdown.options.size()});
                list_box.setFillColor(control_fill);
                list_box.setOutlineThickness(1.0f);
                list_box.setOutlineColor(control_outline);
                window.draw(list_box);

                for (std::size_t i = 0; i < dropdown.options.size(); ++i) {
                    const float row_y = list_y + option_height * static_cast<float>(i);
                    const bool hovered = mouse.x >= dropdown.x &&
                                         mouse.x <= dropdown.x + dropdown.width &&
                                         mouse.y >= row_y &&
                                         mouse.y <= row_y + option_height;
                    if (hovered) {
                        sf::RectangleShape hover;
                        hover.setPosition({dropdown.x, row_y});
                        hover.setSize({dropdown.width, option_height});
                        hover.setFillColor(high_contrast ? sf::Color(60, 60, 60, 255) : sf::Color(50, 70, 100, 200));
                        window.draw(hover);
                    }
                    sf::Text option(font_, dropdown.options[i], scaled_font_size);
                    option.setFillColor(hovered ? accent : control_text);
                    auto opt_bounds = option.getLocalBounds();
                    option.setPosition({
                        dropdown.x + 10.f - opt_bounds.position.x,
                        row_y + (option_height - opt_bounds.size.y) * 0.5f - opt_bounds.position.y
                    });
                    window.draw(option);
                }
            }
        });

    // Text inputs
    reg.view<engine::game::components::UITextInput>(
        [&](std::size_t eid, auto& field) {
            const auto entity = rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(eid)};
            if (!is_visible(reg, entity)) {
                return;
            }
            sf::RectangleShape box;
            box.setPosition({field.x, field.y});
            box.setSize({field.width, field.height});
            box.setFillColor(control_fill);
            box.setOutlineThickness(field.focused ? 2.0f : 1.0f);
            box.setOutlineColor(field.focused ? accent : control_outline);
            window.draw(box);

            const unsigned scaled_font_size = static_cast<unsigned>(16 * font_scale);
            sf::Text value(font_, field.text, scaled_font_size);
            value.setFillColor(control_text);
            auto bounds = value.getLocalBounds();
            value.setPosition({
                field.x + 10.f - bounds.position.x,
                field.y + (field.height - bounds.size.y) * 0.5f - bounds.position.y
            });
            window.draw(value);

            if (field.focused) {
                const float blink = std::fmod(cursor_clock.getElapsedTime().asSeconds(), 1.0f);
                if (blink < 0.5f) {
                    const auto cursor_pos = value.findCharacterPos(field.cursor_index);
                    sf::RectangleShape cursor;
                    cursor.setSize({2.f, static_cast<float>(scaled_font_size)});
                    cursor.setFillColor(accent);
                    cursor.setPosition({cursor_pos.x, field.y + (field.height - scaled_font_size) * 0.5f});
                    window.draw(cursor);
                }
            }
        });

    // Action buttons
    reg.view<engine::game::components::UIAction>(
        [&](std::size_t eid, auto& button) {
            const auto entity = rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(eid)};
            if (!is_visible(reg, entity)) {
                return;
            }
            const bool hovered = mouse.x >= button.x &&
                                 mouse.x <= button.x + button.width &&
                                 mouse.y >= button.y &&
                                 mouse.y <= button.y + button.height;
            sf::RectangleShape box;
            box.setPosition({button.x, button.y});
            box.setSize({button.width, button.height});
            if (high_contrast) {
                box.setFillColor(hovered ? sf::Color(40, 40, 40, 255) : sf::Color(0, 0, 0, 240));
                box.setOutlineThickness(2.0f);
                box.setOutlineColor(hovered ? accent : sf::Color(200, 200, 200));
            } else {
                box.setFillColor(hovered ? sf::Color(40, 60, 96, 230) : sf::Color(28, 32, 40, 210));
                box.setOutlineThickness(1.0f);
                box.setOutlineColor(hovered ? accent : control_outline);
            }
            window.draw(box);

            // Use larger font size for Back button
            const unsigned base_font_size = (button.type == engine::game::components::ActionType::Back) ? 36u : 18u;
            const unsigned scaled_font_size = static_cast<unsigned>(base_font_size * font_scale);
            sf::Text label(font_, action_label(button.type), scaled_font_size);
            label.setFillColor(hovered ? accent : control_text);
            auto bounds = label.getLocalBounds();
            label.setPosition({
                button.x + (button.width - bounds.size.x) * 0.5f - bounds.position.x,
                button.y + (button.height - bounds.size.y) * 0.5f - bounds.position.y
            });
            window.draw(label);
        });

    // Draw standalone text elements (titles, labels, etc.)
    reg.view<engine::game::components::UITransform, engine::game::components::UIText>(
        [&](std::size_t eid, auto& transform, auto& text) {
            const auto entity = rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(eid)};
            if (!is_visible(reg, entity)) {
                return;
            }
            if (reg.try_get<engine::game::components::UIButton>(entity)) {
                return;
            }
            const unsigned scaled_font_size = static_cast<unsigned>(text.font_size * font_scale);
            sf::Text label(font_, text.text, scaled_font_size);

            if (high_contrast) {
                // High contrast: make text white or yellow depending on context
                label.setFillColor(sf::Color(255, 255, 255));
            } else {
                label.setFillColor(color_from_rgba(text.color));
            }

            label.setPosition({transform.x, transform.y});
            window.draw(label);
        });
}

}  // namespace client::systems
