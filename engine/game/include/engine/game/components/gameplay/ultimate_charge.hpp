#pragma once

#include <cstdint>

namespace engine::game::components {

struct UltimateCharge {
    std::uint8_t kills_since_last_ulti{0};
    std::uint8_t ui_frame{1};
    bool ready{false};
};

}  // namespace engine::game::components
