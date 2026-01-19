/*
** EPITECH PROJECT, 2025
** G-CPP-500-BAR-5-2-rtype-2
** File description:
** faction
*/

#pragma once

namespace engine::game::components {

    enum class Faction {
        PLAYER,
        ENEMY,
        NEUTRAL,
        HAZARD,     // Environmental hazards (lava, spikes, etc.) - damages players only
        OBSTACLE    // Destructible obstacles (asteroids) - can be shot, damages on contact
    };

    struct FactionComponent {
        Faction faction_value{};
    };
}
