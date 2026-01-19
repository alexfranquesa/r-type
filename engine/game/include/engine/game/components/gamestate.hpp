#pragma once

namespace engine::game::components {

    /**
     * GameState component to track the current state of the game.
     */
    enum class GameStateType {
        MENU,
        PLAYING,
        PAUSED,
        GAME_OVER,
        VICTORY
    };

    struct GameState {
        GameStateType state{GameStateType::MENU};
    };

}  // namespace engine::game::components
