/**
 * @file boss_behavior_system.cpp
 * @brief Boss AI behavior with phase-based attack patterns
 * 
 * Architecture: Pure gameplay logic in engine/game layer
 * - No SFML dependencies
 * - Uses only ECS components from engine/game/components
 * - Client handles rendering through Sprite component updates
 */

#include "engine/game/systems/gameplay/boss_behavior_system.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/velocity.hpp"
#include "engine/game/components/core/sprite.hpp"
#include "engine/game/components/gameplay/boss_phase.hpp"
#include "engine/game/components/gameplay/collider.hpp"
#include "engine/game/components/gameplay/enemy_type.hpp"
#include "engine/game/components/gameplay/faction.hpp"
#include "engine/game/components/gameplay/health.hpp"
#include "engine/game/components/gameplay/projectile.hpp"
#include "engine/game/components/gameplay/shoot_cooldown.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace rtype::game {

BossBehaviorSystem::BossBehaviorSystem()
    : rng_(std::random_device{}())
{}

void BossBehaviorSystem::reset() {
    // Nothing to reset
}

void BossBehaviorSystem::run(rtype::ecs::registry& reg, float dt, std::uint16_t current_level) {
    // Only run in level 5
    if (current_level != 5) {
        return;
    }
    
    // Find boss entity
    std::size_t boss_entity = 0;
    bool found_boss = false;
    
    reg.view<engine::game::components::EnemyTypeComponent, engine::game::components::Health>(
        [&](std::size_t eid, auto& enemy_type, auto& health) {
            if (enemy_type.type == engine::game::components::EnemyType::Boss && health.current > 0) {
                boss_entity = eid;
                found_boss = true;
            }
        });
    
    if (!found_boss) {
        return;
    }
    
    // Ensure boss has BossPhase component
    auto boss_ent = rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(boss_entity)};
    auto* phase = reg.try_get<engine::game::components::BossPhase>(boss_ent);
    if (!phase) {
        reg.add_component(boss_ent, engine::game::components::BossPhase{});
        phase = reg.try_get<engine::game::components::BossPhase>(boss_ent);
    }
    
    // Update subsystems
    updatePhaseTransitions(reg, boss_entity);
    updateIFramesAndFlash(reg, boss_entity, dt);
    updateMovement(reg, boss_entity, dt);
    
    if (phase) {
        updateShooting(reg, boss_entity, dt, phase->current_phase);
        updateMinionSpawning(reg, boss_entity, dt, phase->current_phase);
    }
}

void BossBehaviorSystem::updatePhaseTransitions(rtype::ecs::registry& reg, std::size_t boss_entity) {
    auto boss_ent = rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(boss_entity)};
    auto* health = reg.try_get<engine::game::components::Health>(boss_ent);
    auto* phase = reg.try_get<engine::game::components::BossPhase>(boss_ent);
    
    if (!health || !phase) {
        return;
    }
    
    float hp_percent = static_cast<float>(health->current) / static_cast<float>(health->max);
    std::uint8_t new_phase = 1;
    
    if (hp_percent <= 0.33f) {
        new_phase = 3;  // Phase 3: <33% HP
    } else if (hp_percent <= 0.66f) {
        new_phase = 2;  // Phase 2: 33-66% HP
    }
    
    if (new_phase != phase->current_phase) {
        std::cout << "[BossBehavior] Phase transition: " << static_cast<int>(phase->current_phase) 
                  << " -> " << static_cast<int>(new_phase) << " (HP: " << (hp_percent * 100.0f) << "%)" << std::endl;
        phase->current_phase = new_phase;
        phase->time_in_phase = 0.0f;
        phase->pattern_timer = 0.0f;  // Shoot immediately on phase change
    }
}

