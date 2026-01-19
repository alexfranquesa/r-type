/**
 * @file ice_enemy_spawn_system.cpp
 * @brief Implementation of ice crab enemy spawning system for Level 4
 */

#include "engine/game/systems/gameplay/ice_enemy_spawn_system.hpp"
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
#include "engine/game/components/gameplay/movement_pattern.hpp"
#include "engine/game/components/visual/sprite_id.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace rtype::game {

IceEnemySpawnSystem::IceEnemySpawnSystem()
    : spawn_timer_(0.0f)
    , spawn_interval_(1.2f)   // Faster spawning (was 3.0)
    , max_enemies_(10)        // More ice crabs at once (was 4)
    , rng_(std::random_device{}())
    , y_distribution_(MIN_Y, MAX_Y)
{}

void IceEnemySpawnSystem::setSpawnInterval(float interval) {
    spawn_interval_ = interval;
}

void IceEnemySpawnSystem::setMaxEnemies(std::size_t max) {
    max_enemies_ = max;
}

std::size_t IceEnemySpawnSystem::countIceEnemies(rtype::ecs::registry& reg) const {
    std::size_t count = 0;
    
    reg.view<engine::game::components::FactionComponent, engine::game::components::EnemyTypeComponent>(
        [&](std::size_t /*entity_id*/, auto& faction, auto& enemy_type) {
            if (faction.faction_value == engine::game::components::Faction::ENEMY &&
                enemy_type.type == engine::game::components::EnemyType::IceCrab) {
                ++count;
            }
        });
    
    return count;
}

void IceEnemySpawnSystem::spawnIceEnemy(rtype::ecs::registry& reg, 
                                         const engine::game::GameSettings& settings) {
    float spawn_y = y_distribution_(rng_);
    
    auto enemy = reg.spawn_entity();
    
    // Position at right edge
    reg.add_component(enemy, engine::game::components::Position{SPAWN_X, spawn_y});
    
    // Slower movement than basic enemies
    const float speed = ENEMY_SPEED * settings.enemy_speed_multiplier();
    reg.add_component(enemy, engine::game::components::Velocity{speed, 0.0f});
    
    // Larger collider for the crab
    reg.add_component(enemy, engine::game::components::Collider{
        ENEMY_WIDTH, ENEMY_HEIGHT, false
    });
    
    // Mark as enemy faction
    reg.add_component(enemy, engine::game::components::FactionComponent{
        engine::game::components::Faction::ENEMY
    });
    
    // Mark as IceCrab type for sprite selection
    reg.add_component(enemy, engine::game::components::EnemyTypeComponent{
        engine::game::components::EnemyType::IceCrab
    });
    
    // 60 HP = 6 hits to kill
    const int hp = static_cast<int>(std::round(ICE_CRAB_HEALTH * settings.enemy_hp_multiplier()));
    const int clamped_hp = std::max(1, hp);
    reg.add_component(enemy, engine::game::components::Health{clamped_hp, clamped_hp});
    
    // Shooting cooldown
    std::uniform_real_distribution<float> cooldown_dist(SHOOT_COOLDOWN * 0.8f, SHOOT_COOLDOWN * 1.2f);
    reg.add_component(enemy, engine::game::components::ShootCooldown{
        SHOOT_COOLDOWN, cooldown_dist(rng_), true
    });
    
    // Track screen entry
    reg.add_component(enemy, engine::game::components::EnteredScreen{false});
    
    // Sprite (client will override with animated version)
    reg.add_component(enemy, engine::game::components::Sprite{
        "ice_crab",
        {0, 0, 34, 34},
        2.0f, 2.0f,
        17.0f, 17.0f,
        0.45f,
        true, false, false
    });
    
    // Movement pattern - slower, more menacing
    std::uniform_int_distribution<int> pattern_dist(0, 2);  // Fewer pattern types
    std::uniform_real_distribution<float> phase_dist(0.0f, 6.28318f);
    
    auto pattern_type = engine::game::components::MovementPatternType::SINE_WAVE;
    int pattern_roll = pattern_dist(rng_);
    if (pattern_roll == 1) {
        pattern_type = engine::game::components::MovementPatternType::LINEAR;
    } else if (pattern_roll == 2) {
        pattern_type = engine::game::components::MovementPatternType::DIVE;
    }
    
    float amplitude = 0.0f;
    float frequency = 0.0f;
    
    switch (pattern_type) {
        case engine::game::components::MovementPatternType::LINEAR:
            amplitude = 0.0f;
            frequency = 0.0f;
            break;
        case engine::game::components::MovementPatternType::SINE_WAVE:
            amplitude = 60.0f;
            frequency = 0.3f;
            break;
        case engine::game::components::MovementPatternType::DIVE:
            amplitude = 100.0f;
            frequency = 0.2f;
            break;
        default:
            break;
    }
    
    reg.add_component(enemy, engine::game::components::MovementPattern{
        pattern_type,
        amplitude,
        frequency,
        phase_dist(rng_),
        spawn_y,
        0.0f,
        0.0f,
        0.0f
    });
    
    std::cout << "[IceEnemySpawn] Spawned ice crab at Y=" << spawn_y 
              << " HP=" << clamped_hp << std::endl;
}

void IceEnemySpawnSystem::run(rtype::ecs::registry& reg, float dt, 
                               std::uint16_t current_level,
                               const engine::game::GameSettings& settings) {
    // Only spawn in level 4 (ice field)
    if (current_level != 4) {
        return;
    }
    
    spawn_timer_ += dt;
    
    // Scale spawn rate with level (more ice crabs in higher levels)
    float level_mult = 1.0f + (current_level - MIN_LEVEL) * 0.3f;
    float effective_interval = spawn_interval_ / level_mult;
    
    if (spawn_timer_ >= effective_interval) {
        const std::size_t current_count = countIceEnemies(reg);
        
        // Scale max enemies with level
        std::size_t effective_max = max_enemies_ + (current_level - MIN_LEVEL);
        
        if (current_count < effective_max) {
            spawnIceEnemy(reg, settings);
            spawn_timer_ = 0.0f;
        } else {
            spawn_timer_ = effective_interval;
        }
    }
}

}  // namespace rtype::game
