#pragma once

#include <asio.hpp>
#include <atomic>
#include <cstdint>
#include <string>
#include <optional>
#include <thread>

#include "engine/net/packet.hpp"
#include "engine/net/thread_safe_queue.hpp"
#include "engine/net/udp_socket.hpp"
#include "engine/net/network_client_interface.hpp"

class NetworkClient : public engine::net::INetworkClient {
public:
    NetworkClient(std::string host, std::uint16_t port);
    ~NetworkClient();

    bool connect(const std::string& player_name, std::uint16_t start_level = 1, std::uint8_t difficulty = 1) override;
    void send_input(std::uint16_t mask) override;
    std::optional<engine::net::SnapshotMessage> poll_snapshot() override;
    void shutdown() override;
    bool is_paused() const override { return paused_; }

    std::uint16_t get_player_id() const override { return player_id_; }
    std::uint32_t get_last_input_sequence() const { return sequence_counter_ - 1; }

private:
    void listen_loop();
    void ping_loop();
    bool paused_ = false;

    std::string host_;
    std::uint16_t port_;
    asio::io_context io_ctx_;
    engine::net::UdpSocket socket_;
    asio::ip::udp::endpoint server_endpoint_;
    std::atomic_bool running_{false};
    std::thread listen_thread_;
    std::thread ping_thread_;
    std::uint16_t player_id_{0};
    std::atomic<std::uint32_t> sequence_counter_{0};
    engine::net::ThreadSafeQueue<engine::net::SnapshotMessage> snapshot_queue_;
};
