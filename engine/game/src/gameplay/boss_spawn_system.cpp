/**
 * @file boss_spawn_system.cpp
 * @brief Implementation of boss spawning and attack system for Level 5
 */

#include "engine/game/systems/gameplay/boss_spawn_system.hpp"
#include "engine/core/registry.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/velocity.hpp"
#include "engine/game/components/core/sprite.hpp"
#include "engine/game/components/gameplay/collider.hpp"
#include "engine/game/components/gameplay/faction.hpp"
#include "engine/game/components/gameplay/health.hpp"
#include "engine/game/components/gameplay/enemy_type.hpp"
#include "engine/game/components/gameplay/shoot_cooldown.hpp"
#include "engine/game/components/gameplay/entered_screen.hpp"
#include "engine/game/components/gameplay/projectile.hpp"
#include "engine/game/components/gameplay/boss_phase.hpp"
#include "engine/game/components/visual/sprite_id.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace rtype::game {

BossSpawnSystem::BossSpawnSystem()
    : boss_entity_(std::nullopt)
    , shoot_timer_(0.0f)
    , rng_(std::random_device{}())
{}

void BossSpawnSystem::reset() {
    boss_entity_ = std::nullopt;
    shoot_timer_ = 0.0f;
}

void BossSpawnSystem::run(rtype::ecs::registry& reg, float dt, std::uint16_t current_level,
                          const engine::game::GameSettings& settings) {
    // Only active in Level 5
    if (current_level != BOSS_LEVEL) {
        return;
    }

    // Spawn boss if not already spawned
    if (!boss_entity_.has_value()) {
        spawnBoss(reg, settings);
    } else {
        // Check if boss is still alive
        auto boss_ent = rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(*boss_entity_)};
        auto* health = reg.try_get<engine::game::components::Health>(boss_ent);
        if (!health || health->current <= 0) {
            boss_entity_ = std::nullopt;
            std::cout << "[BossSpawnSystem] Boss defeated!" << std::endl;
            return;
        }

        // Don't update boss shooting here - handled by BossBehaviorSystem
        // updateBossShooting(reg, dt);
    }
}

void BossSpawnSystem::spawnBoss(rtype::ecs::registry& reg,
                                 const engine::game::GameSettings& settings) {
    auto boss = reg.spawn_entity();
    boss_entity_ = static_cast<std::size_t>(boss);

    std::cout << "[BossSpawnSystem] Spawning final boss!" << std::endl;

    // Position in bottom-right corner
    reg.add_component(boss, engine::game::components::Position{BOSS_X, BOSS_Y});

    // Boss doesn't move
    reg.add_component(boss, engine::game::components::Velocity{0.0f, 0.0f});

    // Hitbox only covers the blue sphere on top (smaller, positioned at sphere)
    // The visual body is larger but only the sphere can be hit
    // Using offset to position hitbox at the blue sphere location (top of boss)
    reg.add_component(boss, engine::game::components::Collider{
        BOSS_HITBOX_WIDTH, BOSS_HITBOX_HEIGHT, false,
        BOSS_HITBOX_OFFSET_X, BOSS_HITBOX_OFFSET_Y
    });

    // Mark as enemy faction
    reg.add_component(boss, engine::game::components::FactionComponent{
        engine::game::components::Faction::ENEMY
    });

    // Mark as Boss type for sprite selection
    reg.add_component(boss, engine::game::components::EnemyTypeComponent{
        engine::game::components::EnemyType::Boss
    });

    // High HP
    const int hp = static_cast<int>(std::round(BOSS_HEALTH * settings.enemy_hp_multiplier()));
    const int clamped_hp = std::max(1, hp);
    reg.add_component(boss, engine::game::components::Health{clamped_hp, clamped_hp});

    // Shooting cooldown
    reg.add_component(boss, engine::game::components::ShootCooldown{
        SHOOT_COOLDOWN, SHOOT_COOLDOWN, true
    });

    // Track screen entry
    reg.add_component(boss, engine::game::components::EnteredScreen{true}); // Already on screen

    // Sprite: BOSS.png - Animated boss sprite (2048x140, 8 frames of 256x140 each)
    // The hitbox is offset to cover only the blue sphere on top
    reg.add_component(boss, engine::game::components::Sprite{
        "BOSS",
        {0, 0, 256, 140},  // First frame (256x140)
        1.5f, 1.5f,        // Scale
        128.0f, 70.0f,     // Center origin
        0.0f,
        true, false, false
    });
    
    // Add phase tracking component for behavior system
    reg.add_component(boss, engine::game::components::BossPhase{});

    shoot_timer_ = SHOOT_COOLDOWN; // Start ready to shoot
}

void BossSpawnSystem::findPlayerPosition(rtype::ecs::registry& reg, float& out_x, float& out_y) {
    out_x = 100.0f;  // Default position if no player found
    out_y = 360.0f;

    reg.view<engine::game::components::FactionComponent, engine::game::components::Position>(
        [&](std::size_t /*entity_id*/, auto& faction, auto& pos) {
            if (faction.faction_value == engine::game::components::Faction::PLAYER) {
                out_x = pos.x;
                out_y = pos.y;
            }
        });
}

void BossSpawnSystem::updateBossShooting(rtype::ecs::registry& reg, float dt) {
    if (!boss_entity_.has_value()) {
        return;
    }

    shoot_timer_ -= dt;
    if (shoot_timer_ <= 0.0f) {
        shoot_timer_ = SHOOT_COOLDOWN;

        // Get boss position
        auto boss_ent = rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(*boss_entity_)};
        auto* boss_pos = reg.try_get<engine::game::components::Position>(boss_ent);
        if (!boss_pos) {
            return;
        }

        // Find player position for initial direction
        float player_x, player_y;
        findPlayerPosition(reg, player_x, player_y);

        // Calculate direction to player
        float dx = player_x - boss_pos->x;
        float dy = player_y - boss_pos->y;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len > 0.0f) {
            dx /= len;
            dy /= len;
        } else {
            dx = -1.0f;
            dy = 0.0f;
        }

        // Spawn homing projectile
        auto projectile = reg.spawn_entity();

        // Spawn from boss center
        float spawn_x = boss_pos->x - 50.0f;  // Left side of boss
        float spawn_y = boss_pos->y;

        reg.add_component(projectile, engine::game::components::Position{spawn_x, spawn_y});
        reg.add_component(projectile, engine::game::components::Velocity{
            dx * PROJECTILE_SPEED, dy * PROJECTILE_SPEED
        });
        reg.add_component(projectile, engine::game::components::Collider{50.0f, 40.0f, false});
        reg.add_component(projectile, engine::game::components::FactionComponent{
            engine::game::components::Faction::ENEMY
        });
        reg.add_component(projectile, engine::game::components::Projectile{
            PROJECTILE_DAMAGE,              // damage
            static_cast<int>(*boss_entity_), // owner_id
            8.0f,                           // lifetime (longer for homing)
            0.0f,                           // elapsed_time
            false,                          // is_ice
            true,                           // is_boss
            true,                           // is_homing
            HOMING_STRENGTH                 // homing_strength
        });

        std::cout << "[BossSpawnSystem] Boss fired homing projectile!" << std::endl;
    }
}

}  // namespace rtype::game
