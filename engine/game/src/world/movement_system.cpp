#include "engine/game/systems/world/movement_system.hpp"
#include "engine/core/registry.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/velocity.hpp"
#include "engine/game/components/gameplay/input_state.hpp"
#include "engine/game/components/gameplay/faction.hpp"
#include "engine/game/components/gameplay/projectile.hpp"
#include "engine/game/components/network/owner.hpp"

namespace rtype::game {

// Player movement speed in pixels per second
constexpr float PLAYER_SPEED = 200.0f;

// Screen boundaries (based on client window size: 1280x720)
constexpr float SCREEN_WIDTH = 1280.0f;
constexpr float SCREEN_HEIGHT = 720.0f;
constexpr float PLAYER_MARGIN = 16.0f;  // Small margin from screen edges

void MovementSystem::run(rtype::ecs::registry& reg, float dt) {
    // First pass: Update velocities based on input for player-controlled entities
    // Only iterates over entities that have ALL three components: Velocity, InputState, FactionComponent
    reg.view<engine::game::components::Velocity, engine::game::components::InputState, engine::game::components::FactionComponent>(
        [&](rtype::ecs::entity_t entity,
            engine::game::components::Velocity& vel,
            engine::game::components::InputState& input,
            engine::game::components::FactionComponent& faction) {
            
            // Solo procesar jugadores
            if (faction.faction_value != engine::game::components::Faction::PLAYER) {
                return;
            }
            
            // Reset velocity
            vel.vx = 0.0f;
            vel.vy = 0.0f;

            // Set velocity based on input
            if (input.up) {
                vel.vy -= PLAYER_SPEED;
            }
            if (input.down) {
                vel.vy += PLAYER_SPEED;
            }
            if (input.left) {
                vel.vx -= PLAYER_SPEED;
            }
            if (input.right) {
                vel.vx += PLAYER_SPEED;
            }

            // Normalize diagonal movement for consistent speed
            if ((input.up || input.down) && (input.left || input.right)) {
                float factor = 0.707f; // 1/sqrt(2) for normalized diagonal
                vel.vx *= factor;
                vel.vy *= factor;
            }
        });

    // Second pass: Apply physics integration to all entities with Position + Velocity
    // Only iterates over entities that have BOTH Position and Velocity components
    reg.view<engine::game::components::Position, engine::game::components::Velocity>(
        [&](rtype::ecs::entity_t entity, 
            engine::game::components::Position& pos, 
            engine::game::components::Velocity& vel) {
            
            // Apply Euler integration: position += velocity * dt
            pos.x += vel.vx * dt;
            pos.y += vel.vy * dt;
        });

    // Third pass: Clamp player positions to screen boundaries
    // NOTE: Projectiles are intentionally excluded so bullets can leave the screen
    reg.view<engine::game::components::Position, engine::game::components::FactionComponent>(
        [&](rtype::ecs::entity_t entity,
            engine::game::components::Position& pos,
            engine::game::components::FactionComponent& faction) {
            
            // Only apply boundary limits to players (not projectiles)
            if (faction.faction_value != engine::game::components::Faction::PLAYER) {
                return;
            }

            // Skip if this entity is a projectile (bullets can leave the screen)
            if (reg.try_get<engine::game::components::Projectile>(entity) != nullptr) {
                return;
            }

            // Clamp position to screen boundaries
            if (pos.x < PLAYER_MARGIN) {
                pos.x = PLAYER_MARGIN;
            } else if (pos.x > SCREEN_WIDTH - PLAYER_MARGIN) {
                pos.x = SCREEN_WIDTH - PLAYER_MARGIN;
            }

            if (pos.y < PLAYER_MARGIN) {
                pos.y = PLAYER_MARGIN;
            } else if (pos.y > SCREEN_HEIGHT - PLAYER_MARGIN) {
                pos.y = SCREEN_HEIGHT - PLAYER_MARGIN;
            }
        });
}

}  // namespace rtype::game

