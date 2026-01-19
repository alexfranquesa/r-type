/**
 * @file lava_drop_spawn_system.cpp
 * @brief Implementation of lava drop hazard spawning system
 */

#include "engine/game/systems/gameplay/lava_drop_spawn_system.hpp"
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

LavaDropSpawnSystem::LavaDropSpawnSystem()
    : spawn_timer_(0.0f)
    , spawn_interval_(1.5f)
    , max_drops_(8)
    , min_level_(2)
    , rng_(std::random_device{}())
    , x_distribution_(MIN_X, MAX_X)
{}

void LavaDropSpawnSystem::setSpawnInterval(float interval) {
    spawn_interval_ = interval;
}

void LavaDropSpawnSystem::setMaxDrops(size_t max) {
    max_drops_ = max;
}

void LavaDropSpawnSystem::setMinLevel(std::uint16_t level) {
    min_level_ = level;
}

size_t LavaDropSpawnSystem::countLavaDrops(rtype::ecs::registry& reg) const {
    size_t count = 0;
    
    // Count entities with HAZARD faction
    reg.view<engine::game::components::FactionComponent>([&](size_t /*entity_id*/, auto& faction) {
        if (faction.faction_value == engine::game::components::Faction::HAZARD) {
            ++count;
        }
    });
    
    return count;
}

void LavaDropSpawnSystem::spawnLavaDrop(rtype::ecs::registry& reg) {
    // Generate random X position
    float spawn_x = x_distribution_(rng_);
    
    // Create new lava drop entity
    auto drop = reg.spawn_entity();
    
    // Add Position component (spawn above screen)
    reg.add_component(drop, 
        engine::game::components::Position{
            spawn_x,
            SPAWN_Y
        });
    
    // Add Velocity component (fall down)
    reg.add_component(drop,
        engine::game::components::Velocity{
            0.0f,           // No horizontal movement
            DROP_SPEED      // Fall downward
        });
    
    // Add Collider component
    reg.add_component(drop,
        engine::game::components::Collider{
            DROP_WIDTH,
            DROP_HEIGHT,
            false
        });
    
    // Add FactionComponent (HAZARD - damages players)
    reg.add_component(drop,
        engine::game::components::FactionComponent{
            engine::game::components::Faction::HAZARD
        });
    
    // Add Health component (so it can be destroyed on impact or by projectiles)
    // Low health so player shots can destroy them
    reg.add_component(drop,
        engine::game::components::Health{
            1,  // current
            1   // max - one shot destroys it
        });
    
    // Add Sprite component for rendering (will be interpreted by client)
    reg.add_component(drop,
        engine::game::components::Sprite{
            "lava_drop",            // texture_id (client will resolve)
            {0, 0, 16, 24},         // texture_rect
            1.5f,                   // scale_x
            1.5f,                   // scale_y
            8.0f,                   // origin_x (center)
            12.0f,                  // origin_y (center)
            0.7f,                   // z_index (above background, below UI)
            true,                   // visible
            false,                  // flip_x
            false                   // flip_y
        });
}

void LavaDropSpawnSystem::run(rtype::ecs::registry& reg, float dt, std::uint16_t current_level) {
    // Only spawn lava drops in volcanic levels (level 2 only, not in asteroid field level 3+)
    if (current_level != min_level_) {
        return;
    }
    
    // Update spawn timer
    spawn_timer_ += dt;
    
    // Scale spawn rate with level (more drops in higher levels)
    float level_multiplier = 1.0f + (current_level - min_level_) * 0.2f;
    float effective_interval = spawn_interval_ / level_multiplier;
    
    // Check if it's time to spawn
    if (spawn_timer_ >= effective_interval) {
        const size_t current_drops = countLavaDrops(reg);
        
        // Scale max drops with level
        size_t effective_max = max_drops_ + (current_level - min_level_) * 2;
        
        if (current_drops < effective_max) {
            spawnLavaDrop(reg);
            spawn_timer_ = 0.0f;
        } else {
            // At max capacity, reset timer to spawn immediately when one is destroyed
            spawn_timer_ = effective_interval;
        }
    }
    
    // Clean up lava drops that went off screen (bottom)
    std::vector<rtype::ecs::entity_t> to_remove;
    reg.view<engine::game::components::Position, engine::game::components::FactionComponent>(
        [&](size_t entity_id, auto& pos, auto& faction) {
            if (faction.faction_value == engine::game::components::Faction::HAZARD) {
                if (pos.y > 800.0f) {  // Below screen
                    to_remove.push_back(rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(entity_id)});
                }
            }
        });
    
    for (auto entity : to_remove) {
        reg.kill_entity(entity);
    }
}

}  // namespace rtype::game
