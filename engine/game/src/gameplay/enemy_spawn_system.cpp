#include "engine/game/systems/gameplay/enemy_spawn_system.hpp"
#include "engine/core/registry.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/velocity.hpp"
#include "engine/game/components/gameplay/collider.hpp"
#include "engine/game/components/gameplay/entered_screen.hpp"
#include "engine/game/components/gameplay/faction.hpp"
#include "engine/game/components/gameplay/health.hpp"
#include "engine/game/components/gameplay/movement_pattern.hpp"
#include "engine/game/components/gameplay/shoot_cooldown.hpp"
#include "engine/game/components/core/sprite.hpp"
#include "engine/game/game_settings.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace rtype::game {

EnemySpawnSystem::EnemySpawnSystem()
    : spawn_timer_(2.0f)  // Start above interval for immediate first spawn
    , spawn_interval_(1.0f)  // Faster spawning: 1 enemy per second
    , max_enemies_(50)
    , rng_(std::random_device{}())
    , y_distribution_(MIN_Y, MAX_Y)
    , total_spawned_(0)
{}

void EnemySpawnSystem::setSpawnInterval(float interval) {
    spawn_interval_ = interval;
}

void EnemySpawnSystem::setMaxEnemies(size_t max) {
    max_enemies_ = max;
}

size_t EnemySpawnSystem::countEnemies(rtype::ecs::registry& reg) const {
    size_t count = 0;
    
    // Use view API to count enemies (only iterates over entities with FactionComponent)
    reg.view<engine::game::components::FactionComponent>([&](size_t /*entity_id*/, auto& faction) {
        if (faction.faction_value == engine::game::components::Faction::ENEMY) {
            ++count;
        }
    });
    
    return count;
}

void EnemySpawnSystem::spawnEnemy(rtype::ecs::registry& reg, const engine::game::GameSettings& settings) {
    // Generate random Y position
    float spawn_y = y_distribution_(rng_);
    
    // Create new enemy entity
    auto enemy = reg.spawn_entity();
    
    // Add Position component (spawn at right edge)
    reg.add_component(enemy, 
        engine::game::components::Position{
            SPAWN_X,
            spawn_y
        });
    
    // Add Velocity component (move left)
    const float speed = ENEMY_SPEED * settings.enemy_speed_multiplier();
    reg.add_component(enemy,
        engine::game::components::Velocity{
            speed,
            0.0f
        });
    
    // Add Collider component
    reg.add_component(enemy,
        engine::game::components::Collider{
            ENEMY_WIDTH,
            ENEMY_HEIGHT,
            false
        });
    
    // Add FactionComponent (ENEMY)
    reg.add_component(enemy,
        engine::game::components::FactionComponent{
            engine::game::components::Faction::ENEMY
        });
    
    // Add Health component
    const int base_health = static_cast<int>(std::round(ENEMY_HEALTH * settings.enemy_hp_multiplier()));
    const int clamped_health = std::max(1, base_health);
    reg.add_component(enemy,
        engine::game::components::Health{
            clamped_health,
            clamped_health
        });

    const float base_cooldown = ENEMY_SHOOT_COOLDOWN * settings.enemy_shoot_cooldown_multiplier();
    std::uniform_real_distribution<float> cooldown_dist(base_cooldown * 0.5f,
                                                        base_cooldown * 1.5f);
    reg.add_component(enemy,
        engine::game::components::ShootCooldown{
            base_cooldown,
            cooldown_dist(rng_),
            true
        });

    // Track when the enemy has entered the visible screen.
    reg.add_component(enemy, engine::game::components::EnteredScreen{false});
    
    // Add Sprite component for rendering
    reg.add_component(enemy,
        engine::game::components::Sprite{
            "enemy_basic",              // texture_id
            {0, 0, 32, 32},            // texture_rect
            1.0f,                       // scale_x
            1.0f,                       // scale_y
            0.0f,                       // origin_x
            0.0f,                       // origin_y
            0.5f,                       // z_index
            true,                       // visible
            false,                      // flip_x
            false                       // flip_y
        });

    // Add random movement pattern
    std::uniform_int_distribution<int> pattern_dist(0, 4);
    std::uniform_real_distribution<float> phase_dist(0.0f, 6.28318f);

    auto pattern_type = static_cast<engine::game::components::MovementPatternType>(pattern_dist(rng_));
    const float max_vertical_room = std::max(20.0f, std::min(spawn_y - MIN_Y, MAX_Y - spawn_y));

    auto clamp_amplitude = [&](float min_val, float max_val) {
        std::uniform_real_distribution<float> dist(min_val, max_val);
        float raw = dist(rng_);
        return std::min(raw, max_vertical_room);
    };

    float amplitude = 0.0f;
    float frequency = 0.0f;

    switch (pattern_type) {
        case engine::game::components::MovementPatternType::LINEAR:
            amplitude = 0.0f;
            frequency = 0.0f;
            break;
        case engine::game::components::MovementPatternType::SINE_WAVE:
            amplitude = clamp_amplitude(50.0f, 110.0f);
            frequency = std::uniform_real_distribution<float>(0.35f, 0.8f)(rng_);
            break;
        case engine::game::components::MovementPatternType::ZIGZAG:
            amplitude = clamp_amplitude(70.0f, 130.0f);
            frequency = std::uniform_real_distribution<float>(0.5f, 1.0f)(rng_);
            break;
        case engine::game::components::MovementPatternType::DIVE:
            amplitude = clamp_amplitude(80.0f, 150.0f);
            frequency = std::uniform_real_distribution<float>(0.25f, 0.55f)(rng_);
            break;
        case engine::game::components::MovementPatternType::CIRCLE:
            amplitude = clamp_amplitude(45.0f, 90.0f);
            frequency = std::uniform_real_distribution<float>(0.35f, 0.7f)(rng_);
            break;
    }

    reg.add_component(enemy,
        engine::game::components::MovementPattern{
            pattern_type,
            amplitude,              // amplitude
            frequency,              // frequency
            phase_dist(rng_),       // initial phase
            spawn_y,                // base_y (center line)
            0.0f,                   // elapsed
            0.0f,                   // offset_x
            0.0f                    // offset_y
        });

    ++total_spawned_;
}

void EnemySpawnSystem::run(rtype::ecs::registry& reg, float dt, std::uint16_t current_level, const engine::game::GameSettings& settings) {
    // Don't spawn regular enemies in level 5 (boss level)
    if (current_level == 5) {
        return;
    }
    
    // Update spawn timer
    spawn_timer_ += dt;

    const std::size_t max_enemies = static_cast<std::size_t>(settings.get_enemies_per_wave());
    const float rate_multiplier = settings.enemy_spawn_rate * settings.enemy_spawn_rate_multiplier();
    const float effective_rate = (rate_multiplier > 0.0f) ? rate_multiplier : 1.0f;
    const float effective_interval = spawn_interval_ / effective_rate;

    // Check if it's time to spawn and we haven't reached the limit
    if (spawn_timer_ >= effective_interval) {
        const size_t current_enemies = countEnemies(reg);

        if (current_enemies < max_enemies) {
            spawnEnemy(reg, settings);
            spawn_timer_ = 0.0f;  // Reset timer
        } else {
            // Don't reset timer if at max capacity
            // This allows immediate spawn when an enemy is destroyed
            spawn_timer_ = effective_interval;
        }
    }
}

}  // namespace rtype::game
