#pragma once

#include <chrono>
#include <deque>
#include <cstdint>

#include "engine/core/registry.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/velocity.hpp"

namespace client::systems {

/**
 * @brief Client-side prediction system for instant input response
 * 
 * Applies player inputs immediately on the client before server confirmation,
 * then reconciles when server snapshots arrive to maintain consistency.
 * 
 * Architecture compliance:
 * - Lives in client/systems (client-specific visual/UX concern)
 * - Only modifies local player entity (server remains authoritative)
 * - No SFML dependencies (pure component manipulation)
 */
class ClientPredictionSystem {
public:
    ClientPredictionSystem();

    /**
     * @brief Apply input locally and store for reconciliation
     * Call this when local player sends input to server
     * 
     * @param registry The ECS registry
     * @param player_entity_id The local player's entity ID
     * @param input_mask Input bitmask
     * @param input_sequence Sequence number of this input
     * @param delta_time Time step to apply movement
     */
    void apply_input_locally(
        rtype::ecs::registry& registry,
        std::uint16_t player_entity_id,
        std::uint16_t input_mask,
        std::uint32_t input_sequence,
        float delta_time
    );

    /**
     * @brief Reconcile prediction when server snapshot arrives
     * 
     * @param registry The ECS registry
     * @param player_entity_id The local player's entity ID
     * @param server_x Server-authoritative X position
     * @param server_y Server-authoritative Y position
     * @param last_processed_input Last input sequence the server processed
     * @param delta_time Time step for re-applying inputs
     */
    void reconcile(
        rtype::ecs::registry& registry,
        std::uint16_t player_entity_id,
        float server_x,
        float server_y,
        std::uint32_t last_processed_input,
        float delta_time
    );

    /**
     * @brief Reset prediction state (e.g., on disconnect/reconnect)
     */
    void reset();

    /**
     * @brief Set movement speed in pixels per second
     */
    void set_move_speed(float speed) { move_speed_ = speed; }

private:
    struct PendingInput {
        std::uint32_t sequence;
        std::uint16_t input_mask;
        std::chrono::steady_clock::time_point timestamp;
    };

    std::deque<PendingInput> pending_inputs_;
    float move_speed_{300.0f};  // Pixels per second
    std::chrono::steady_clock::time_point start_time_;

    void apply_movement(
        engine::game::components::Position& pos,
        std::uint16_t input_mask,
        float delta_time
    );
};

}  // namespace client::systems
