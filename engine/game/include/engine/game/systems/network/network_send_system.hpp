/*
** EPITECH PROJECT, 2025
** G-CPP-500-BAR-5-2-rtype-2
** File description:
** network_send_system
*/

#pragma once

#include <cstdint>
#include <cstring>
#include <unordered_map>
#include "engine/core/registry.hpp"
#include "engine/net/packet.hpp"

namespace rtype::game {

class NetworkSendSystem {
public:
    engine::net::SnapshotMessage build_snapshot(
        rtype::ecs::registry& reg, 
        std::uint32_t tick,
        bool paused
    );

    void set_debug_logging(bool enabled) { debug_logging_ = enabled; }
    void set_delta_compression(bool enabled) { delta_compression_enabled_ = enabled; }
    void set_full_snapshot_interval(std::uint32_t interval) { full_snapshot_interval_ = interval; }

private:
    struct EntitySnapshot {
        float x{0.0f};
        float y{0.0f};
        float vx{0.0f};
        float vy{0.0f};
        std::int16_t hp_cur{0};
        std::int16_t hp_max{0};
        std::uint16_t sprite_id{0};
        std::uint16_t owner_id{0};
        std::int16_t lives_remaining{-1};
        std::int16_t lives_max{-1};
        std::uint8_t is_spectating{0};
        std::uint8_t ultimate_frame{0};
        std::uint8_t ultimate_ready{0};
    };

    bool debug_logging_ = false;
    bool delta_compression_enabled_ = true;  // Enable by default
    std::uint32_t full_snapshot_interval_ = 60;  // Send full snapshot every N ticks
    std::uint32_t last_full_snapshot_tick_ = 0;
    std::unordered_map<std::uint16_t, EntitySnapshot> previous_state_;

    void serialize_float(std::vector<std::uint8_t>& blob, float value);
    void serialize_uint16(std::vector<std::uint8_t>& blob, std::uint16_t value);
    void serialize_uint32(std::vector<std::uint8_t>& blob, std::uint32_t value);
    void serialize_int16(std::vector<std::uint8_t>& blob, std::int16_t value);
    std::uint16_t pick_sprite_id(rtype::ecs::registry& reg, std::size_t entity_id) const;
    bool entity_changed(std::uint16_t entity_id, const EntitySnapshot& current) const;
};

}  // namespace rtype::game
