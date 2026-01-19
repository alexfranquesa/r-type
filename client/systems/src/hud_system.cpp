#include "hud_system.hpp"

#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cmath>

#include "engine/game/components/gameplay/spectator.hpp"
#include "engine/game/components/gameplay/enemy_type.hpp"

namespace client::systems {

HUDSystem::HUDSystem(const sf::Font& font) : font_(font) {
    // Start with transition active for level 1
    transition_active_ = true;
    transition_level_ = 1;
    transition_timer_ = 0.0f;
    last_level_ = 1;
}

void HUDSystem::update(float dt) {
    if (transition_active_) {
        transition_timer_ += dt;
        if (transition_timer_ >= TRANSITION_TOTAL) {
            transition_active_ = false;
            transition_timer_ = 0.0f;
        }
    }
}

void HUDSystem::reset_transition() {
    last_level_ = 0;
    transition_active_ = true;
    transition_level_ = 1;
    transition_timer_ = 0.0f;
}

void HUDSystem::draw_level_transition(sf::RenderWindow& window) {
    if (!transition_active_) {
        return;
    }

    // Calculate alpha based on transition phase
    float alpha = 0.0f;
    if (transition_timer_ < TRANSITION_FADE_IN) {
        // Fade in
        alpha = transition_timer_ / TRANSITION_FADE_IN;
    } else if (transition_timer_ < TRANSITION_FADE_IN + TRANSITION_HOLD) {
        // Hold at full
        alpha = 1.0f;
    } else {
        // Fade out
        float fade_progress = (transition_timer_ - TRANSITION_FADE_IN - TRANSITION_HOLD) / TRANSITION_FADE_OUT;
        alpha = 1.0f - fade_progress;
    }

    alpha = std::clamp(alpha, 0.0f, 1.0f);
    const auto alpha_byte = static_cast<std::uint8_t>(alpha * 255.0f);

    auto win_size = window.getSize();
    const float center_x = static_cast<float>(win_size.x) * 0.5f;
    const float center_y = static_cast<float>(win_size.y) * 0.5f;

    // Semi-transparent background overlay
    sf::RectangleShape overlay;
    overlay.setPosition({0.f, 0.f});
    overlay.setSize({static_cast<float>(win_size.x), static_cast<float>(win_size.y)});
    overlay.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(alpha * 150.0f)));
    window.draw(overlay);

    // "LEVEL X" text - large and prominent
    std::string level_text = "LEVEL " + std::to_string(transition_level_);
    sf::Text level_txt(font_, level_text, 72);
    level_txt.setFillColor(sf::Color(255, 255, 255, alpha_byte));
    level_txt.setOutlineColor(sf::Color(0, 0, 0, alpha_byte));
    level_txt.setOutlineThickness(3.f);

    auto level_bounds = level_txt.getLocalBounds();
    level_txt.setPosition({
        center_x - level_bounds.size.x * 0.5f,
        center_y - 60.f
    });
    window.draw(level_txt);

    // Level name subtitle
    const char* level_name = getLevelName(transition_level_);
    sf::Text name_txt(font_, level_name, 36);
    name_txt.setFillColor(sf::Color(200, 200, 255, alpha_byte));
    name_txt.setOutlineColor(sf::Color(0, 0, 0, alpha_byte));
    name_txt.setOutlineThickness(2.f);

    auto name_bounds = name_txt.getLocalBounds();
    name_txt.setPosition({
        center_x - name_bounds.size.x * 0.5f,
        center_y + 30.f
    });
    window.draw(name_txt);
}

void HUDSystem::update_fps(float delta_time) {
    if (delta_time <= 0.0f) {
        return; // Avoid division by zero or invalid delta
    }

    fps_accumulator_ += delta_time;
    fps_frame_count_++;

    // Update display FPS twice per second for smooth but stable reading
    if (fps_accumulator_ >= fps_update_interval_) {
        fps_display_ = static_cast<float>(fps_frame_count_) / fps_accumulator_;
        fps_accumulator_ = 0.0f;
        fps_frame_count_ = 0;
    }
}

void HUDSystem::draw_fps(sf::RenderWindow& window, bool show_fps) {
    if (!show_fps) {
        return;
    }

    std::ostringstream oss;
    oss << "FPS: " << std::fixed << std::setprecision(1) << fps_display_;

    sf::Text fps_text(font_, oss.str(), 16);
    fps_text.setFillColor(sf::Color::Yellow);
    fps_text.setPosition({10.f, 10.f});

    window.draw(fps_text);
}

