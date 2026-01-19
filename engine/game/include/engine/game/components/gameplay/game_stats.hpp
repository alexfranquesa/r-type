// Singleton-style component to broadcast game-level stats (score, wave, level).
#pragma once

#include <cstdint>
#include <unordered_map>

namespace engine::game::components {

struct GameStats {
    std::uint32_t score{0};
    std::uint16_t wave{1};

    // Level progression system
    std::uint16_t current_level{1};          // Current level (1-indexed)
    std::uint16_t kills_this_level{0};       // Kills in current level
    std::uint16_t kills_to_next_level{15};   // Kills needed to advance
    std::uint16_t total_kills{0};            // Total kills across all levels

    // Per-player tracking
    std::unordered_map<std::uint16_t, std::uint32_t> player_kills;  // Per-player kill counts (player_id -> kills)

    // Helper to get remaining kills for next level
    std::uint16_t kills_remaining() const {
        return (kills_to_next_level > kills_this_level)
            ? (kills_to_next_level - kills_this_level) : 0;
    }
};

}  // namespace engine::game::components
