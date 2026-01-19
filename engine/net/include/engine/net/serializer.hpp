#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <type_traits>
#include <vector>

namespace engine::net {

inline void write_bytes(std::vector<std::uint8_t>& buffer, std::span<const std::uint8_t> bytes) {
    if (bytes.empty()) return;
    const auto old_size = buffer.size();
    buffer.resize(old_size + bytes.size());
    std::memcpy(buffer.data() + old_size, bytes.data(), bytes.size());
}

inline bool read_bytes(std::span<const std::uint8_t>& buffer, std::span<std::uint8_t> out) {
    if (buffer.size() < out.size()) {
        return false;
    }
    std::copy_n(buffer.begin(), out.size(), out.begin());
    buffer = buffer.subspan(out.size());
    return true;
}

template <typename T>
void write_value(std::vector<std::uint8_t>& buffer, const T& value) {
    static_assert(std::is_trivially_copyable_v<T>, "write_value requires trivially copyable types");
    const auto* data = reinterpret_cast<const std::uint8_t*>(&value);
    write_bytes(buffer, std::span<const std::uint8_t>(data, sizeof(T)));
}

template <typename T>
bool read_value(std::span<const std::uint8_t>& buffer, T& out) {
    static_assert(std::is_trivially_copyable_v<T>, "read_value requires trivially copyable types");
    if (buffer.size() < sizeof(T)) {
        return false;
    }
    std::memcpy(&out, buffer.data(), sizeof(T));
    buffer = buffer.subspan(sizeof(T));
    return true;
}

}  // namespace engine::net
