/*
** EPITECH PROJECT, 2025
** G-CPP-500-BAR-5-2-rtype-2
** File description:
** scene
*/

#pragma once

namespace engine::game::components {

    enum class SceneType {
        MAIN_MENU,
        IN_GAME,
        GAME_OVER
    };

    struct Scene {
        SceneType current;
    };

}