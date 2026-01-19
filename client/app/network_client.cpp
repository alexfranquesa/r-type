#include "network_client.hpp"

#include <array>
#include <chrono>
#include <iostream>
#include <cstdio>

namespace {
constexpr std::uint8_t kSnapshotLogLimit = 5;
}

NetworkClient::NetworkClient(std::string host, std::uint16_t port)
    : host_(std::move(host)), port_(port), socket_(io_ctx_) {}

NetworkClient::~NetworkClient() {
    shutdown();
}

bool NetworkClient::connect(const std::string& player_name, std::uint16_t start_level, std::uint8_t difficulty) {
    asio::ip::udp::endpoint local_endpoint(asio::ip::udp::v4(), 0);
    socket_.native().open(local_endpoint.protocol());
    socket_.native().bind(local_endpoint);
    server_endpoint_ =
        asio::ip::udp::endpoint(asio::ip::make_address(host_), port_);

    engine::net::Packet hello;
    hello.header.type = static_cast<std::uint8_t>(engine::net::MessageType::Hello);
    hello.header.sequence = sequence_counter_++;
    engine::net::HelloMessage hello_msg{};
    std::snprintf(hello_msg.player_name, sizeof(hello_msg.player_name), "%s", player_name.c_str());
    hello_msg.start_level = start_level;
    hello_msg.difficulty = difficulty;
    engine::net::encode_hello_payload(hello_msg, hello.payload);
    auto bytes = engine::net::serialize(hello);
    socket_.send_to(std::span<const std::uint8_t>(bytes.data(), bytes.size()), server_endpoint_);

    std::array<std::uint8_t, engine::net::kMaxPacketSize> buffer{};
    asio::ip::udp::endpoint sender;
    std::error_code ec;
    auto received = socket_.native().receive_from(asio::buffer(buffer.data(), buffer.size()), sender, 0, ec);
    if (ec) {
        std::cerr << "[client] Failed to receive welcome: " << ec.message() << std::endl;
        return false;
    }
    auto packet = engine::net::deserialize(
        std::span<const std::uint8_t>(buffer.data(), static_cast<std::size_t>(received)));
    if (!packet || packet->header.type != static_cast<std::uint8_t>(engine::net::MessageType::Welcome)) {
        std::cerr << "[client] Invalid welcome packet" << std::endl;
        return false;
    }
    auto welcome = engine::net::decode_welcome_payload(
        std::span<const std::uint8_t>(packet->payload.data(), packet->payload.size()));
    if (!welcome) {
        std::cerr << "[client] Failed to parse welcome payload" << std::endl;
        return false;
    }
    player_id_ = welcome->player_id;
    std::cout << "[client] Connected as #" << player_id_ << " tickrate=" << welcome->tick_rate << std::endl;

    running_ = true;
    listen_thread_ = std::thread([this] { listen_loop(); });
    ping_thread_ = std::thread([this] { ping_loop(); });
    return true;
}

void NetworkClient::send_input(std::uint16_t mask) {
    if (player_id_ == 0) {
        return;
    }
    engine::net::Packet packet;
    packet.header.type = static_cast<std::uint8_t>(engine::net::MessageType::Input);
    packet.header.sequence = sequence_counter_++;
    engine::net::InputMessage msg{
        .player_id = player_id_,
        .input_mask = mask,
        .client_time_ms = static_cast<std::uint32_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch())
                .count()),
    };
    engine::net::encode_input_payload(msg, packet.payload);
    auto bytes = engine::net::serialize(packet);
    socket_.send_to(std::span<const std::uint8_t>(bytes.data(), bytes.size()), server_endpoint_);
}

std::optional<engine::net::SnapshotMessage> NetworkClient::poll_snapshot() {
    return snapshot_queue_.try_pop();
}

void NetworkClient::shutdown() {
    if (!running_) {
        return;
    }
    running_ = false;
    
    // Shutdown and close socket
    std::error_code ec;
    socket_.native().shutdown(asio::socket_base::shutdown_both, ec);
    socket_.native().close(ec);
    
    // Stop io_context
    io_ctx_.stop();
    
    // Detach thread instead of joining (socket already closed, thread will exit)
    if (listen_thread_.joinable()) {
        listen_thread_.detach();
    }
    if (ping_thread_.joinable()) {
        ping_thread_.detach();
    }
}

void NetworkClient::listen_loop() {
    std::array<std::uint8_t, engine::net::kMaxPacketSize> buffer{};
    std::uint8_t snapshot_counter = 0;
    while (running_) {
        asio::ip::udp::endpoint sender;
        std::error_code ec;
        auto received = socket_.native().receive_from(asio::buffer(buffer.data(), buffer.size()), sender, 0, ec);
        if (ec) {
            if (running_) {
                std::cerr << "[client] Receive error: " << ec.message() << std::endl;
            }
            break;  // Exit immediately on error (socket closed)
        }
        auto packet = engine::net::deserialize(
            std::span<const std::uint8_t>(buffer.data(), static_cast<std::size_t>(received)));
        if (!packet) {
            continue;
        }
        if (packet->header.type == static_cast<std::uint8_t>(engine::net::MessageType::Snapshot)) {
            auto snapshot =
                engine::net::decode_snapshot_payload(
                    std::span<const std::uint8_t>(packet->payload.data(),
                                                packet->payload.size()));
            if (snapshot) {

                // ✅ STEP 3 — store pause state from server
                paused_ = snapshot->paused;

                if (snapshot_counter++ % kSnapshotLogLimit == 0) {
                    std::cout << "[client] Snapshot tick=" << snapshot->tick
                            << " paused=" << snapshot->paused
                            << " payload=" << snapshot->blob.size() << " bytes" << std::endl;
                }

                snapshot_queue_.push(std::move(*snapshot));
            }
        }
    }
}

void NetworkClient::ping_loop() {
    using namespace std::chrono_literals;
    while (running_) {
        engine::net::Packet packet;
        packet.header.type = static_cast<std::uint8_t>(engine::net::MessageType::Ping);
        packet.header.sequence = sequence_counter_++;
        auto bytes = engine::net::serialize(packet);
        std::error_code ec;
        socket_.native().send_to(asio::buffer(bytes.data(), bytes.size()), server_endpoint_, 0, ec);
        std::this_thread::sleep_for(500ms);  // Send ping every 0.5s (well below 2s timeout)
    }
}
