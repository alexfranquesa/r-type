/**
 * @file asteroid_spawn_system.hpp
 * @brief Asteroid obstacle spawning system for asteroid field levels
 * 
 * Spawns asteroids moving from right to left that players must avoid.
 * Asteroids require multiple hits to destroy (5 shots).
 * Only active in level 3+.
 */

#pragma once

#include <cstdint>
#include <random>

namespace rtype::ecs { class registry; }
namespace engine::game { class GameSettings; }

namespace rtype::game {

/**
 * @brief Manages asteroid obstacle spawning during asteroid field levels
 * 
 * The AsteroidSpawnSystem creates asteroid entities that move from
 * right to left. These obstacles deal damage to players on contact
 * and require 5 hits to destroy.
 */
class AsteroidSpawnSystem {
public:
    AsteroidSpawnSystem();
    ~AsteroidSpawnSystem() = default;

    /**
     * @brief Update spawn timer and create asteroids when ready
     * @param reg The ECS registry containing all entities and components
     * @param dt Delta time in seconds since last frame
     * @param current_level Current game level (1-indexed)
     */
    void run(rtype::ecs::registry& reg, float dt, std::uint16_t current_level);

    /**
     * @brief Set the spawn interval
     * @param interval Time in seconds between spawns
     */
    void setSpawnInterval(float interval);

    /**
     * @brief Set the maximum number of asteroids allowed
     * @param max Maximum active asteroids
     */
    void setMaxAsteroids(size_t max);

    /**
     * @brief Set the level where asteroids appear
     * @param level Level for asteroids (default: 3)
     */
    void setActiveLevel(std::uint16_t level);

private:
    /**
     * @brief Count the number of active asteroids
     * @param reg The ECS registry
     * @return Number of asteroid entities
     */
    size_t countAsteroids(rtype::ecs::registry& reg) const;

    /**
     * @brief Spawn a new asteroid entity
     * @param reg The ECS registry
     */
    void spawnAsteroid(rtype::ecs::registry& reg);

    // Time tracking
    float spawn_timer_{0.0f};           // Time since last spawn
    float spawn_interval_{2.5f};        // Seconds between spawns
    
    // Spawn limits
    size_t max_asteroids_{6};           // Maximum active asteroids
    std::uint16_t active_level_{3};     // Level with asteroids
    
    // Random number generation
    std::mt19937 rng_;
    std::uniform_real_distribution<float> y_distribution_;
    std::uniform_real_distribution<float> speed_distribution_;
    std::uniform_real_distribution<float> size_distribution_;
    
    // Asteroid configuration
    static constexpr float SPAWN_X = 1350.0f;       // Right of screen
    static constexpr float MIN_Y = 80.0f;           // Minimum spawn Y
    static constexpr float MAX_Y = 640.0f;          // Maximum spawn Y
    static constexpr float MIN_SPEED = -120.0f;     // Minimum horizontal speed (moving left)
    static constexpr float MAX_SPEED = -200.0f;     // Maximum horizontal speed (faster)
    static constexpr float BASE_WIDTH = 48.0f;      // Base collider width
    static constexpr float BASE_HEIGHT = 48.0f;     // Base collider height
    static constexpr int ASTEROID_HP = 50;          // 5 shots to destroy (player does 10 damage)
    static constexpr int ASTEROID_DAMAGE = 25;      // Damage on contact with player
};

}  // namespace rtype::game
