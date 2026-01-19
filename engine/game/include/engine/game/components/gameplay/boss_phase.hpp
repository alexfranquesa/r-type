#pragma once

#include <cstdint>

namespace engine::game::components {

/**
 * @brief Boss phase tracking component
 * 
 * Tracks which phase the boss is in based on HP percentage.
 * Phase 1: 100-66% HP - Basic circular pattern
 * Phase 2: 66-33% HP - Adds minion spawns + aggressive movement  
 * Phase 3: <33% HP - Bullet hell + frequent minions
 */
struct BossPhase {
    std::uint8_t current_phase{1};     // 1, 2, or 3
    float time_in_phase{0.0f};         // Time spent in current phase
    float minion_spawn_timer{0.0f};    // Countdown for next minion spawn
    float pattern_timer{1.0f};         // Countdown for next attack pattern (start at 1.0s to shoot quickly)
    float movement_timer{4.0f};        // Countdown for next movement change
    
    // Movement state
    float target_y{360.0f};            // Target Y position for movement
    bool moving_up{false};             // Current movement direction
    
    // I-frames (invulnerability frames)
    float iframe_timer{0.0f};          // Remaining i-frame time
    bool is_invulnerable{false};       // Currently invulnerable?
    
    // Visual feedback
    bool flash_visible{true};          // For damage flash effect
    float flash_timer{0.0f};           // Flash animation timer
};

}  // namespace engine::game::components
