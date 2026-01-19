#pragma once

#include "engine/core/ISystem.hpp"
#include "engine/core/registry.hpp"

namespace rtype::game {

/**
 * @brief System that detects game over conditions (all players dead)
 * 
 * This system checks if all players have exhausted their lives.
 * When game over is detected, it sets a flag that can be queried.
 */
class GameOverSystem : public rtype::ecs::ISystem {
public:
    GameOverSystem() = default;
    ~GameOverSystem() override = default;

    /**
     * @brief Check for game over condition
     * @param reg The ECS registry
     * @param dt Delta time (unused but required by ISystem)
     */
    void run(rtype::ecs::registry& reg, float dt) override;

    /**
     * @brief Check if game over condition is met
     * @return true if all players are dead, false otherwise
     */
    [[nodiscard]] bool is_game_over() const { return game_over_; }

    /**
     * @brief Reset the game over state (for new game)
     */
    void reset() { game_over_ = false; }

private:
    bool game_over_{false};
};

}  // namespace rtype::game
