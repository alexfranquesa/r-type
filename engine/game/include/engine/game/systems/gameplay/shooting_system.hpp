#pragma once

#include <unordered_map>
#include <cstddef>

namespace rtype::ecs {
    class registry;
}

namespace engine::game {
    struct GameSettings;
}

namespace rtype::game {

/**
 * @brief Handles shooting logic for player entities
 * 
 * The ShootingSystem checks for shoot input from players and creates
 * projectile entities when the shoot button is pressed. It manages
 * shooting cooldown to prevent continuous shooting.
 */
class ShootingSystem {
public:
    ShootingSystem() = default;
    ~ShootingSystem() = default;

    /**
     * @brief Process shooting input and create projectiles
     * @param reg The ECS registry containing all entities and components
     * @param dt Delta time in seconds since last frame
     */
    void run(rtype::ecs::registry& reg, float dt, const engine::game::GameSettings& settings);

private:
    // Cooldown tracking per entity (entity_id -> time remaining)
    std::unordered_map<std::size_t, float> cooldowns_;
    
    // Time between shots in seconds
    static constexpr float SHOOT_COOLDOWN = 0.3f;
    
    // Projectile speed in pixels per second
    static constexpr float PROJECTILE_SPEED = 400.0f;
};

}  // namespace rtype::game
