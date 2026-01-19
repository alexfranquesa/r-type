/*
** EPITECH PROJECT, 2025
** G-CPP-500-BAR-5-2-rtype-2
** File description:
** ai_behavior
*/

#pragma once

namespace engine::game::components {

    enum class Pattern {
        STRAIGHT,
        AGGRESSIVE,
        SINEWAVE,
    };

    struct AIBehavior {
        Pattern behavior{};
        float param_a{};
        float param_b{};
    };
}
