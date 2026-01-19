/*
** EPITECH PROJECT, 2025
** G-CPP-500-BAR-5-2-rtype-2
** File description:
** spectator - marks a player as spectator (eliminated but still connected)
*/
#pragma once
#include <cstdint>

namespace engine::game::components {

/**
 * @brief Component marking a player as spectator
 * When a player runs out of lives, they become a spectator instead of disconnecting
 */
struct Spectator {
    bool is_spectating{true};
};

}  // namespace engine::game::components
