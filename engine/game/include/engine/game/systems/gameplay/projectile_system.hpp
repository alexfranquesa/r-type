#pragma once

#include <cstddef>

namespace rtype::ecs {
    class registry;
}

namespace rtype::game {

/**
 * @brief Manages projectile entities lifecycle
 * 
 * The ProjectileSystem updates the elapsed time of all projectile entities
 * and automatically destroys those that have exceeded their lifetime.
 * This prevents projectiles from existing indefinitely and accumulating
 * in memory.
 */
class ProjectileSystem {
public:
    ProjectileSystem() = default;
    ~ProjectileSystem() = default;

    /**
     * @brief Update projectile lifetimes and destroy expired ones
     * @param reg The ECS registry containing all entities and components
     * @param dt Delta time in seconds since last frame
     */
    void run(rtype::ecs::registry& reg, float dt);
};

}  // namespace rtype::game
