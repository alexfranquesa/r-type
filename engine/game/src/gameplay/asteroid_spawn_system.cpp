/**
 * @file asteroid_spawn_system.cpp
 * @brief Implementation of asteroid obstacle spawning system
 */

#include "engine/game/systems/gameplay/asteroid_spawn_system.hpp"
#include "engine/core/registry.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/velocity.hpp"
#include "engine/game/components/core/sprite.hpp"
#include "engine/game/components/gameplay/collider.hpp"
#include "engine/game/components/gameplay/faction.hpp"
#include "engine/game/components/gameplay/health.hpp"
#include "engine/game/components/visual/sprite_id.hpp"

#include <iostream>

namespace rtype::game {

AsteroidSpawnSystem::AsteroidSpawnSystem()
    : spawn_timer_(0.0f)
    , spawn_interval_(0.8f)       // Much faster spawning
    , max_asteroids_(15)          // Many more asteroids on screen
    , active_level_(3)
    , rng_(std::random_device{}())
    , y_distribution_(MIN_Y, MAX_Y)
    , speed_distribution_(MAX_SPEED, MIN_SPEED)  // Note: both negative, MAX is more negative
    , size_distribution_(0.8f, 1.5f)
{}

void AsteroidSpawnSystem::setSpawnInterval(float interval) {
    spawn_interval_ = interval;
}

void AsteroidSpawnSystem::setMaxAsteroids(size_t max) {
    max_asteroids_ = max;
}

void AsteroidSpawnSystem::setActiveLevel(std::uint16_t level) {
    active_level_ = level;
}

size_t AsteroidSpawnSystem::countAsteroids(rtype::ecs::registry& reg) const {
    size_t count = 0;
    
    // Count entities with sprite ID "asteroid" (to differentiate from other ENEMY entities)
    reg.view<engine::game::components::Sprite>([&](size_t /*entity_id*/, auto& sprite) {
        if (sprite.texture_id == "asteroid") {
            ++count;
        }
    });
    
    return count;
}

void AsteroidSpawnSystem::spawnAsteroid(rtype::ecs::registry& reg) {
    // Generate random Y position and size
    float spawn_y = y_distribution_(rng_);
    float speed = speed_distribution_(rng_);
    float size_mult = size_distribution_(rng_);
    
    // Create new asteroid entity
    auto asteroid = reg.spawn_entity();
    
    // Add Position component (spawn right of screen)
    reg.add_component(asteroid, 
        engine::game::components::Position{
            SPAWN_X,
            spawn_y
        });
    
    // Add Velocity component (move left)
    reg.add_component(asteroid,
        engine::game::components::Velocity{
            speed,      // Horizontal movement (negative = left)
            0.0f        // No vertical movement
        });
    
    // Scale collider with size multiplier
    float width = BASE_WIDTH * size_mult;
    float height = BASE_HEIGHT * size_mult;
    
    // Add Collider component
    reg.add_component(asteroid,
        engine::game::components::Collider{
            width,
            height,
            false
        });
    
    // Add FactionComponent (ENEMY - so they count towards level progression)
    reg.add_component(asteroid,
        engine::game::components::FactionComponent{
            engine::game::components::Faction::ENEMY
        });
    
    // Add Health component (5 shots to destroy, player does 10 damage)
    reg.add_component(asteroid,
        engine::game::components::Health{
            ASTEROID_HP,  // current (50 HP)
            ASTEROID_HP   // max
        });
    
    // Add Sprite component for rendering
    reg.add_component(asteroid,
        engine::game::components::Sprite{
            "asteroid",             // texture_id (client will resolve)
            {0, 0, 48, 48},         // texture_rect
            size_mult,              // scale_x (varies)
            size_mult,              // scale_y (varies)
            24.0f,                  // origin_x (center)
            24.0f,                  // origin_y (center)
            0.5f,                   // z_index
            true,                   // visible
            false,                  // flip_x
            false                   // flip_y
        });
        
    std::cout << "[AsteroidSpawnSystem] Spawned asteroid at Y=" << spawn_y 
              << " speed=" << speed << " size=" << size_mult << std::endl;
}

void AsteroidSpawnSystem::run(rtype::ecs::registry& reg, float dt, std::uint16_t current_level) {
    // Only spawn asteroids in level 3
    if (current_level != active_level_) {
        return;
    }
    
    // Update spawn timer
    spawn_timer_ += dt;
    
    // Check if it's time to spawn
    if (spawn_timer_ >= spawn_interval_) {
        spawn_timer_ = 0.0f;
        
        // Check if we can spawn more
        size_t current_count = countAsteroids(reg);
        if (current_count < max_asteroids_) {
            spawnAsteroid(reg);
        }
    }
}

}  // namespace rtype::game
