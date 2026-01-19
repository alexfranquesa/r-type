#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <string>

#include "engine/core/registry.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/gameplay/faction.hpp"
#include "engine/game/components/gameplay/health.hpp"
#include "engine/game/components/gameplay/lives.hpp"
#include "engine/game/components/gameplay/game_stats.hpp"
#include "engine/game/components/gameplay/enemy_type.hpp"
#include "engine/game/components/network/owner.hpp"
#include "engine/game/components/core/sprite.hpp"

namespace client::systems {

/**
 * @brief Level names for transition display
 */
inline const char* getLevelName(std::uint16_t level) {
    switch (level) {
        case 1: return "DEEP SPACE";
        case 2: return "VOLCANIC PLANET";
        case 3: return "ASTEROID FIELD";
        case 4: return "ICE WORLD";
        case 5: return "FINAL BOSS";
        default: return "UNKNOWN SECTOR";
    }
}

class HUDSystem {
public:
    explicit HUDSystem(const sf::Font& font);

    // Draw player/enemy counts (right) and perf info (left). Labels over players with id/HP.
    void draw(rtype::ecs::registry& reg, sf::RenderWindow& window, std::uint16_t local_player_id = 0);

    // Check if pause button was clicked at the given position
    bool is_pause_button_clicked(const sf::Vector2f& mouse_pos);

    // Get pause button bounds for hit testing
    sf::FloatRect get_pause_button_bounds() const;

    // Update FPS calculation with the current frame's delta time
    void update_fps(float delta_time);

    // Draw FPS overlay (top-left corner)
    void draw_fps(sf::RenderWindow& window, bool show_fps);
    
    // Update level transition state (call every frame with delta time)
    void update(float dt);
    
    // Reset level transition (call when starting a new game)
    void reset_transition();

private:
    // Draw the level transition overlay if active
    void draw_level_transition(sf::RenderWindow& window);

    const sf::Font& font_;
    sf::FloatRect pause_button_bounds_;
    float fps_accumulator_ = 0.0f;
    int fps_frame_count_ = 0;
    float fps_display_ = 0.0f;
    static constexpr float fps_update_interval_ = 0.5f; // Update FPS display twice per second
    
    // Level transition overlay state
    std::uint16_t last_level_ = 0;           // Last known level to detect changes
    std::uint16_t transition_level_ = 1;     // Level being displayed in transition
    float transition_timer_ = 0.0f;          // Current transition time
    bool transition_active_ = false;          // Is transition currently showing
    
    // Transition timing constants
    static constexpr float TRANSITION_FADE_IN = 0.5f;    // Fade in duration
    static constexpr float TRANSITION_HOLD = 2.0f;       // Hold duration 
    static constexpr float TRANSITION_FADE_OUT = 0.5f;   // Fade out duration
    static constexpr float TRANSITION_TOTAL = TRANSITION_FADE_IN + TRANSITION_HOLD + TRANSITION_FADE_OUT;
};

}  // namespace client::systems
