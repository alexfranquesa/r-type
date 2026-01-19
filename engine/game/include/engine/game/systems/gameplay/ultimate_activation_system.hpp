#pragma once

namespace rtype::ecs {
class registry;
}

namespace rtype::game {

class UltimateActivationSystem {
public:
    void run(rtype::ecs::registry& reg);

private:
    static constexpr float kProjectileSpeed = 650.0f;
    static constexpr float kProjectileOffsetX = 30.0f;
    static constexpr float kProjectileLifetime = 4.0f;
    static constexpr float kColliderWidth = 34.0f;
    static constexpr float kColliderHeight = 34.0f;
    static constexpr int kUltimateDamage = 9999;
};

}  // namespace rtype::game
