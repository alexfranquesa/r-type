#include "apply_input_system.hpp"

#include "engine/game/components/core/velocity.hpp"
#include "engine/game/components/gameplay/faction.hpp"
#include "engine/game/components/gameplay/input_state.hpp"
#include "engine/game/components/gameplay/spectator.hpp"

#include <iostream>

namespace server::systems {

void ApplyInputSystem::register_player_entity(std::uint16_t player_id,
                                             std::uint16_t entity_id) {
    player_to_entity_[player_id] = entity_id;
    std::cout << "[ApplyInputSystem] Registered player #" << player_id
              << " -> entity #" << entity_id << std::endl;
}

void ApplyInputSystem::set_player_input(std::uint16_t player_id,
                                       std::uint16_t input_mask,
                                       std::uint32_t sequence) {
    auto& buffer = player_inputs_[player_id];

    // Drop duplicate or out-of-order packets
    if (sequence <= buffer.last_sequence) {
        return;
    }

    buffer.last_sequence = sequence;
    buffer.last_input_mask = input_mask;
}

void ApplyInputSystem::update(rtype::ecs::registry& registry) {
    auto& velocities   = registry.get_components<engine::game::components::Velocity>();
    auto& input_states = registry.get_components<engine::game::components::InputState>();

    // Iterate through all registered players
    for (const auto& [player_id, entity_id] : player_to_entity_) {
        // Check if player is a spectator - if so, ignore their input
        auto entity = rtype::ecs::entity_t{entity_id};
        if (registry.try_get<engine::game::components::Spectator>(entity)) {
            // Player is spectating, don't process their input
            continue;
        }
        
        // Ensure entity exists and has velocity
        if (entity_id >= velocities.size() || !velocities[entity_id]) {
            continue;
        }

        // Ensure InputState exists for this entity
        if (entity_id >= input_states.size() || !input_states[entity_id]) {
            registry.emplace_component<engine::game::components::InputState>(
                rtype::ecs::entity_t{entity_id});
        }

        auto& input = *input_states[entity_id];

        auto it = player_inputs_.find(player_id);
        std::uint16_t mask =
            (it == player_inputs_.end()) ? 0u : it->second.last_input_mask;

        input.up    = (mask & (1u << 0)) != 0;
        input.down  = (mask & (1u << 1)) != 0;
        input.left  = (mask & (1u << 2)) != 0;
        input.right = (mask & (1u << 3)) != 0;
        input.shoot = (mask & (1u << 4)) != 0;
        input.ultimate = (mask & (1u << 6)) != 0;
    }
}

void ApplyInputSystem::remove_player(std::uint16_t player_id,
                                     rtype::ecs::registry& registry) {
    auto it = player_to_entity_.find(player_id);
    if (it == player_to_entity_.end()) {
        return;
    }

    const auto entity_id = it->second;
    registry.kill_entity(rtype::ecs::entity_t{entity_id});

    player_to_entity_.erase(it);
    player_inputs_.erase(player_id);
}


}  // namespace server::systems
