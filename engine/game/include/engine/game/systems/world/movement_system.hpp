#pragma once

#include "engine/core/ISystem.hpp"

namespace rtype::game {

/**
 * @brief Updates entity positions based on their velocities
 * 
 * The MovementSystem applies simple Euler integration to move entities
 * each frame. It only processes entities that have both Position and
 * Velocity components.
 */
class MovementSystem : public rtype::ecs::ISystem {
public:
    MovementSystem() = default;
    ~MovementSystem() override = default;

    /**
     * @brief Update positions based on velocities
     * @param reg The ECS registry containing all entities and components
     * @param dt Delta time in seconds since last frame
     */
    void run(rtype::ecs::registry& reg, float dt) override;
};

}  // namespace rtype::game

