#pragma once

#include <cstdint>
#include <random>
#include "engine/core/registry.hpp"
#include "engine/game/game_settings.hpp"

namespace rtype::game {

/**
 * @brief System that spawns ice crab enemies in ice levels (Level 4+)
 * 
 * These are stronger enemies that require 6 hits to destroy (60 HP)
 * and fire ice projectiles.
 */
class IceEnemySpawnSystem {
public:
    IceEnemySpawnSystem();

    /**
     * @brief Run the spawn system
     * @param reg The ECS registry
     * @param dt Delta time since last frame
     * @param current_level Current game level
     * @param settings Game settings for multipliers
     */
    void run(rtype::ecs::registry& reg, float dt, std::uint16_t current_level, 
             const engine::game::GameSettings& settings);

    void setSpawnInterval(float interval);
    void setMaxEnemies(std::size_t max);

private:
    static constexpr float SPAWN_X = 1350.0f;
    static constexpr float MIN_Y = 80.0f;
    static constexpr float MAX_Y = 640.0f;
    static constexpr float ENEMY_SPEED = -80.0f;  // Slower than basic enemies
    static constexpr float ENEMY_WIDTH = 68.0f;   // Larger hitbox
    static constexpr float ENEMY_HEIGHT = 68.0f;
    static constexpr int ICE_CRAB_HEALTH = 30;    // 3 hits to kill
    static constexpr float SHOOT_COOLDOWN = 2.0f; // Shoots ice projectiles
    static constexpr std::uint16_t MIN_LEVEL = 4; // Only spawn in level 4+

    float spawn_timer_;
    float spawn_interval_;
    std::size_t max_enemies_;
    std::mt19937 rng_;
    std::uniform_real_distribution<float> y_distribution_;

    std::size_t countIceEnemies(rtype::ecs::registry& reg) const;
    void spawnIceEnemy(rtype::ecs::registry& reg, const engine::game::GameSettings& settings);
};

}  // namespace rtype::game
