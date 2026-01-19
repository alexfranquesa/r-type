/*
** EPITECH PROJECT, 2025
** G-CPP-500-BAR-5-2-rtype-2
** File description:
** network_owned
*/

#pragma once
#include <cstdint>

namespace engine::game::components {

    struct NetworkOwnedComponent {
        std::uint32_t client_id{0}; // ID of the player who owns this entity
    };

}