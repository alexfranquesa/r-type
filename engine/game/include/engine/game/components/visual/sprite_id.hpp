#pragma once

#include <cstdint>

namespace engine::game::components {

enum class SpriteId : std::uint16_t {
    Player = 0,
    EnemyBasic = 1,
    PlayerProjectile = 2,
    EnemyProjectile = 3,
    LavaDrop = 4,           // Falling lava hazard
    VolcanicEnemy = 5,      // Level 2 volcanic enemy type
    Asteroid = 6,           // Level 3 asteroid obstacle
    IceEnemy = 7,           // Level 4 ice crab enemy (animated)
    IceProjectile = 8,      // Level 4 ice projectile
    Boss = 9,               // Level 5 final boss (8-frame animation)
    BossProjectile = 10,    // Level 5 boss homing projectile (4-frame animation)
    UltimateProjectile = 11 // Ultimate projectile
};

}  // namespace engine::game::components
