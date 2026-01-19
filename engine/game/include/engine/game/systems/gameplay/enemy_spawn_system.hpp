#pragma once

#include <cstdint>
#include <random>

namespace rtype::ecs {
    class registry;
}

namespace engine::game {
    struct GameSettings;
}

namespace rtype::game {

/**
 * @brief Manages dynamic enemy spawning during gameplay
 * 
 * The EnemySpawnSystem creates enemy entities at regular intervals
 * with configurable spawn positions, rates, and enemy properties.
 * It prevents spawn overflow by limiting the maximum number of
 * active enemies.
 */
class EnemySpawnSystem {
public:
    EnemySpawnSystem();
    ~EnemySpawnSystem() = default;

    /**
     * @brief Update spawn timer and create enemies when ready
     * @param reg The ECS registry containing all entities and components
     * @param dt Delta time in seconds since last frame
     * @param current_level Current game level (no spawns in level 5 - boss level)
     */
    void run(rtype::ecs::registry& reg, float dt, std::uint16_t current_level, const engine::game::GameSettings& settings);

    /**
     * @brief Set the spawn interval
     * @param interval Time in seconds between spawns
     */
    void setSpawnInterval(float interval);

    /**
     * @brief Set the maximum number of enemies allowed
     * @param max Maximum active enemies
     */
    void setMaxEnemies(size_t max);

    /**
     * @brief Total enemies spawned since start (for score/wave)
     */
    size_t totalSpawned() const { return total_spawned_; }

private:
    // Time tracking
    float spawn_timer_{0.0f};           // Time since last spawn
    float spawn_interval_{2.0f};        // Seconds between spawns
    
    // Spawn limits
    size_t max_enemies_{50};            // Maximum active enemies
    size_t total_spawned_{0};           // Total enemies ever spawned (for stats)
    
    // Random number generation
    std::mt19937 rng_;                  // Random number generator
    std::uniform_real_distribution<float> y_distribution_;  // Random Y position
    
    // Enemy configuration
    static constexpr float SPAWN_X = 1300.0f;      // Just outside right edge (screen width 1280)
    // Speed tuned: still slower than the original but visible on screen in reasonable time
    static constexpr float ENEMY_SPEED = -90.0f;    // Pixels/second (move left)
    // Base HP = 10 (1 player shot = 10 damage)
    // hp_multiplier = level number for "level N = N shots to kill"
    static constexpr int ENEMY_HEALTH = 10;         // Base hit points (1 shot at level 1)
    static constexpr float ENEMY_SHOOT_COOLDOWN = 2.5f;  // Seconds between shots
    static constexpr float ENEMY_WIDTH = 40.0f;     // Collider width (increased for better hit detection)
    static constexpr float ENEMY_HEIGHT = 40.0f;    // Collider height (increased for better hit detection)
    static constexpr float MIN_Y = 50.0f;           // Minimum spawn Y
    static constexpr float MAX_Y = 670.0f;          // Maximum spawn Y (720 - 50)

    /**
     * @brief Count the number of active enemies
     * @param reg The ECS registry
     * @return Number of enemies with Faction::ENEMY
     */
    size_t countEnemies(rtype::ecs::registry& reg) const;

    /**
     * @brief Spawn a new enemy entity
     * @param reg The ECS registry
     */
    void spawnEnemy(rtype::ecs::registry& reg, const engine::game::GameSettings& settings);
};

}  // namespace rtype::game
