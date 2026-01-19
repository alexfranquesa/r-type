#include "client_prediction_system.hpp"

#include <algorithm>
#include <iostream>

namespace client::systems {

// Input bit masks (from InputKeys in network_client_interface.hpp)
constexpr std::uint16_t INPUT_LEFT  = (1 << 0);
constexpr std::uint16_t INPUT_RIGHT = (1 << 1);
constexpr std::uint16_t INPUT_UP    = (1 << 2);
constexpr std::uint16_t INPUT_DOWN  = (1 << 3);
constexpr std::uint16_t INPUT_SHOOT = (1 << 4);

ClientPredictionSystem::ClientPredictionSystem() 
    : start_time_(std::chrono::steady_clock::now()) {}

void ClientPredictionSystem::apply_input_locally(
    rtype::ecs::registry& registry,
    std::uint16_t player_entity_id,
    std::uint16_t input_mask,
    std::uint32_t input_sequence,
    float delta_time
) {
    // Store this input for potential reconciliation
    PendingInput pending{
        input_sequence,
        input_mask,
        std::chrono::steady_clock::now()
    };
    pending_inputs_.push_back(pending);

    // Keep only last 1 second of inputs (at 60 FPS = ~60 inputs)
    constexpr auto max_age = std::chrono::seconds(1);
    while (!pending_inputs_.empty() && 
           (pending.timestamp - pending_inputs_.front().timestamp) > max_age) {
        pending_inputs_.pop_front();
    }

    // Apply movement immediately to local player
    rtype::ecs::entity_t entity{static_cast<rtype::ecs::entity_id_t>(player_entity_id)};
    auto pos = registry.try_get<engine::game::components::Position>(entity);
    if (pos) {
        apply_movement(*pos, input_mask, delta_time);
    }
}

void ClientPredictionSystem::reconcile(
    rtype::ecs::registry& registry,
    std::uint16_t player_entity_id,
    float server_x,
    float server_y,
    std::uint32_t last_processed_input,
    float delta_time
) {
    rtype::ecs::entity_t entity{static_cast<rtype::ecs::entity_id_t>(player_entity_id)};
    auto pos = registry.try_get<engine::game::components::Position>(entity);
    if (!pos) {
        return;
    }

    // Remove all inputs up to and including the one the server processed
    while (!pending_inputs_.empty() && 
           pending_inputs_.front().sequence <= last_processed_input) {
        pending_inputs_.pop_front();
    }

    // Check if our prediction matches the server
    const float prediction_error = std::abs(pos->x - server_x) + std::abs(pos->y - server_y);
    constexpr float error_threshold = 2.0f;  // 2 pixels tolerance

    if (prediction_error > error_threshold) {
        // Misprediction detected - snap to server position
        pos->x = server_x;
        pos->y = server_y;

        // Re-apply all pending inputs that came after the server's last processed input
        for (const auto& pending : pending_inputs_) {
            apply_movement(*pos, pending.input_mask, delta_time);
        }
        
        // Log significant mispredictions for debugging
        if (prediction_error > 10.0f) {
            std::cout << "[ClientPrediction] Large correction: " << prediction_error 
                      << "px, re-applied " << pending_inputs_.size() << " inputs\n";
        }
    }
    // else: prediction was accurate, no correction needed
}

void ClientPredictionSystem::reset() {
    pending_inputs_.clear();
}

void ClientPredictionSystem::apply_movement(
    engine::game::components::Position& pos,
    std::uint16_t input_mask,
    float delta_time
) {
    float dx = 0.0f;
    float dy = 0.0f;

    if (input_mask & INPUT_LEFT)  dx -= 1.0f;
    if (input_mask & INPUT_RIGHT) dx += 1.0f;
    if (input_mask & INPUT_UP)    dy -= 1.0f;
    if (input_mask & INPUT_DOWN)  dy += 1.0f;

    // Normalize diagonal movement
    if (dx != 0.0f && dy != 0.0f) {
        const float inv_sqrt_2 = 0.70710678118f;  // 1/sqrt(2)
        dx *= inv_sqrt_2;
        dy *= inv_sqrt_2;
    }

    // Apply movement
    pos.x += dx * move_speed_ * delta_time;
    pos.y += dy * move_speed_ * delta_time;
}

}  // namespace client::systems
