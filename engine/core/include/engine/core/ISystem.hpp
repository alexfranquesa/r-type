#pragma once

namespace rtype::ecs {

// Forward declaration
class registry;

/**
 * @brief Abstract base class for all ECS systems
 * 
 * All game systems (MovementSystem, CollisionSystem, etc.) inherit from this
 * interface and implement the run() method to process entities each frame.
 */
class ISystem {
public:
    virtual ~ISystem() = default;

    /**
     * @brief Execute the system logic for one frame
     * @param reg The ECS registry containing all entities and components
     * @param dt Delta time in seconds since last frame
     */
    virtual void run(registry& reg, float dt) = 0;
};

}  // namespace rtype::ecs
