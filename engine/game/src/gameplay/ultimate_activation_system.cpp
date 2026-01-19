#include "engine/game/systems/gameplay/ultimate_activation_system.hpp"

#include "engine/core/registry.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/velocity.hpp"
#include "engine/game/components/gameplay/collider.hpp"
#include "engine/game/components/gameplay/faction.hpp"
#include "engine/game/components/gameplay/input_state.hpp"
#include "engine/game/components/gameplay/projectile.hpp"
#include "engine/game/components/gameplay/ultimate_charge.hpp"
#include "engine/game/components/gameplay/ultimate_projectile.hpp"
#include "engine/game/components/network/owner.hpp"

namespace rtype::game {

void UltimateActivationSystem::run(rtype::ecs::registry& reg) {
    reg.view<engine::game::components::Position,
             engine::game::components::InputState,
             engine::game::components::FactionComponent,
             engine::game::components::UltimateCharge>(
        [&](rtype::ecs::entity_t entity, auto& position, auto& input, auto& faction, auto& charge) {
            if (faction.faction_value != engine::game::components::Faction::PLAYER) {
                return;
            }
            if (!input.ultimate || !charge.ready) {
                return;
            }

            charge.kills_since_last_ulti = 0;
            charge.ui_frame = 1;
            charge.ready = false;

            auto projectile_entity = reg.spawn_entity();

            reg.add_component(projectile_entity,
                engine::game::components::Position{
                    position.x + kProjectileOffsetX,
                    position.y
                });

            reg.add_component(projectile_entity,
                engine::game::components::Velocity{
                    kProjectileSpeed,
                    0.0f
                });

            reg.add_component(projectile_entity,
                engine::game::components::FactionComponent{
                    engine::game::components::Faction::PLAYER
                });

            reg.add_component(projectile_entity,
                engine::game::components::Projectile{
                    kUltimateDamage,
                    static_cast<int>(entity.id()),
                    kProjectileLifetime,
                    0.0f
                });

            reg.add_component(projectile_entity,
                engine::game::components::Collider{
                    kColliderWidth,
                    kColliderHeight,
                    false
                });

            auto* owner = reg.try_get<engine::game::components::Owner>(entity);
            if (owner) {
                reg.add_component(projectile_entity, engine::game::components::Owner{owner->player_id});
                reg.add_component(projectile_entity, engine::game::components::UltimateProjectile{owner->player_id});
            } else {
                reg.add_component(projectile_entity, engine::game::components::UltimateProjectile{0});
            }
        });
}

}  // namespace rtype::game
