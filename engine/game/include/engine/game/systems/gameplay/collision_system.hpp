#pragma once

#include <cstddef>
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/gameplay/collider.hpp"
#include "engine/game/components/gameplay/projectile.hpp"
#include "engine/game/components/gameplay/faction.hpp"
#include "engine/game/components/gameplay/health.hpp"

namespace rtype::ecs {
    class registry;
}

namespace rtype::game {

/**
 * @brief Handles collision detection between game entities
 * 
 * The CollisionSystem detects collisions between projectiles and enemies
 * using AABB (Axis-Aligned Bounding Box) collision detection. When a
 * collision is detected, it destroys the projectile and applies damage
 * to the enemy entity.
 */
class CollisionSystem {
public:
    CollisionSystem() = default;
    ~CollisionSystem() = default;

    /**
     * @brief Check for collisions and handle them
     * @param reg The ECS registry containing all entities and components
     * @param dt Delta time in seconds since last frame (unused but kept for consistency)
     */
    void run(rtype::ecs::registry& reg, float dt);

private:
    /**
     * @brief Check if two axis-aligned bounding boxes intersect
     * @param x1 X position of first entity
     * @param y1 Y position of first entity
     * @param w1 Width of first entity
     * @param h1 Height of first entity
     * @param x2 X position of second entity
     * @param y2 Y position of second entity
     * @param w2 Width of second entity
     * @param h2 Height of second entity
     * @return true if the boxes intersect, false otherwise
     */
    bool checkAABBCollision(
        float x1, float y1, float w1, float h1,
        float x2, float y2, float w2, float h2
    ) const;

    /**
     * @brief Get actual collision box position with offset applied
     * @param pos Entity position
     * @param col Entity collider (with offset)
     * @param out_x Output X position of collision box
     * @param out_y Output Y position of collision box
     * @param out_w Output width of collision box
     * @param out_h Output height of collision box
     */
    void getCollisionBox(
        const engine::game::components::Position& pos,
        const engine::game::components::Collider& col,
        float& out_x, float& out_y, float& out_w, float& out_h
    ) const;

    bool projectiles_collide(const engine::game::components::FactionComponent* a_faction,
                             const engine::game::components::FactionComponent* b_faction) const;
};

}  // namespace rtype::game
