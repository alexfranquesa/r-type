/**
 * @file lava_drop_spawn_system.hpp
 * @brief Lava drop hazard spawning system for volcanic levels
 * 
 * Spawns falling lava drops that damage players on contact.
 * Only active in levels with volcanic theme (level 2+).
 */

#pragma once

#include <cstdint>
#include <random>

namespace rtype::ecs { class registry; }
namespace engine::game { class GameSettings; }

namespace rtype::game {

/**
 * @brief Manages lava drop hazard spawning during volcanic levels
 * 
 * The LavaDropSpawnSystem creates falling lava drop entities at
 * random horizontal positions. These hazards deal damage to players
 * on contact and must be avoided.
 */
class LavaDropSpawnSystem {
public:
    LavaDropSpawnSystem();
    ~LavaDropSpawnSystem() = default;

    /**
     * @brief Update spawn timer and create lava drops when ready
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
     * @brief Set the maximum number of lava drops allowed
     * @param max Maximum active lava drops
     */
    void setMaxDrops(size_t max);

    /**
     * @brief Set the first level where lava drops appear
     * @param level Minimum level for lava drops (default: 2)
     */
    void setMinLevel(std::uint16_t level);

private:
    /**
     * @brief Count the number of active lava drops
     * @param reg The ECS registry
     * @return Number of lava drop entities
     */
    size_t countLavaDrops(rtype::ecs::registry& reg) const;

    /**
     * @brief Spawn a new lava drop entity
     * @param reg The ECS registry
     */
    void spawnLavaDrop(rtype::ecs::registry& reg);

    // Time tracking
    float spawn_timer_{0.0f};           // Time since last spawn
    float spawn_interval_{1.5f};        // Seconds between spawns
    
    // Spawn limits
    size_t max_drops_{8};               // Maximum active lava drops
    std::uint16_t min_level_{2};        // First level with lava drops
    
    // Random number generation
    std::mt19937 rng_;
    std::uniform_real_distribution<float> x_distribution_;
    
    // Lava drop configuration
    static constexpr float SPAWN_Y = -20.0f;        // Above screen
    static constexpr float DROP_SPEED = 200.0f;     // Pixels/second (falling down)
    static constexpr float DROP_WIDTH = 16.0f;      // Collider width
    static constexpr float DROP_HEIGHT = 24.0f;     // Collider height
    static constexpr int DROP_DAMAGE = 15;          // Damage on contact
    static constexpr float MIN_X = 100.0f;          // Minimum spawn X (avoid left edge)
    static constexpr float MAX_X = 1180.0f;         // Maximum spawn X (avoid right edge)
};

}  // namespace rtype::game
