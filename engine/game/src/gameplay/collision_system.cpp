#include "engine/game/systems/gameplay/collision_system.hpp"
#include "engine/core/registry.hpp"
#include "engine/core/entity.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/gameplay/collider.hpp"
#include "engine/game/components/gameplay/projectile.hpp"
#include "engine/game/components/gameplay/faction.hpp"
#include "engine/game/components/gameplay/health.hpp"
#include "engine/game/components/gameplay/killer.hpp"
#include "engine/game/components/gameplay/boss_phase.hpp"
#include "engine/game/components/gameplay/enemy_type.hpp"
#include "engine/game/components/gameplay/ultimate_projectile.hpp"
#include "engine/game/components/network/owner.hpp"
#include <vector>

namespace rtype::game {

bool CollisionSystem::checkAABBCollision(
    float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2
) const {
    // AABB collision detection
    return (x1 < x2 + w2 &&
            x1 + w1 > x2 &&
            y1 < y2 + h2 &&
            y1 + h1 > y2);
}

// Helper to get actual collision box position with offset applied
void CollisionSystem::getCollisionBox(
    const engine::game::components::Position& pos,
    const engine::game::components::Collider& col,
    float& out_x, float& out_y, float& out_w, float& out_h
) const {
    out_x = pos.x + col.offset_x;
    out_y = pos.y + col.offset_y;
    out_w = col.width;
    out_h = col.height;
}

bool CollisionSystem::projectiles_collide(const engine::game::components::FactionComponent* a_faction,
                                          const engine::game::components::FactionComponent* b_faction) const {
    if (!a_faction || !b_faction) {
        return false;
    }
    return a_faction->faction_value != b_faction->faction_value;
}

void CollisionSystem::run(rtype::ecs::registry& reg, float /*dt*/) {
    // Track entities to destroy (projectiles that hit)
    std::vector<rtype::ecs::entity_t> projectiles_to_destroy;
    std::vector<rtype::ecs::entity_t> enemies_to_destroy;
    
    // === PLAYER PROJECTILES VS ENEMIES ===
    // Check each projectile against each enemy using view API
    reg.view<engine::game::components::Position, engine::game::components::Collider, engine::game::components::Projectile>(
        [&](size_t proj_id, auto& proj_pos, auto& proj_col, auto& proj) {
            // Get projectile faction (if any)
            const rtype::ecs::entity_t proj_entity{static_cast<rtype::ecs::entity_id_t>(proj_id)};
            auto* proj_faction = reg.try_get<engine::game::components::FactionComponent>(proj_entity);
            const bool is_ultimate = (reg.try_get<engine::game::components::UltimateProjectile>(proj_entity) != nullptr);
            
            // If projectile is from ENEMY faction, skip this check (enemy projectiles don't hurt enemies)
            if (proj_faction && proj_faction->faction_value == engine::game::components::Faction::ENEMY) {
                return;
            }
            
            // Get projectile collision box with offset
            float proj_x, proj_y, proj_w, proj_h;
            getCollisionBox(proj_pos, proj_col, proj_x, proj_y, proj_w, proj_h);
            
            // Check against all enemies (nested view)
            reg.view<engine::game::components::Position, engine::game::components::Collider, engine::game::components::FactionComponent>(
                [&](size_t enemy_id, auto& enemy_pos, auto& enemy_col, auto& faction) {
                    // Solo verificar colisiones con enemigos
                    if (faction.faction_value != engine::game::components::Faction::ENEMY) {
                        return;
                    }
                    
                    // Get enemy collision box with offset
                    float enemy_x, enemy_y, enemy_w, enemy_h;
                    getCollisionBox(enemy_pos, enemy_col, enemy_x, enemy_y, enemy_w, enemy_h);
                    
                    // Check AABB collision
                    if (checkAABBCollision(
                        proj_x, proj_y, proj_w, proj_h,
                        enemy_x, enemy_y, enemy_w, enemy_h
                    )) {
                        // Collision detected!
                        
                        // Check if this is the boss and if it has i-frames active
                        const rtype::ecs::entity_t enemy_entity{static_cast<rtype::ecs::entity_id_t>(enemy_id)};
                        auto* enemy_type = reg.try_get<engine::game::components::EnemyTypeComponent>(enemy_entity);
                        auto* boss_phase = reg.try_get<engine::game::components::BossPhase>(enemy_entity);
                        
                        if (enemy_type && enemy_type->type == engine::game::components::EnemyType::Boss && 
                            boss_phase && boss_phase->is_invulnerable) {
                            // Boss is invulnerable, don't apply damage but still destroy projectile
                            projectiles_to_destroy.push_back(rtype::ecs::entity_t(proj_id));
                            return;
                        }
                        
                        // Apply damage to enemy if it has a health component
                        auto& healths = reg.get_components<engine::game::components::Health>();
                        if (enemy_id < healths.size() && healths[enemy_id].has_value()) {
                            auto& health = healths[enemy_id].value();
                            if (is_ultimate) {
                                health.current = 0;
                            } else {
                                health.current -= proj.damage;
                            }
                            
                            // If this is the boss, activate i-frames and damage flash
                            if (enemy_type && enemy_type->type == engine::game::components::EnemyType::Boss && boss_phase) {
                                boss_phase->is_invulnerable = true;
                                boss_phase->iframe_timer = 0.1f;  // 0.1s i-frames
                                boss_phase->flash_timer = 0.2f;   // 0.2s flash effect
                            }
                            
                            // Track which player killed this enemy
                            if (proj_faction && proj_faction->faction_value == engine::game::components::Faction::PLAYER) {
                                auto* proj_owner = reg.try_get<engine::game::components::Owner>(proj_entity);
                                const rtype::ecs::entity_t enemy_entity{static_cast<rtype::ecs::entity_id_t>(enemy_id)};
                                auto* enemy_killer = reg.try_get<engine::game::components::Killer>(enemy_entity);
                                
                                if (!enemy_killer) {
                                    reg.emplace_component<engine::game::components::Killer>(
                                        enemy_entity,
                                        proj_owner ? proj_owner->player_id : 0
                                    );
                                } else if (proj_owner) {
                                    enemy_killer->player_id = proj_owner->player_id;
                                }
                            }
                            
                            // Note: The health system will handle enemy destruction if current <= 0
                        }
                        
                        // Mark projectile for destruction
                        projectiles_to_destroy.push_back(rtype::ecs::entity_t(proj_id));
                    }
                }
            );
        }
    );

    // === ENEMY PROJECTILES VS PLAYERS ===
    reg.view<engine::game::components::Position, engine::game::components::Collider, engine::game::components::Projectile, engine::game::components::FactionComponent>(
        [&](size_t proj_id, auto& proj_pos, auto& proj_col, auto& proj, auto& proj_faction) {
            // Only process ENEMY projectiles
            if (proj_faction.faction_value != engine::game::components::Faction::ENEMY) {
                return;
            }
            
            // Get projectile collision box with offset
            float proj_x, proj_y, proj_w, proj_h;
            getCollisionBox(proj_pos, proj_col, proj_x, proj_y, proj_w, proj_h);
            
            // Check against all players
            reg.view<engine::game::components::Position, engine::game::components::Collider, engine::game::components::FactionComponent, engine::game::components::Health>(
                [&](size_t player_id, auto& player_pos, auto& player_col, auto& player_faction, auto& player_health) {
                    // Only check players
                    if (player_faction.faction_value != engine::game::components::Faction::PLAYER) {
                        return;
                    }
                    
                    // Get player collision box with offset
                    float player_x, player_y, player_w, player_h;
                    getCollisionBox(player_pos, player_col, player_x, player_y, player_w, player_h);
                    
                    // Check AABB collision
                    if (checkAABBCollision(
                        proj_x, proj_y, proj_w, proj_h,
                        player_x, player_y, player_w, player_h
                    )) {
                        // Collision detected! Apply damage to player
                        player_health.current -= proj.damage;
                        
                        // Mark projectile for destruction
                        projectiles_to_destroy.push_back(rtype::ecs::entity_t(proj_id));
                    }
                }
            );
        }
    );

    // === PROJECTILE VS PROJECTILE (player vs enemy) ===
    reg.view<engine::game::components::Position, engine::game::components::Collider, engine::game::components::Projectile, engine::game::components::FactionComponent>(
        [&](size_t a_id, auto& a_pos, auto& a_col, auto& /*a_proj*/, auto& a_faction) {
            // Get first projectile collision box with offset
            float a_x, a_y, a_w, a_h;
            getCollisionBox(a_pos, a_col, a_x, a_y, a_w, a_h);
            
            reg.view<engine::game::components::Position, engine::game::components::Collider, engine::game::components::Projectile, engine::game::components::FactionComponent>(
                [&](size_t b_id, auto& b_pos, auto& b_col, auto& /*b_proj*/, auto& b_faction) {
                    if (a_id >= b_id) {
                        return; // avoid double checks and self
                    }
                    const rtype::ecs::entity_t a_entity{static_cast<rtype::ecs::entity_id_t>(a_id)};
                    const rtype::ecs::entity_t b_entity{static_cast<rtype::ecs::entity_id_t>(b_id)};
                    if (reg.try_get<engine::game::components::UltimateProjectile>(a_entity) ||
                        reg.try_get<engine::game::components::UltimateProjectile>(b_entity)) {
                        return;
                    }
                    if (!projectiles_collide(&a_faction, &b_faction)) {
                        return;
                    }
                    
                    // Get second projectile collision box with offset
                    float b_x, b_y, b_w, b_h;
                    getCollisionBox(b_pos, b_col, b_x, b_y, b_w, b_h);
                    
                    if (checkAABBCollision(a_x, a_y, a_w, a_h, b_x, b_y, b_w, b_h)) {
                        projectiles_to_destroy.push_back(rtype::ecs::entity_t(a_id));
                        projectiles_to_destroy.push_back(rtype::ecs::entity_t(b_id));
                    }
                });
        });

    // Contact damage: player collides with enemy
    constexpr int contact_damage = 10;
    reg.view<engine::game::components::Position,
             engine::game::components::Collider,
             engine::game::components::FactionComponent,
             engine::game::components::Health>(
        [&](size_t player_id, auto& p_pos, auto& p_col, auto& p_faction, auto& p_health) {
            if (p_faction.faction_value != engine::game::components::Faction::PLAYER) {
                return;
            }
            
            // Get player collision box with offset
            float p_x, p_y, p_w, p_h;
            getCollisionBox(p_pos, p_col, p_x, p_y, p_w, p_h);
            
            reg.view<engine::game::components::Position,
                     engine::game::components::Collider,
                     engine::game::components::FactionComponent>(
                [&](size_t enemy_id, auto& e_pos, auto& e_col, auto& e_faction) {
                    if (e_faction.faction_value != engine::game::components::Faction::ENEMY) {
                        return;
                    }
                    
                    // Get enemy collision box with offset
                    float e_x, e_y, e_w, e_h;
                    getCollisionBox(e_pos, e_col, e_x, e_y, e_w, e_h);
                    
                    if (checkAABBCollision(p_x, p_y, p_w, p_h, e_x, e_y, e_w, e_h)) {
                        p_health.current -= contact_damage;
                        enemies_to_destroy.push_back(rtype::ecs::entity_t(enemy_id));
                    }
                });
        });
    
    // === HAZARD VS PLAYER (lava drops, etc.) ===
    std::vector<rtype::ecs::entity_t> hazards_to_destroy;
    constexpr int hazard_damage = 15;
    
    reg.view<engine::game::components::Position,
             engine::game::components::Collider,
             engine::game::components::FactionComponent,
             engine::game::components::Health>(
        [&](size_t player_id, auto& p_pos, auto& p_col, auto& p_faction, auto& p_health) {
            if (p_faction.faction_value != engine::game::components::Faction::PLAYER) {
                return;
            }
            
            // Get player collision box with offset
            float p_x, p_y, p_w, p_h;
            getCollisionBox(p_pos, p_col, p_x, p_y, p_w, p_h);
            
            reg.view<engine::game::components::Position,
                     engine::game::components::Collider,
                     engine::game::components::FactionComponent>(
                [&](size_t hazard_id, auto& h_pos, auto& h_col, auto& h_faction) {
                    if (h_faction.faction_value != engine::game::components::Faction::HAZARD) {
                        return;
                    }
                    
                    // Get hazard collision box with offset
                    float h_x, h_y, h_w, h_h;
                    getCollisionBox(h_pos, h_col, h_x, h_y, h_w, h_h);
                    
                    if (checkAABBCollision(p_x, p_y, p_w, p_h, h_x, h_y, h_w, h_h)) {
                        p_health.current -= hazard_damage;
                        hazards_to_destroy.push_back(rtype::ecs::entity_t(hazard_id));
                    }
                });
        });
    
    // === PLAYER PROJECTILE VS HAZARD (can shoot lava drops) ===
    reg.view<engine::game::components::Position, engine::game::components::Collider, engine::game::components::Projectile>(
        [&](size_t proj_id, auto& proj_pos, auto& proj_col, auto& /*proj*/) {
            const rtype::ecs::entity_t proj_entity{static_cast<rtype::ecs::entity_id_t>(proj_id)};
            auto* proj_faction = reg.try_get<engine::game::components::FactionComponent>(proj_entity);
            if (reg.try_get<engine::game::components::UltimateProjectile>(proj_entity)) {
                return;
            }
            
            // Only player projectiles can destroy hazards
            if (proj_faction && proj_faction->faction_value == engine::game::components::Faction::ENEMY) {
                return;
            }
            
            // Get projectile collision box with offset
            float proj_x, proj_y, proj_w, proj_h;
            getCollisionBox(proj_pos, proj_col, proj_x, proj_y, proj_w, proj_h);
            
            reg.view<engine::game::components::Position, engine::game::components::Collider, engine::game::components::FactionComponent>(
                [&](size_t hazard_id, auto& h_pos, auto& h_col, auto& h_faction) {
                    if (h_faction.faction_value != engine::game::components::Faction::HAZARD) {
                        return;
                    }
                    
                    // Get hazard collision box with offset
                    float h_x, h_y, h_w, h_h;
                    getCollisionBox(h_pos, h_col, h_x, h_y, h_w, h_h);
                    
                    if (checkAABBCollision(proj_x, proj_y, proj_w, proj_h, h_x, h_y, h_w, h_h)) {
                        projectiles_to_destroy.push_back(rtype::ecs::entity_t(proj_id));
                        hazards_to_destroy.push_back(rtype::ecs::entity_t(hazard_id));
                    }
                });
        });

    // Destroy all projectiles that hit enemies
    for (const auto& entity : projectiles_to_destroy) {
        reg.kill_entity(entity);
    }
    for (const auto& enemy : enemies_to_destroy) {
        reg.kill_entity(enemy);
    }
    for (const auto& hazard : hazards_to_destroy) {
        reg.kill_entity(hazard);
    }
}

}  // namespace rtype::game
