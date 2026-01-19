#include "engine/game/systems/gameplay/health_system.hpp"
#include "engine/game/game_settings.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/sprite.hpp"
#include "engine/game/components/gameplay/lives.hpp"
#include "engine/game/components/gameplay/faction.hpp"
#include "engine/game/components/gameplay/game_stats.hpp"
#include "engine/game/components/gameplay/spectator.hpp"
#include "engine/game/components/gameplay/collider.hpp"
#include "engine/game/components/gameplay/killer.hpp"
#include "engine/game/components/gameplay/ultimate_charge.hpp"
#include "engine/game/components/network/owner.hpp"
#include <iostream>

namespace engine::game::systems {

using engine::game::components::Health;
using engine::game::components::FactionComponent;
using engine::game::components::Faction;
using engine::game::components::GameStats;

void health_system(rtype::ecs::registry& registry, const engine::game::GameSettings& settings) {
    // Track how many enemies were killed this frame
    std::size_t enemies_killed_this_frame = 0;
    
    auto& healths = registry.get_components<Health>();
    for (std::size_t i = 0; i < healths.size(); ++i) {
        auto& opt = healths[i];
        if (!opt.has_value()) {
            continue;
        }
        auto& h = opt.value();
        if (h.current > h.max) {
            h.current = h.max;
        }
        if (h.current < 0) {
            h.current = 0;
        }
        if (h.current == 0) {
            const auto entity = registry.entity_from_index(static_cast<rtype::ecs::entity_id_t>(i));
            const bool is_player = (registry.try_get<engine::game::components::Owner>(entity) != nullptr);
            
            // Check if this is an enemy being killed
            auto* faction = registry.try_get<FactionComponent>(entity);
            const bool is_enemy = (faction && faction->faction_value == Faction::ENEMY);

            if (is_player) {
                if (settings.infinite_lives) {
                    h.current = h.max;
                    continue;
                }

                auto* lives = registry.try_get<engine::game::components::Lives>(entity);
                if (!lives) {
                    registry.emplace_component<engine::game::components::Lives>(
                        entity,
                        std::max(0, settings.player_lives),
                        std::max(0, settings.player_lives));
                    lives = registry.try_get<engine::game::components::Lives>(entity);
                }

                if (lives && lives->remaining > 0) {
                    lives->remaining -= 1;
                    if (lives->remaining > 0) {
                        h.current = h.max;
                        continue;
                    } else {
                        // Player ran out of lives - make them a spectator instead of killing them
                        registry.emplace_component<engine::game::components::Spectator>(entity);
                        // Remove their collider so they don't interfere with gameplay
                        registry.remove_component<engine::game::components::Collider>(entity);
                        // Hide the sprite immediately
                        auto* sprite = registry.try_get<engine::game::components::Sprite>(entity);
                        if (sprite) {
                            sprite->visible = false;
                        }
                        std::cout << "[health_system] Player entity " << entity << " became spectator" << std::endl;
                        continue; // Don't kill the entity, keep it alive as spectator
                    }
                }
            }
            
            // If it's an enemy dying from damage, count it as a kill
            if (is_enemy) {
                ++enemies_killed_this_frame;
                auto* killer = registry.try_get<engine::game::components::Killer>(entity);
                if (killer && killer->player_id > 0) {
                    registry.view<engine::game::components::Owner,
                                  engine::game::components::UltimateCharge>(
                        [&](std::size_t /*eid*/, auto& owner, auto& charge) {
                            if (owner.player_id != killer->player_id) {
                                return;
                            }
                            if (charge.ready) {
                                charge.kills_since_last_ulti = 3;
                                charge.ui_frame = 4;
                                return;
                            }
                            const int next_kills = std::min<int>(3, charge.kills_since_last_ulti + 1);
                            charge.kills_since_last_ulti = static_cast<std::uint8_t>(next_kills);
                            charge.ui_frame = static_cast<std::uint8_t>(next_kills + 1);
                            charge.ready = (next_kills >= 3);
                        });
                }
            }

            // Only kill non-player entities (enemies, projectiles, hazards)
            if (!is_player) {
                registry.kill_entity(entity);
            }
        }
    }
    
    // Update GameStats with enemy kills (find stats entity and update)
    if (enemies_killed_this_frame > 0) {
        registry.view<GameStats>([&](std::size_t /*eid*/, GameStats& stats) {
            // We mark that kills happened - the LevelManager will handle progression
            // We use a simple increment approach here
            stats.total_kills += static_cast<std::uint16_t>(enemies_killed_this_frame);
            stats.kills_this_level += static_cast<std::uint16_t>(enemies_killed_this_frame);
            
            std::cout << "[health_system] Enemy killed! Total kills: " << stats.total_kills 
                      << ", This level: " << stats.kills_this_level << "/" << stats.kills_to_next_level << std::endl;
        });
    }
}

}  // namespace engine::game::systems
