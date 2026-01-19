#pragma once

#include "engine/core/registry.hpp"
#include <cstdint>
#include <unordered_map>
#include "engine/game/components/gameplay/input_state.hpp"

namespace server::systems {

/**
 * @brief System that converts player input masks to velocity
 * 
 * Reads input_mask bits and sets Velocity component on player entities.
 * Input mask bits: 0=Up, 1=Down, 2=Left, 3=Right, 4=Shoot, 6=Ultimate
 */
class ApplyInputSystem {
public:
    /**
     * @brief Register mapping from player_id to entity_id
     * @param player_id Player identifier
     * @param entity_id Entity index in registry
     */
    void register_player_entity(std::uint16_t player_id, std::uint16_t entity_id);

    /**
     * @brief Store input for a specific player
     * @param player_id Player identifier
     * @param input_mask Bitmask of pressed keys
     */
    void set_player_input(std::uint16_t player_id,
                        std::uint16_t input_mask,
                        std::uint32_t sequence);

    /**
     * @brief Apply stored inputs to player entities
     * @param registry ECS registry containing entities
     */
    void update(rtype::ecs::registry& registry);
    void remove_player(std::uint16_t player_id, rtype::ecs::registry& registry);


private:
    struct PlayerInputBuffer {
        std::uint32_t last_sequence = 0;
        std::uint16_t last_input_mask = 0;
    };

    std::unordered_map<std::uint16_t, PlayerInputBuffer> player_inputs_;
    std::unordered_map<std::uint16_t, std::uint16_t> player_to_entity_;
    
    static constexpr float PLAYER_SPEED = 300.0f;  // pixels per second
};

}  // namespace server::systems
