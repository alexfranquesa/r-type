/*
** EPITECH PROJECT, 2025
** G-CPP-500-BAR-5-2-rtype-2
** File description:
** killer - tracks which player killed this entity
*/

#pragma once

#include <cstdint>

namespace engine::game::components {

// Tracks which player killed this entity (for score tracking)
struct Killer {
    std::uint16_t player_id{0};  // 0 = not killed by a player, >0 = player who killed it
};

}  // namespace engine::game::components
