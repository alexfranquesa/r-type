#pragma once

#include <cstdint>
#include <cstring>
#include <optional>
#include <span>
#include <vector>

#include "engine/net/serializer.hpp"

namespace engine::net {

inline constexpr std::uint16_t kPacketMagic = 0xCAFE;
inline constexpr std::uint8_t kProtocolVersion = 1;
inline constexpr std::size_t kMaxPacketSize = 1400;

enum class MessageType : std::uint8_t {
    Hello = 0,
    Welcome = 1,
    Input = 2,
    Snapshot = 3,
    Ping = 4
};

struct PacketHeader {
    std::uint16_t magic{kPacketMagic};
    std::uint8_t version{kProtocolVersion};
    std::uint8_t type{0};
    std::uint32_t sequence{0};
};

struct Packet {
    PacketHeader header;
    std::vector<std::uint8_t> payload;
};

inline std::vector<std::uint8_t> serialize(const Packet& packet) {
    std::vector<std::uint8_t> buffer;
    buffer.reserve(sizeof(PacketHeader) + packet.payload.size());
    write_value(buffer, packet.header.magic);
    write_value(buffer, packet.header.version);
    write_value(buffer, packet.header.type);
    write_value(buffer, packet.header.sequence);
    write_bytes(buffer, packet.payload);
    return buffer;
}

inline std::optional<Packet> deserialize(std::span<const std::uint8_t> buffer) {
    Packet packet;
    if (buffer.size() < sizeof(PacketHeader)) {
        return std::nullopt;
    }
    if (!read_value(buffer, packet.header.magic) ||
        !read_value(buffer, packet.header.version) ||
        !read_value(buffer, packet.header.type) ||
        !read_value(buffer, packet.header.sequence)) {
        return std::nullopt;
    }
    if (packet.header.magic != kPacketMagic || packet.header.version != kProtocolVersion) {
        return std::nullopt;
    }
    packet.payload.assign(buffer.begin(), buffer.end());
    return packet;
}

struct HelloMessage {
    char player_name[16]{};
    std::uint16_t start_level{1};  // Level to start at (1-5, default 1)
    std::uint8_t difficulty{1};    // Difficulty: 0=Easy, 1=Normal, 2=Hard, 3=Hardcore
};

struct WelcomeMessage {
    std::uint16_t player_id{0};
    std::uint16_t tick_rate{60};
};

struct InputMessage {
    std::uint16_t player_id{0};
    std::uint16_t input_mask{0};
    std::uint32_t client_time_ms{0};
};

struct SnapshotMessage {
    std::uint32_t tick{0};
    std::uint8_t flags{0};
    std::vector<std::uint8_t> blob;
    bool paused{false};
    std::uint32_t last_processed_input{0};  // For client-side prediction reconciliation
};

inline void encode_hello_payload(const HelloMessage& msg, std::vector<std::uint8_t>& payload) {
    payload.assign(std::begin(msg.player_name), std::end(msg.player_name));
    write_value(payload, msg.start_level);
    write_value(payload, msg.difficulty);
}

inline HelloMessage decode_hello_payload(std::span<const std::uint8_t> payload) {
    HelloMessage msg{};
    if (payload.size() >= sizeof(msg.player_name)) {
        std::memcpy(msg.player_name, payload.data(), sizeof(msg.player_name));
        auto remaining = payload.subspan(sizeof(msg.player_name));
        if (remaining.size() >= 2) {
            read_value(remaining, msg.start_level);
        }
        if (remaining.size() >= 1) {
            read_value(remaining, msg.difficulty);
        }
    } else {
        const auto len = std::min<std::size_t>(payload.size(), sizeof(msg.player_name));
        std::memcpy(msg.player_name, payload.data(), len);
    }
    return msg;
}

inline void encode_welcome_payload(const WelcomeMessage& msg, std::vector<std::uint8_t>& payload) {
    payload.clear();
    write_value(payload, msg.player_id);
    write_value(payload, msg.tick_rate);
}

inline std::optional<WelcomeMessage> decode_welcome_payload(std::span<const std::uint8_t> payload) {
    WelcomeMessage msg{};
    if (!read_value(payload, msg.player_id) || !read_value(payload, msg.tick_rate)) {
        return std::nullopt;
    }
    return msg;
}

inline void encode_input_payload(const InputMessage& msg, std::vector<std::uint8_t>& payload) {
    payload.clear();
    write_value(payload, msg.player_id);
    write_value(payload, msg.input_mask);
    write_value(payload, msg.client_time_ms);
}

inline std::optional<InputMessage> decode_input_payload(std::span<const std::uint8_t> payload) {
    InputMessage msg{};
    if (!read_value(payload, msg.player_id) || !read_value(payload, msg.input_mask) ||
        !read_value(payload, msg.client_time_ms)) {
        return std::nullopt;
    }
    return msg;
}

inline void encode_snapshot_payload(const SnapshotMessage& msg, std::vector<std::uint8_t>& payload) {
    payload.clear();
    write_value(payload, msg.tick);
    write_value(payload, msg.flags);
    write_value(payload, msg.paused);
    write_value(payload, msg.last_processed_input);
    payload.insert(payload.end(), msg.blob.begin(), msg.blob.end());
}


inline std::optional<SnapshotMessage> decode_snapshot_payload(std::span<const std::uint8_t> payload) {
    SnapshotMessage msg{};
    if (!read_value(payload, msg.tick) ||
        !read_value(payload, msg.flags) ||
        !read_value(payload, msg.paused) ||
        !read_value(payload, msg.last_processed_input)) {
        return std::nullopt;
    }

    msg.blob.assign(payload.begin(), payload.end());
    return msg;
}

}  // namespace engine::net
