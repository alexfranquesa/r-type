#pragma once

#include <cstdint>
#include <string>

namespace engine::game {

struct ScoreEntry {
    std::string player_name;       // Max 16 chars
    std::uint32_t score{0};
    std::uint16_t wave_reached{0};
    std::uint16_t level_reached{0};
    std::int64_t timestamp{0};     // Unix timestamp

    // Sorting priority: score > wave > level
    bool operator>(const ScoreEntry& other) const {
        if (score != other.score) return score > other.score;
        if (wave_reached != other.wave_reached) return wave_reached > other.wave_reached;
        return level_reached > other.level_reached;
    }

    bool operator<(const ScoreEntry& other) const {
        return other > *this;
    }
};

}  // namespace engine::game
