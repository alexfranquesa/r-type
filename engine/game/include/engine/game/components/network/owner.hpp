// Identifies which client owns an entity (0 for server-owned like enemies).
#pragma once

#include <cstdint>

namespace engine::game::components {

struct Owner {
    std::uint16_t player_id{0};
};

}  // namespace engine::game::components
