#include "engine/game/systems/gameplay/projectile_system.hpp"
#include "engine/core/registry.hpp"
#include "engine/game/components/gameplay/projectile.hpp"
#include "engine/game/components/gameplay/faction.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/velocity.hpp"
#include "engine/core/entity.hpp"
#include <vector>
#include <cmath>

namespace rtype::game {

namespace {
    // Find player position for homing projectiles
    bool findPlayerPosition(rtype::ecs::registry& reg, float& out_x, float& out_y) {
        bool found = false;
        reg.view<engine::game::components::FactionComponent, engine::game::components::Position>(
            [&](std::size_t /*entity_id*/, auto& faction, auto& pos) {
                if (faction.faction_value == engine::game::components::Faction::PLAYER) {
                    out_x = pos.x;
                    out_y = pos.y;
                    found = true;
                }
            });
        return found;
    }
}

void ProjectileSystem::run(rtype::ecs::registry& reg, float dt) {
    // Rastrear entidades para eliminar (aquellas que excedieron su tiempo de vida)
    std::vector<rtype::ecs::entity_t> entities_to_kill;
    
    // Screen boundaries (slightly larger to allow smooth exit)
    constexpr float SCREEN_LEFT = -100.0f;
    constexpr float SCREEN_RIGHT = 2020.0f;
    constexpr float SCREEN_TOP = -100.0f;
    constexpr float SCREEN_BOTTOM = 1180.0f;
    
    // Find player position once for all homing projectiles
    float player_x = 0.0f, player_y = 0.0f;
    bool player_found = findPlayerPosition(reg, player_x, player_y);
    
    // Actualizar todos los proyectiles con view API
    reg.view<engine::game::components::Projectile>([&](size_t entity_id, auto& projectile) {
        auto ent = rtype::ecs::entity_t{static_cast<rtype::ecs::entity_id_t>(entity_id)};
        
        // Check if projectile is out of bounds
        auto* pos = reg.try_get<engine::game::components::Position>(ent);
        if (pos) {
            if (pos->x < SCREEN_LEFT || pos->x > SCREEN_RIGHT ||
                pos->y < SCREEN_TOP || pos->y > SCREEN_BOTTOM) {
                entities_to_kill.push_back(ent);
                return;
            }
        }
        
        // Actualizar tiempo transcurrido
        projectile.elapsed_time += dt;
        
        // Verificar si el proyectil ha excedido su tiempo de vida
        if (projectile.elapsed_time >= projectile.lifetime) {
            entities_to_kill.push_back(ent);
            return;
        }
        
        // Handle homing projectiles (boss projectiles)
        if (projectile.is_homing && player_found) {
            auto* vel = reg.try_get<engine::game::components::Velocity>(ent);
            
            if (pos && vel) {
                // Calculate direction to player
                float dx = player_x - pos->x;
                float dy = player_y - pos->y;
                float dist = std::sqrt(dx * dx + dy * dy);
                
                if (dist > 10.0f) {  // Only track if not too close
                    // Normalize target direction
                    float target_dx = dx / dist;
                    float target_dy = dy / dist;
                    
                    // Get current velocity magnitude
                    float speed = std::sqrt(vel->vx * vel->vx + vel->vy * vel->vy);
                    if (speed < 1.0f) speed = 120.0f;  // Fallback speed
                    
                    // Normalize current direction
                    float curr_dx = vel->vx / speed;
                    float curr_dy = vel->vy / speed;
                    
                    // Smoothly rotate towards target (magnetic tracking)
                    float turn_rate = projectile.homing_strength * dt;
                    curr_dx += (target_dx - curr_dx) * turn_rate;
                    curr_dy += (target_dy - curr_dy) * turn_rate;
                    
                    // Renormalize and apply speed
                    float new_len = std::sqrt(curr_dx * curr_dx + curr_dy * curr_dy);
                    if (new_len > 0.0f) {
                        vel->vx = (curr_dx / new_len) * speed;
                        vel->vy = (curr_dy / new_len) * speed;
                    }
                }
            }
        }
    });
    
    // Eliminar todos los proyectiles expirados
    for (const auto& entity : entities_to_kill) {
        reg.kill_entity(entity);
    }
}

}  // namespace rtype::game
