#pragma once

#include <asio.hpp>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "engine/net/packet.hpp"
#include "engine/net/thread_safe_queue.hpp"
#include "engine/net/udp_socket.hpp"

namespace server {

struct InputCommand {
    std::uint16_t player_id{0};
    std::uint16_t input_mask{0};
    std::uint32_t client_time_ms{0};
    std::uint32_t sequence{0};
};

class NetworkServer {
public:
    NetworkServer();
    ~NetworkServer();

    void start(std::uint16_t port);
    void stop();

    std::optional<InputCommand> poll_input();
    void broadcast_snapshot(const engine::net::SnapshotMessage& snapshot);
    
    // Callback signature: (player_id, start_level, difficulty) -> entity_id
    using OnPlayerConnectCallback = std::function<std::uint16_t(std::uint16_t player_id, std::uint16_t start_level, std::uint8_t difficulty)>;
    void set_on_player_connect(OnPlayerConnectCallback callback) { 
        on_player_connect_ = callback; 
    }

    using OnPlayerDisconnectCallback = std::function<void(std::uint16_t player_id)>;
    void set_on_player_disconnect(OnPlayerDisconnectCallback callback) {
        on_player_disconnect_ = callback;
    }


private:
    struct ClientInfo {
        std::uint16_t id;
        asio::ip::udp::endpoint endpoint;
        std::chrono::steady_clock::time_point last_seen;
        std::uint16_t entity_id{0};  // Player entity for this client
        std::uint32_t last_processed_input{0};  // Last input sequence processed for this client
    };

    void listen_loop();
    void process_packet(const engine::net::Packet& packet, const asio::ip::udp::endpoint& endpoint);
    void handle_hello(const engine::net::Packet& packet, const asio::ip::udp::endpoint& endpoint);
    void handle_input(const engine::net::Packet& packet, const asio::ip::udp::endpoint& endpoint);
    void send_welcome(const ClientInfo& client);
    void send_snapshot(const ClientInfo& client, const engine::net::SnapshotMessage& snapshot);
    void prune_timeouts();

    std::string endpoint_key(const asio::ip::udp::endpoint& endpoint) const;

    std::atomic_bool running_{false};
    asio::io_context io_ctx_;
    std::unique_ptr<engine::net::UdpSocket> socket_;
    std::thread listener_thread_;
    std::thread maintenance_thread_;
    engine::net::ThreadSafeQueue<InputCommand> input_queue_;
    std::unordered_map<std::string, ClientInfo> clients_;
    std::uint16_t next_client_id_{1};
    std::atomic<std::uint32_t> sequence_counter_{0};
    OnPlayerConnectCallback on_player_connect_;
    OnPlayerDisconnectCallback on_player_disconnect_;
};

}  // namespace server