void HUDSystem::draw(rtype::ecs::registry& reg, sf::RenderWindow& window, std::uint16_t /*local_player_id*/) {
    using engine::game::components::Health;
    using engine::game::components::Position;
    using engine::game::components::Owner;
    using engine::game::components::Sprite;
    using engine::game::components::GameStats;
    using engine::game::components::Lives;

    std::size_t players = 0;
    std::size_t enemies = 0;
    std::size_t entities = 0;
    std::uint32_t score = 0;
    std::uint16_t wave = 1;
    std::uint16_t current_level = 1;
    std::uint16_t kills_remaining = 15;

    // Read global stats if present
    reg.view<GameStats>([&](std::size_t /*eid*/, GameStats& gs) {
        score = gs.score;
        wave = gs.wave;
        current_level = gs.current_level;
        kills_remaining = gs.kills_remaining();
    });

    // Detect level change and trigger transition (only for valid levels 1-5)
    constexpr std::uint16_t MAX_LEVEL = 5;
    if (current_level != last_level_ && current_level > 0 && current_level <= MAX_LEVEL) {
        last_level_ = current_level;
        transition_level_ = current_level;
        transition_active_ = true;
        transition_timer_ = 0.0f;
    }

    reg.view<Position>([&](std::size_t /*eid*/, Position& /*p*/) {
        ++entities;
    });

    // Count players/enemies based on Owner + Sprite (client no longer has Faction)
    reg.view<Owner, Sprite>([&](std::size_t /*eid*/, Owner& owner, Sprite& sprite) {
        if (owner.player_id > 0) {
            ++players;
        } else if (sprite.texture_id == "enemy_basic") {
            ++enemies;
        }
    });

    auto win_size = window.getSize();

    // Right panel: game stats
    {
        std::ostringstream oss;
        oss << "LEVEL " << current_level << "\n";
        oss << "Next: " << kills_remaining << " kills\n";
        oss << "Score: " << score;
        sf::Text txt(font_, oss.str(), 14);
        txt.setFillColor(sf::Color::White);
        const float panel_w = 200.f;
        sf::Vector2f pos{static_cast<float>(win_size.x) - panel_w - 10.f, 10.f};
        txt.setPosition(pos);

        sf::RectangleShape bg;
        bg.setPosition(pos - sf::Vector2f{4.f, 4.f});
        bg.setSize({panel_w, 65.f});
        bg.setFillColor(sf::Color(0, 0, 0, 160));
        bg.setOutlineThickness(1.f);
        bg.setOutlineColor(sf::Color(80, 80, 100));

        window.draw(bg);
        window.draw(txt);
    }

    // Level progress bar (below right panel)
    {
        const float bar_w = 190.f;
        const float bar_h = 10.f;
        sf::Vector2f bar_pos{static_cast<float>(win_size.x) - bar_w - 15.f, 80.f};

        // Background
        sf::RectangleShape bar_bg;
        bar_bg.setPosition(bar_pos);
        bar_bg.setSize({bar_w, bar_h});
        bar_bg.setFillColor(sf::Color(40, 40, 60, 200));
        bar_bg.setOutlineThickness(1.f);
        bar_bg.setOutlineColor(sf::Color(80, 80, 100));

        // Get kills info from GameStats
        std::uint16_t kills_this = 0;
        std::uint16_t kills_needed = 15;
        reg.view<GameStats>([&](std::size_t /*eid*/, GameStats& gs) {
            kills_this = gs.kills_this_level;
            kills_needed = gs.kills_to_next_level;
        });

        float progress = (kills_needed > 0)
            ? std::clamp(static_cast<float>(kills_this) / static_cast<float>(kills_needed), 0.f, 1.f)
            : 0.f;

        // Progress fill with color gradient based on progress
        sf::RectangleShape bar_fill;
        bar_fill.setPosition(bar_pos);
        bar_fill.setSize({bar_w * progress, bar_h});

        // Color transitions from blue (0%) -> green (50%) -> gold (100%)
        sf::Color fill_color;
        if (progress < 0.5f) {
            float t = progress * 2.f;
            fill_color = sf::Color(
                static_cast<std::uint8_t>(50 * (1.f - t)),
                static_cast<std::uint8_t>(150 + 50 * t),
                static_cast<std::uint8_t>(255 * (1.f - t))
            );
        } else {
            float t = (progress - 0.5f) * 2.f;
            fill_color = sf::Color(
                static_cast<std::uint8_t>(255 * t),
                static_cast<std::uint8_t>(200 - 50 * t),
                static_cast<std::uint8_t>(0)
            );
        }
        bar_fill.setFillColor(fill_color);

        window.draw(bar_bg);
        window.draw(bar_fill);
    }

    // Boss health bar (top center, only shown when boss exists)
    {
        using engine::game::components::EnemyTypeComponent;
        using engine::game::components::EnemyType;
        
        bool boss_found = false;
        int boss_hp = 0;
        int boss_max_hp = 1;
        
        reg.view<Health, EnemyTypeComponent>([&](std::size_t /*eid*/, Health& hp, EnemyTypeComponent& etype) {
            if (etype.type == EnemyType::Boss && hp.current > 0) {
                boss_found = true;
                boss_hp = hp.current;
                boss_max_hp = hp.max;
            }
        });
        
        if (boss_found) {
            const float bar_w = 300.f;
            const float bar_h = 20.f;
            const float bar_x = (static_cast<float>(win_size.x) - bar_w) * 0.5f;
            const float bar_y = 60.f;
            
            // Background
            sf::RectangleShape boss_bar_bg;
            boss_bar_bg.setPosition({bar_x, bar_y});
            boss_bar_bg.setSize({bar_w, bar_h});
            boss_bar_bg.setFillColor(sf::Color(40, 0, 0, 220));
            boss_bar_bg.setOutlineThickness(2.f);
            boss_bar_bg.setOutlineColor(sf::Color(255, 100, 100));
            
            // Health fill
            float ratio = std::clamp(static_cast<float>(boss_hp) / static_cast<float>(boss_max_hp), 0.f, 1.f);
            sf::RectangleShape boss_bar_fill;
            boss_bar_fill.setPosition({bar_x, bar_y});
            boss_bar_fill.setSize({bar_w * ratio, bar_h});
            boss_bar_fill.setFillColor(sf::Color(255, 50, 50));  // Red health bar
            
            // Boss label
            sf::Text boss_label(font_, "BOSS", 14);
            boss_label.setFillColor(sf::Color::White);
            boss_label.setPosition({bar_x + bar_w * 0.5f - 20.f, bar_y - 18.f});
            
            // HP text
            std::ostringstream hp_oss;
            hp_oss << boss_hp << "/" << boss_max_hp;
            sf::Text hp_text(font_, hp_oss.str(), 12);
            hp_text.setFillColor(sf::Color::White);
            hp_text.setPosition({bar_x + bar_w * 0.5f - 25.f, bar_y + 2.f});
            
            window.draw(boss_bar_bg);
            window.draw(boss_bar_fill);
            window.draw(boss_label);
            window.draw(hp_text);
        }
    }

    // Pause button (center top)
    {
        const float button_w = 100.f;
        const float button_h = 40.f;
        const float button_x = (static_cast<float>(win_size.x) - button_w) * 0.5f;
        const float button_y = 10.f;

        pause_button_bounds_ = sf::FloatRect(sf::Vector2f(button_x, button_y), sf::Vector2f(button_w, button_h));

        sf::RectangleShape button_bg;
        button_bg.setPosition(sf::Vector2f(button_x, button_y));
        button_bg.setSize(sf::Vector2f(button_w, button_h));
        button_bg.setFillColor(sf::Color(100, 100, 120, 200));
        button_bg.setOutlineThickness(2.f);
        button_bg.setOutlineColor(sf::Color(200, 200, 200));

        sf::Text button_text(font_, "PAUSE", 16);
        button_text.setFillColor(sf::Color::White);
        const auto text_bounds = button_text.getLocalBounds();
        button_text.setPosition(sf::Vector2f(
            button_x + (button_w - text_bounds.size.x) * 0.5f,
            button_y + (button_h - text_bounds.size.y) * 0.5f - 4.f
        ));

        window.draw(button_bg);
        window.draw(button_text);
    }

    // Labels over players with HP bar
    reg.view<Position, Owner, Health>(
        [&](std::size_t eid, Position& pos, Owner& owner, Health& hp) {
            if (owner.player_id == 0) {
                return; // non-player
            }
            
            // Don't show health bar for spectators
            auto* spectator = reg.try_get<engine::game::components::Spectator>(
                rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(eid)});
            if (spectator && spectator->is_spectating) {
                return;
            }
            
            const float bar_width = 48.f;
            const float bar_height = 6.f;
            float ratio = (hp.max > 0) ? std::clamp(static_cast<float>(hp.current) / static_cast<float>(hp.max), 0.f, 1.f) : 0.f;

            sf::RectangleShape bg;
            bg.setSize({bar_width, bar_height});
            bg.setPosition({pos.x - bar_width * 0.5f, pos.y - 18.f});
            bg.setFillColor(sf::Color(60, 60, 60, 200));
            bg.setOutlineThickness(1.f);
            bg.setOutlineColor(sf::Color(20, 20, 20));

            sf::RectangleShape fg;
            fg.setSize({bar_width * ratio, bar_height});
            fg.setPosition(bg.getPosition());
            fg.setFillColor(sf::Color(0, 200, 80));

            // Get lives info if available
            std::ostringstream label;
            label << "P" << owner.player_id << " HP:" << hp.current << "/" << hp.max;

            auto* lives = reg.try_get<Lives>(
                rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(eid)});
            if (lives) {
                label << " | Lives: " << lives->remaining;
            }

            sf::Text txt(font_, label.str(), 12);
            txt.setFillColor(sf::Color::Cyan);
            txt.setPosition({pos.x - bar_width * 0.5f, pos.y - 32.f});

            window.draw(bg);
            window.draw(fg);
            window.draw(txt);
        });

    // Draw level transition overlay on top of everything
    draw_level_transition(window);
}

bool HUDSystem::is_pause_button_clicked(const sf::Vector2f& mouse_pos) {
    return pause_button_bounds_.contains(mouse_pos);
}

sf::FloatRect HUDSystem::get_pause_button_bounds() const {
    return pause_button_bounds_;
}

}  // namespace client::systems