void BossBehaviorSystem::updateMovement(rtype::ecs::registry& reg, std::size_t boss_entity, float dt) {
    auto boss_ent = rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(boss_entity)};
    auto* pos = reg.try_get<engine::game::components::Position>(boss_ent);
    auto* vel = reg.try_get<engine::game::components::Velocity>(boss_ent);
    auto* phase = reg.try_get<engine::game::components::BossPhase>(boss_ent);
    
    if (!pos || !vel || !phase) {
        return;
    }
    
    // Phase 1: No movement
    if (phase->current_phase == 1) {
        vel->vx = 0.0f;
        vel->vy = 0.0f;
        return;
    }
    
    // Phase 2+: Vertical movement
    phase->movement_timer -= dt;
    if (phase->movement_timer <= 0.0f) {
        // Pick new target Y
        std::uniform_real_distribution<float> dist(MIN_Y, MAX_Y);
        phase->target_y = dist(rng_);
        phase->movement_timer = MOVEMENT_INTERVAL;
    }
    
    // Move toward target
    float dy = phase->target_y - pos->y;
    if (std::abs(dy) > 5.0f) {
        vel->vy = (dy > 0.0f ? 1.0f : -1.0f) * MOVEMENT_SPEED;
    } else {
        vel->vy = 0.0f;
    }
    
    // Clamp position
    if (pos->y < MIN_Y) {
        pos->y = MIN_Y;
        vel->vy = 0.0f;
    } else if (pos->y > MAX_Y) {
        pos->y = MAX_Y;
        vel->vy = 0.0f;
    }
}

void BossBehaviorSystem::updateShooting(rtype::ecs::registry& reg, std::size_t boss_entity, 
                                        float dt, std::uint8_t phase) {
    auto boss_ent = rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(boss_entity)};
    auto* boss_phase = reg.try_get<engine::game::components::BossPhase>(boss_ent);
    auto* pos = reg.try_get<engine::game::components::Position>(boss_ent);
    
    if (!boss_phase || !pos) {
        return;
    }
    
    // Determine shoot interval based on phase
    float shoot_interval = PHASE_1_SHOOT_INTERVAL;
    if (phase == 2) {
        shoot_interval = PHASE_2_SHOOT_INTERVAL;
    } else if (phase == 3) {
        shoot_interval = PHASE_3_SHOOT_INTERVAL;
    }
    
    boss_phase->pattern_timer -= dt;
    if (boss_phase->pattern_timer <= 0.0f) {
        boss_phase->pattern_timer = shoot_interval;
        
        float player_x, player_y;
        findPlayerPosition(reg, player_x, player_y);
        
        // All phases: Only fire homing magnetic projectiles toward player
        // Phase determines how many projectiles per volley
        int projectile_count = 1;
        if (phase == 2) {
            projectile_count = 2;  // Phase 2: 2 homing projectiles
        } else if (phase == 3) {
            projectile_count = 3;  // Phase 3: 3 homing projectiles
        }
        
        for (int i = 0; i < projectile_count; ++i) {
            // Slight spread for multiple projectiles
            float offset_y = (projectile_count > 1) ? (static_cast<float>(i) - static_cast<float>(projectile_count - 1) / 2.0f) * 30.0f : 0.0f;
            shootTargetedProjectile(reg, pos->x, pos->y + offset_y, player_x, player_y);
        }
    }
}

void BossBehaviorSystem::updateMinionSpawning(rtype::ecs::registry& reg, std::size_t boss_entity,
                                               float dt, std::uint8_t phase) {
    // DISABLED: No minions for now - boss solo fight
    // Phase 1: No minions
    // if (phase < 2) {
    //     return;
    // }
    // 
    // auto boss_ent = rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(boss_entity)};
    // auto* boss_phase = reg.try_get<engine::game::components::BossPhase>(boss_ent);
    // auto* pos = reg.try_get<engine::game::components::Position>(boss_ent);
    // 
    // if (!boss_phase || !pos) {
    //     return;
    // }
    // 
    // float minion_interval = (phase == 2) ? PHASE_2_MINION_INTERVAL : PHASE_3_MINION_INTERVAL;
    // 
    // boss_phase->minion_spawn_timer -= dt;
    // if (boss_phase->minion_spawn_timer <= 0.0f) {
    //     boss_phase->minion_spawn_timer = minion_interval;
    //     
    //     // Spawn 2-3 minions
    //     int minion_count = (phase == 2) ? 2 : 3;
    //     for (int i = 0; i < minion_count; ++i) {
    //         spawnMinion(reg, pos->x, pos->y);
    //     }
    //     
    //     std::cout << "[BossBehavior] Spawned " << minion_count << " minions" << std::endl;
    // }
}

