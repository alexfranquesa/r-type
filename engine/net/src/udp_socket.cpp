#include "engine/net/udp_socket.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>

UdpSocket::UdpSocket() {
    sock = socket(AF_INET, SOCK_DGRAM, 0);
}

UdpSocket::~UdpSocket() {
    if (sock >= 0)
        close(sock);
}

bool UdpSocket::bindTo(int port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;        // IPv4
    addr.sin_addr.s_addr = INADDR_ANY; // Accept packets from any IP
    addr.sin_port = htons(port);      // Convert port to network byte order

    // Bind socket to the port
    return bind(sock, (sockaddr*)&addr, sizeof(addr)) == 0;
}

int UdpSocket::sendTo(const std::string &ip, int port, const std::vector<uint8_t> &data) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;        // IPv4
    addr.sin_port = htons(port);      // Convert port
    inet_aton(ip.c_str(), &addr.sin_addr); // Convert IP string to binary

    // Send UDP datagram
    return sendto(sock, data.data(), data.size(), 0,
                  (sockaddr*)&addr, sizeof(addr));
}

int UdpSocket::receive(std::vector<uint8_t> &buffer, std::string &fromIp, int &fromPort) {
    buffer.resize(2048);

    sockaddr_in sender{};
    socklen_t senderLen = sizeof(sender);

    // Non-blocking receive (MSG_DONTWAIT)
    int bytes = recvfrom(sock, buffer.data(), buffer.size(), MSG_DONTWAIT,
                         (sockaddr*)&sender, &senderLen);

    if (bytes <= 0)
        return 0; // No data received

    buffer.resize(bytes);
    fromIp = inet_ntoa(sender.sin_addr);
    fromPort = ntohs(sender.sin_port);

    return bytes;
}
