#pragma once

#include <cstdint>

namespace engine::game::components {

    /**
     * PlayerControl component to identify player-controlled entities.
     */
    struct PlayerControl {
        std::uint32_t player_id{0};  ///< Unique player identifier (0-3 for 4 players).
        bool can_shoot{true};
        bool can_move{true};
    };

}  // namespace engine::game::components