void BossBehaviorSystem::updateIFramesAndFlash(rtype::ecs::registry& reg, std::size_t boss_entity, float dt) {
    auto boss_ent = rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(boss_entity)};
    auto* phase = reg.try_get<engine::game::components::BossPhase>(boss_ent);
    auto* sprite = reg.try_get<engine::game::components::Sprite>(boss_ent);
    
    if (!phase) {
        return;
    }
    
    // Update i-frames
    if (phase->is_invulnerable) {
        phase->iframe_timer -= dt;
        if (phase->iframe_timer <= 0.0f) {
            phase->is_invulnerable = false;
        }
    }
    
    // Update damage flash
    if (phase->flash_timer > 0.0f) {
        phase->flash_timer -= dt;
        
        // Toggle visibility for flash effect
        float normalized_time = phase->flash_timer / FLASH_DURATION;
        int flash_count = static_cast<int>(normalized_time / FLASH_INTERVAL);
        phase->flash_visible = (flash_count % 2 == 0);
        
        // Apply to sprite (client will read this for rendering)
        if (sprite) {
            sprite->visible = phase->flash_visible;
        }
    } else {
        phase->flash_visible = true;
        if (sprite) {
            sprite->visible = true;
        }
    }
}

void BossBehaviorSystem::shootCircularPattern(rtype::ecs::registry& reg, float x, float y, 
                                               int projectile_count) {
    const float angle_step = (2.0f * 3.14159f) / static_cast<float>(projectile_count);
    
    for (int i = 0; i < projectile_count; ++i) {
        float angle = static_cast<float>(i) * angle_step;
        float dx = std::cos(angle);
        float dy = std::sin(angle);
        
        auto projectile = reg.spawn_entity();
        float spawn_x = x - 50.0f;  // Left side of boss
        
        reg.add_component(projectile, engine::game::components::Position{spawn_x, y});
        reg.add_component(projectile, engine::game::components::Velocity{dx * 120.0f, dy * 120.0f});
        reg.add_component(projectile, engine::game::components::Collider{24.0f, 24.0f, false});
        reg.add_component(projectile, engine::game::components::FactionComponent{
            engine::game::components::Faction::ENEMY
        });
        reg.add_component(projectile, engine::game::components::Projectile{
            30,      // damage
            -1,      // owner_id
            8.0f,    // lifetime (longer for homing)
            0.0f,    // elapsed_time
            false,   // is_ice
            true,    // is_boss
            true,    // is_homing (magnetic tracking)
            2.0f     // homing_strength (strong tracking)
        });
        reg.add_component(projectile, engine::game::components::Sprite{
            "enemiebullet",
            {0, 0, 32, 32},
            1.0f, 1.0f,
            16.0f, 16.0f,
            0.0f,
            true, false, false
        });
    }
}

void BossBehaviorSystem::shootFanPattern(rtype::ecs::registry& reg, float x, float y,
                                         float player_x, float player_y) {
    // Calculate base direction to player
    float dx = player_x - x;
    float dy = player_y - y;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len > 0.0f) {
        dx /= len;
        dy /= len;
    }
    
    float base_angle = std::atan2(dy, dx);
    const float spread = 0.4f;  // Radians spread
    
    // Shoot 3 projectiles in a fan
    for (int i = -1; i <= 1; ++i) {
        float angle = base_angle + (static_cast<float>(i) * spread);
        float vx = std::cos(angle) * 150.0f;
        float vy = std::sin(angle) * 150.0f;
        
        auto projectile = reg.spawn_entity();
        float spawn_x = x - 50.0f;
        
        reg.add_component(projectile, engine::game::components::Position{spawn_x, y});
        reg.add_component(projectile, engine::game::components::Velocity{vx, vy});
        reg.add_component(projectile, engine::game::components::Collider{24.0f, 24.0f, false});
        reg.add_component(projectile, engine::game::components::FactionComponent{
            engine::game::components::Faction::ENEMY
        });
        reg.add_component(projectile, engine::game::components::Projectile{
            35,      // damage
            -1,      // owner_id
            8.0f,    // lifetime (longer for homing)
            0.0f,    // elapsed_time
            false,   // is_ice
            true,    // is_boss
            true,    // is_homing (magnetic tracking)
            2.5f     // homing_strength (strong tracking)
        });
        reg.add_component(projectile, engine::game::components::Sprite{
            "enemiebullet",
            {0, 0, 32, 32},
            1.2f, 1.2f,
            16.0f, 16.0f,
            0.0f,
            true, false, false
        });
    }
}

