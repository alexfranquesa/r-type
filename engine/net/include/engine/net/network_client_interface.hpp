#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "engine/net/packet.hpp"

namespace engine::net {

// Minimal client-side network abstraction so the game can run without knowing the transport.
class INetworkClient {
public:
    virtual ~INetworkClient() = default;

    // Connect to server with player name, optional starting level, and difficulty. Returns true on success.
    virtual bool connect(const std::string& player_name, std::uint16_t start_level = 1, std::uint8_t difficulty = 1) = 0;

    // Send packed input mask to the server.
    virtual void send_input(std::uint16_t mask) = 0;

    // Poll the most recent snapshot from the server (non-blocking).
    virtual std::optional<SnapshotMessage> poll_snapshot() = 0;

    // Graceful shutdown of network resources.
    virtual void shutdown() = 0;

    // Whether the client is currently paused (server-driven).
    virtual bool is_paused() const = 0;

    // Player id assigned by the server (0 if not connected).
    virtual std::uint16_t get_player_id() const = 0;
};

}  // namespace engine::net

