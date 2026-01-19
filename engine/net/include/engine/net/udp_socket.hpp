#pragma once

#include <asio.hpp>
#include <cstdint>
#include <span>

namespace engine::net {

class UdpSocket {
public:
    explicit UdpSocket(asio::io_context& io)
        : io_(io), socket_(io_) {}

    void bind(std::uint16_t port) {
        asio::ip::udp::endpoint endpoint(asio::ip::udp::v4(), port);
        socket_.open(endpoint.protocol());
        socket_.bind(endpoint);
    }

    std::size_t receive_from(std::span<std::uint8_t> buffer, asio::ip::udp::endpoint& endpoint) {
        return socket_.receive_from(asio::buffer(buffer.data(), buffer.size()), endpoint);
    }

    void send_to(std::span<const std::uint8_t> data, const asio::ip::udp::endpoint& endpoint) {
        socket_.send_to(asio::buffer(data.data(), data.size()), endpoint);
    }

    asio::ip::udp::socket& native() { return socket_; }

private:
    asio::io_context& io_;
    asio::ip::udp::socket socket_;
};

}  // namespace engine::net
