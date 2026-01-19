#pragma once

#include "engine/core/registry.hpp"
#include <random>

namespace engine::game {
    struct GameSettings;
}

namespace rtype::game {

/**
 * @brief System that handles enemy shooting behavior.
 * 
 * Enemies will periodically shoot projectiles towards the left side
 * of the screen (towards the players).
 */
class EnemyShootingSystem {
public:
    EnemyShootingSystem();

    /**
     * @brief Set the shooting interval for enemies.
     * @param interval Time in seconds between shots.
     */
    void setShootInterval(float interval);

    /**
     * @brief Set the projectile speed.
     * @param speed Speed in pixels per second (negative = left).
     */
    void setProjectileSpeed(float speed);

    /**
     * @brief Run the system, making enemies shoot periodically.
     * @param reg The ECS registry.
     * @param dt Delta time since last frame.
     */
    void run(rtype::ecs::registry& reg, float dt, const engine::game::GameSettings& settings);

private:
    void shootProjectile(rtype::ecs::registry& reg, size_t enemy_id, float x, float y, bool is_ice = false);

    float shoot_interval_;                          ///< Base interval between shots
    float projectile_speed_;                        ///< Speed of enemy projectiles
    std::mt19937 rng_;                             ///< Random number generator
    std::uniform_real_distribution<float> variance_dist_;  ///< Variance for shoot timing

    static constexpr float DEFAULT_SHOOT_INTERVAL = 2.5f;
    static constexpr float DEFAULT_PROJECTILE_SPEED = -300.0f;  // Negative = towards left
    static constexpr float PROJECTILE_DAMAGE = 10.0f;
    static constexpr float TIMING_VARIANCE = 0.5f;  // Random variance in shoot timing
    static constexpr float SCREEN_ENTRY_X = 1280.0f;  // Visible playfield width
    static constexpr float SHOOT_MIN_X = 50.0f;       // Avoid shooting when leaving the screen
};

}  // namespace rtype::game
