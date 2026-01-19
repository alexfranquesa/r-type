#include "network_server.hpp"

#include <array>
#include <iostream>
#include <span>

namespace server {

namespace {
// Allow idle clients to stay connected longer so snapshots keep flowing even if they stop sending input briefly.
// Reduced for testing - change back to 60 for production
constexpr std::chrono::seconds kClientTimeout{2};
constexpr std::uint16_t kServerTickrate = 60;
}  // namespace

NetworkServer::NetworkServer() = default;

NetworkServer::~NetworkServer() {
    stop();
}

void NetworkServer::start(std::uint16_t port) {
    if (running_) {
        return;
    }
    running_ = true;
    socket_ = std::make_unique<engine::net::UdpSocket>(io_ctx_);
    socket_->bind(port);

    listener_thread_ = std::thread([this] { listen_loop(); });
    maintenance_thread_ = std::thread([this] {
        while (running_) {
            prune_timeouts();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    std::cout << "[server] Networking listening on port " << port << std::endl;
}

void NetworkServer::stop() {
    if (!running_) {
        return;
    }
    running_ = false;
    io_ctx_.stop();
    if (listener_thread_.joinable()) {
        listener_thread_.join();
    }
    if (maintenance_thread_.joinable()) {
        maintenance_thread_.join();
    }
}

std::optional<InputCommand> NetworkServer::poll_input() {
    return input_queue_.try_pop();
}

void NetworkServer::broadcast_snapshot(const engine::net::SnapshotMessage& snapshot) {
    for (const auto& [_, client] : clients_) {
        send_snapshot(client, snapshot);
    }
}

void NetworkServer::listen_loop() {
    std::array<std::uint8_t, engine::net::kMaxPacketSize> buffer{};
    while (running_) {
        asio::ip::udp::endpoint endpoint;
        std::error_code ec;
        auto received =
            socket_->native().receive_from(asio::buffer(buffer.data(), buffer.size()), endpoint, 0, ec);
        if (ec) {
            if (running_) {
                std::cerr << "[server] Receive error: " << ec.message() << std::endl;
            }
            continue;
        }
        auto packet = engine::net::deserialize(
            std::span<const std::uint8_t>(buffer.data(), static_cast<std::size_t>(received)));
        if (!packet) {
            continue;
        }
        process_packet(*packet, endpoint);
    }
}

void NetworkServer::process_packet(const engine::net::Packet& packet,
                                   const asio::ip::udp::endpoint& endpoint) {
    // Update last_seen for any valid packet
    const auto key = endpoint_key(endpoint);
    auto it = clients_.find(key);
    if (it != clients_.end()) {
        it->second.last_seen = std::chrono::steady_clock::now();
    }

    switch (static_cast<engine::net::MessageType>(packet.header.type)) {
        case engine::net::MessageType::Hello:
            handle_hello(packet, endpoint);
            break;
        case engine::net::MessageType::Input:
            handle_input(packet, endpoint);
            break;
        case engine::net::MessageType::Ping: {
            auto bytes = engine::net::serialize(packet);
            socket_->send_to(std::span<const std::uint8_t>(bytes.data(), bytes.size()), endpoint);
            break;
        }
        default:
            break;
    }
}

void NetworkServer::handle_hello(const engine::net::Packet& packet,
                                 const asio::ip::udp::endpoint& endpoint) {
    const auto key = endpoint_key(endpoint);
    auto it = clients_.find(key);
    if (it == clients_.end()) {
        // Decode HelloMessage to get start_level and difficulty
        auto payload = std::span<const std::uint8_t>(packet.payload.data(), packet.payload.size());
        auto hello_msg = engine::net::decode_hello_payload(payload);
        std::uint16_t start_level = hello_msg.start_level;
        if (start_level < 1 || start_level > 5) {
            start_level = 1;  // Clamp to valid range
        }
        std::uint8_t difficulty = hello_msg.difficulty;
        if (difficulty > 3) {
            difficulty = 1;  // Clamp to valid range (0-3)
        }

        const auto client_id = next_client_id_++;
        std::uint16_t entity_id = 0;
        if (on_player_connect_) {
            entity_id = on_player_connect_(client_id, start_level, difficulty);
        }
        ClientInfo info{};
        info.id = client_id;
        info.endpoint = endpoint;
        info.last_seen = std::chrono::steady_clock::now();
        info.entity_id = entity_id;
        clients_.emplace(key, info);
        std::cout << "[server] New client #" << info.id << " (entity #" << entity_id << ", level " << start_level << ", difficulty " << static_cast<int>(difficulty) << ") from " << key << std::endl;
        send_welcome(info);
    } else {
        it->second.last_seen = std::chrono::steady_clock::now();
        send_welcome(it->second);
    }
}

void NetworkServer::handle_input(const engine::net::Packet& packet,
                                 const asio::ip::udp::endpoint& endpoint) {
    auto payload = std::span<const std::uint8_t>(packet.payload.data(), packet.payload.size());
    auto input = engine::net::decode_input_payload(payload);
    if (!input) {
        return;
    }
    const auto key = endpoint_key(endpoint);
    auto it = clients_.find(key);
    if (it == clients_.end()) {
        return;
    }
    it->second.last_seen = std::chrono::steady_clock::now();
    
    // Update last processed input sequence for this client
    it->second.last_processed_input = packet.header.sequence;
    
    InputCommand cmd{
        .player_id = input->player_id,
        .input_mask = input->input_mask,
        .client_time_ms = input->client_time_ms,
        .sequence = packet.header.sequence,
    };
    input_queue_.push(cmd);
}

void NetworkServer::send_welcome(const ClientInfo& client) {
    engine::net::Packet packet;
    packet.header.type = static_cast<std::uint8_t>(engine::net::MessageType::Welcome);
    packet.header.sequence = sequence_counter_++;
    engine::net::WelcomeMessage welcome{
        .player_id = client.id,
        .tick_rate = kServerTickrate,
    };
    engine::net::encode_welcome_payload(welcome, packet.payload);
    auto bytes = engine::net::serialize(packet);
    socket_->send_to(std::span<const std::uint8_t>(bytes.data(), bytes.size()), client.endpoint);
}

void NetworkServer::send_snapshot(const ClientInfo& client,
                                  const engine::net::SnapshotMessage& snapshot) {
    engine::net::Packet packet;
    packet.header.type = static_cast<std::uint8_t>(engine::net::MessageType::Snapshot);
    packet.header.sequence = sequence_counter_++;
    
    // Create client-specific snapshot with their last processed input
    engine::net::SnapshotMessage client_snapshot = snapshot;
    client_snapshot.last_processed_input = client.last_processed_input;
    
    engine::net::encode_snapshot_payload(client_snapshot, packet.payload);
    
    static std::uint32_t log_counter = 0;
    if (log_counter++ % 60 == 0) {
        std::cout << "[server] Sending snapshot: blob.size()=" << snapshot.blob.size() 
                  << " payload.size()=" << packet.payload.size() << std::endl;
    }
    
    auto bytes = engine::net::serialize(packet);
    socket_->send_to(std::span<const std::uint8_t>(bytes.data(), bytes.size()), client.endpoint);
}

void NetworkServer::prune_timeouts() {
    const auto now = std::chrono::steady_clock::now();
    for (auto it = clients_.begin(); it != clients_.end();) {
        if (now - it->second.last_seen > kClientTimeout) {
            std::cout << "[server] Client #" << it->second.id << " timed out\n";

            if (on_player_disconnect_) {
                on_player_disconnect_(it->second.id);
            }

            it = clients_.erase(it);
        } else {
            ++it;
        }
    }
}


std::string NetworkServer::endpoint_key(const asio::ip::udp::endpoint& endpoint) const {
    return endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
}

}  // namespace server
