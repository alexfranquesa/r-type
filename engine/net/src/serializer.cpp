#pragma once
#include "serializer.hpp"

static std::vector<uint8_t> serialize(const Packet &p) {
    std::vector<uint8_t> buffer;
    buffer.push_back((uint8_t)p.type);

    buffer.insert(buffer.end(), p.data.begin(), p.data.end());
    return buffer;
}

static Packet deserialize(const std::vector<uint8_t> &buffer) {
    Packet p;
    p.type = (PacketType)buffer[0];
    p.data = std::vector<uint8_t>(buffer.begin() + 1, buffer.end());
    return p;
}