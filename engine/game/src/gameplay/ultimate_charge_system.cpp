#include "engine/game/systems/gameplay/ultimate_charge_system.hpp"

#include <algorithm>
#include <unordered_map>

#include "engine/core/registry.hpp"
#include "engine/game/components/gameplay/faction.hpp"
#include "engine/game/components/gameplay/health.hpp"
#include "engine/game/components/gameplay/killer.hpp"
#include "engine/game/components/gameplay/ultimate_charge.hpp"
#include "engine/game/components/network/owner.hpp"

namespace rtype::game {

void UltimateChargeSystem::run(rtype::ecs::registry& reg) {
    std::unordered_map<std::uint16_t, std::uint8_t> kills_by_player;

    reg.view<engine::game::components::FactionComponent,
             engine::game::components::Health,
             engine::game::components::Killer>(
        [&](rtype::ecs::entity_t /*entity*/, auto& faction, auto& health, auto& killer) {
            if (faction.faction_value != engine::game::components::Faction::ENEMY) {
                return;
            }
            if (health.current > 0) {
                return;
            }
            if (killer.player_id == 0) {
                return;
            }

            auto& count = kills_by_player[killer.player_id];
            if (count < kMaxKills) {
                ++count;
            }
        });

    if (kills_by_player.empty()) {
        return;
    }

    constexpr std::uint8_t kMaxFrame = kMaxKills + 1;

    reg.view<engine::game::components::Owner, engine::game::components::UltimateCharge>(
        [&](rtype::ecs::entity_t /*entity*/, auto& owner, auto& charge) {
            const auto it = kills_by_player.find(owner.player_id);
            if (it == kills_by_player.end()) {
                return;
            }

            if (charge.ready) {
                charge.kills_since_last_ulti = kMaxKills;
                charge.ui_frame = kMaxFrame;
                return;
            }

            const std::uint8_t new_kills = static_cast<std::uint8_t>(
                std::min<int>(kMaxKills, charge.kills_since_last_ulti + it->second));
            charge.kills_since_last_ulti = new_kills;
            charge.ui_frame = static_cast<std::uint8_t>(
                std::clamp<int>(new_kills + 1, 1, kMaxFrame));
            if (new_kills >= kMaxKills) {
                charge.ready = true;
            }
        });
}

}  // namespace rtype::game
