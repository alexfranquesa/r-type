#include "engine/game/systems/gameplay/enemy_shooting_system.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/velocity.hpp"
#include "engine/game/components/core/sprite.hpp"
#include "engine/game/components/gameplay/entered_screen.hpp"
#include "engine/game/components/gameplay/faction.hpp"
#include "engine/game/components/gameplay/shoot_cooldown.hpp"
#include "engine/game/components/gameplay/projectile.hpp"
#include "engine/game/components/gameplay/collider.hpp"
#include "engine/game/components/gameplay/enemy_type.hpp"
#include "engine/game/game_settings.hpp"
#include <algorithm>
#include <iostream>

namespace rtype::game {

EnemyShootingSystem::EnemyShootingSystem()
    : shoot_interval_(DEFAULT_SHOOT_INTERVAL)
    , projectile_speed_(DEFAULT_PROJECTILE_SPEED)
    , rng_(std::random_device{}())
    , variance_dist_(-TIMING_VARIANCE, TIMING_VARIANCE)
{}

void EnemyShootingSystem::setShootInterval(float interval) {
    shoot_interval_ = interval;
}

void EnemyShootingSystem::setProjectileSpeed(float speed) {
    projectile_speed_ = speed;
}

void EnemyShootingSystem::shootProjectile(rtype::ecs::registry& reg, size_t enemy_id, float x, float y, bool is_ice) {
    auto projectile = reg.spawn_entity();
    
    std::cout << "[EnemyShootingSystem] Enemy #" << enemy_id 
              << " shooting " << (is_ice ? "ICE " : "") << "projectile at (" << x << ", " << y << ")" << std::endl;

    // Position: spawn slightly to the left of the enemy
    // Ice crabs are larger and need different offset
    const float x_offset = is_ice ? -35.0f : -20.0f;
    reg.add_component(projectile,
        engine::game::components::Position{
            x + x_offset,  // Offset to spawn in front (left side) of enemy
            y
        });

    // Velocity: move towards left (negative X) - ice projectiles are slower
    const float speed = is_ice ? (projectile_speed_ * 0.7f) : projectile_speed_;
    reg.add_component(projectile,
        engine::game::components::Velocity{
            speed,
            0.0f
        });

    // Projectile metadata - ice projectiles do more damage
    reg.add_component(projectile,
        engine::game::components::Projectile{
            is_ice ? static_cast<int>(PROJECTILE_DAMAGE * 1.5) : static_cast<int>(PROJECTILE_DAMAGE),
            static_cast<int>(enemy_id),  // owner is the enemy
            5.0f,   // lifetime
            0.0f,   // elapsed
            is_ice  // mark as ice projectile
        });

    // Collider for collision detection
    reg.add_component(projectile,
        engine::game::components::Collider{
            12.0f,  // width
            8.0f,   // height
            false   // isTrigger
        });

    // Faction: ENEMY projectile
    reg.add_component(projectile,
        engine::game::components::FactionComponent{
            engine::game::components::Faction::ENEMY
        });

    // Sprite for rendering
    reg.add_component(projectile,
        engine::game::components::Sprite{
            "projectile",           // texture_id (reuse same texture)
            {0, 0, 16, 16},        // texture_rect
            1.0f,                   // scale_x
            1.0f,                   // scale_y
            0.0f,                   // origin_x
            0.0f,                   // origin_y
            1.0f,                   // z_index
            true,                   // visible
            true,                   // flip_x (flip horizontally since going left)
            false                   // flip_y
        });
}

void EnemyShootingSystem::run(rtype::ecs::registry& reg, float dt, const engine::game::GameSettings& settings) {
    const float base_interval = shoot_interval_ * settings.enemy_shoot_cooldown_multiplier();
    // Process all enemies
    reg.view<engine::game::components::Position,
             engine::game::components::FactionComponent,
             engine::game::components::EnteredScreen,
             engine::game::components::ShootCooldown>(
        [&](size_t entity_id, auto& position, auto& faction, auto& entered, auto& cooldown) {
            // Only process enemies
            if (faction.faction_value != engine::game::components::Faction::ENEMY) {
                return;
            }

            if (!entered.entered && position.x <= SCREEN_ENTRY_X) {
                entered.entered = true;
            }

            if (!cooldown.enabled) {
                return;
            }

            if (cooldown.cooldown_seconds <= 0.0f) {
                cooldown.cooldown_seconds = base_interval;
            }

            cooldown.remaining_seconds -= dt;
            if (cooldown.remaining_seconds < 0.0f) {
                cooldown.remaining_seconds = 0.0f;
            }

            // Check if ready to shoot
            if (cooldown.ready()) {
                if (entered.entered && position.x > SHOOT_MIN_X) {
                    // Check enemy type
                    auto ent = rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(entity_id)};
                    auto* enemy_type = reg.try_get<engine::game::components::EnemyTypeComponent>(ent);
                    
                    // Skip boss - boss has its own shooting system (BossBehaviorSystem)
                    if (enemy_type && enemy_type->type == engine::game::components::EnemyType::Boss) {
                        return;
                    }
                    
                    bool is_ice = (enemy_type && enemy_type->type == engine::game::components::EnemyType::IceCrab);
                    
                    shootProjectile(reg, entity_id, position.x, position.y, is_ice);
                }
                
                // Reset cooldown with some variance
                const float interval = (cooldown.cooldown_seconds > 0.0f) ? cooldown.cooldown_seconds : base_interval;
                cooldown.remaining_seconds = std::max(0.0f, interval + variance_dist_(rng_));
            }
        }
    );

    // Cooldowns are stored on entities, so they disappear when entities are destroyed.
}

}  // namespace rtype::game
