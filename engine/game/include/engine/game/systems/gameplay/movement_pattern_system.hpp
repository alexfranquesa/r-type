#pragma once

#include "engine/core/registry.hpp"

namespace rtype::game {

/**
 * @brief System that updates entity positions based on their movement patterns.
 */
class MovementPatternSystem {
public:
    /**
     * @brief Update all entities with movement patterns.
     * @param reg The ECS registry.
     * @param dt Delta time since last frame.
     */
    void run(rtype::ecs::registry& reg, float dt);
};

}  // namespace rtype::game