void BossBehaviorSystem::shootTargetedProjectile(rtype::ecs::registry& reg, float x, float y,
                                                  float player_x, float player_y) {
    float dx = player_x - x;
    float dy = player_y - y;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len > 0.0f) {
        dx /= len;
        dy /= len;
    }
    
    auto projectile = reg.spawn_entity();
    float spawn_x = x - 50.0f;
    
    reg.add_component(projectile, engine::game::components::Position{spawn_x, y});
    reg.add_component(projectile, engine::game::components::Velocity{dx * 100.0f, dy * 100.0f});
    reg.add_component(projectile, engine::game::components::Collider{35.0f, 35.0f, false});  // Match 40x40 sprite
    reg.add_component(projectile, engine::game::components::FactionComponent{
        engine::game::components::Faction::ENEMY
    });
    reg.add_component(projectile, engine::game::components::Projectile{
        40,      // damage (high damage)
        -1,      // owner_id
        12.0f,   // lifetime (extra long for aggressive tracking)
        0.0f,    // elapsed_time
        false,   // is_ice
        true,    // is_boss
        true,    // is_homing (magnetic tracking)
        3.0f     // homing_strength (very strong tracking)
    });
    // Use FinalProjectile sprite for homing projectile (512x512 per frame, scaled down)
    reg.add_component(projectile, engine::game::components::Sprite{
        "FinalProjectile",
        {0, 0, 512, 512},  // First frame
        0.08f, 0.08f,      // Scale down (512px -> ~40px)
        256.0f, 256.0f,    // Center origin
        0.0f,
        true, false, false
    });
}

void BossBehaviorSystem::spawnMinion(rtype::ecs::registry& reg, float boss_x, float boss_y) {
    // Spawn basic enemy as minion
    auto minion = reg.spawn_entity();
    
    // Spawn near boss with random offset
    std::uniform_real_distribution<float> y_offset(-100.0f, 100.0f);
    float spawn_y = boss_y + y_offset(rng_);
    spawn_y = std::clamp(spawn_y, MIN_Y, MAX_Y);
    
    reg.add_component(minion, engine::game::components::Position{boss_x - 150.0f, spawn_y});
    reg.add_component(minion, engine::game::components::Velocity{-100.0f, 0.0f});  // Move left
    reg.add_component(minion, engine::game::components::Collider{32.0f, 32.0f, false});
    reg.add_component(minion, engine::game::components::FactionComponent{
        engine::game::components::Faction::ENEMY
    });
    reg.add_component(minion, engine::game::components::EnemyTypeComponent{
        engine::game::components::EnemyType::Basic
    });
    reg.add_component(minion, engine::game::components::Health{20, 20});  // Low HP minions
    reg.add_component(minion, engine::game::components::ShootCooldown{2.0f, 2.0f, true});
    reg.add_component(minion, engine::game::components::Sprite{
        "enemy",
        {0, 0, 33, 36},
        1.0f, 1.0f,
        16.5f, 18.0f,
        0.0f,
        true, false, false
    });
}

void BossBehaviorSystem::findPlayerPosition(rtype::ecs::registry& reg, float& out_x, float& out_y) {
    out_x = 100.0f;
    out_y = 360.0f;
    
    reg.view<engine::game::components::FactionComponent, engine::game::components::Position>(
        [&](std::size_t /*eid*/, auto& faction, auto& pos) {
            if (faction.faction_value == engine::game::components::Faction::PLAYER) {
                out_x = pos.x;
                out_y = pos.y;
            }
        });
}

}  // namespace rtype::game
