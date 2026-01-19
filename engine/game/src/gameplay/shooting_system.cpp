#include "engine/game/systems/gameplay/shooting_system.hpp"
#include "engine/core/registry.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/velocity.hpp"
#include "engine/game/components/gameplay/input_state.hpp"
#include "engine/game/components/gameplay/faction.hpp"
#include "engine/game/components/gameplay/projectile.hpp"
#include "engine/game/components/gameplay/collider.hpp"
#include "engine/game/components/core/sprite.hpp"
#include "engine/game/components/network/owner.hpp"
#include "engine/game/game_settings.hpp"

namespace rtype::game {

void ShootingSystem::run(rtype::ecs::registry& reg, float dt, const engine::game::GameSettings& settings) {
    const float effective_cooldown = SHOOT_COOLDOWN * settings.player_shoot_cooldown_multiplier();
    // Actualiza cooldowns
    for (auto& [entity_id, cooldown] : cooldowns_) {
        cooldown -= dt;
        if (cooldown < 0.0f) {
            cooldown = 0.0f;
        }
    }

    // Procesar jugadores con view API (solo itera sobre entidades con todos los componentes)
    reg.view<engine::game::components::Position, engine::game::components::InputState, engine::game::components::FactionComponent>(
        [&](size_t entity_id, auto& position, auto& input, auto& faction) {
            // Solo procesar jugadores (verificar que la facción sea PLAYER)
            if (faction.faction_value != engine::game::components::Faction::PLAYER) {
                return;
            }
            
            // Verificar si el botón de disparo está presionado y el cooldown ha expirado
            if (input.shoot && cooldowns_[entity_id] <= 0.0f) {
                // Crear una nueva entidad de proyectil
                auto projectile_entity = reg.spawn_entity();
                const rtype::ecs::entity_t player_entity{static_cast<rtype::ecs::entity_id_t>(entity_id)};
                auto* player_owner = reg.try_get<engine::game::components::Owner>(player_entity);
                
                // Añadir componente de posición (generar proyectil en la posición del jugador)
                reg.add_component(projectile_entity, 
                    engine::game::components::Position{
                        position.x + 30.0f,  // Offset to spawn in front of player
                        position.y
                    });
                
                // Añadir componente de velocidad (mover proyectil hacia la derecha)
                reg.add_component(projectile_entity,
                    engine::game::components::Velocity{
                        PROJECTILE_SPEED,
                        0.0f
                    });

                // Faction so we can differentiate player projectiles on client/collision
                reg.add_component(projectile_entity,
                    engine::game::components::FactionComponent{
                        engine::game::components::Faction::PLAYER
                    });
                
                // Añadir componente de proyectil con metadata
                reg.add_component(projectile_entity,
                    engine::game::components::Projectile{
                        10,                          // damage
                        static_cast<int>(entity_id), // owner_id
                        5.0f,                        // lifetime
                        0.0f                         // elapsed_time
                    });
                
                if (player_owner) {
                    reg.add_component(projectile_entity,
                        engine::game::components::Owner{player_owner->player_id});
                }

                // Collider so collisions can be detected
                reg.add_component(projectile_entity,
                    engine::game::components::Collider{
                        8.0f,   // width
                        8.0f,   // height
                        false   // isTrigger
                    });

                // Añadir componente de sprite para renderizado
                reg.add_component(projectile_entity,
                    engine::game::components::Sprite{
                        "projectile",           // texture_id
                        {0, 0, 16, 16},        // texture_rect
                        1.0f,                   // scale_x
                        1.0f,                   // scale_y
                        0.0f,                   // origin_x
                        0.0f,                   // origin_y
                        1.0f,                   // z_index
                        true,                   // visible
                        false,                  // flip_x
                        false                   // flip_y
                    });
                
                // Reiniciar cooldown para esta entidad
                cooldowns_[entity_id] = effective_cooldown;
            }
        }
    );
}

}  // namespace rtype::game
