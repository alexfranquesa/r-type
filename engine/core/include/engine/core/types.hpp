#pragma once

#include <cstdint>

namespace rtype::ecs {

// Entity ID type: uint32_t for stable network serialization
using entity_id_t = std::uint32_t;

// Invalid/null entity ID sentinel value
constexpr entity_id_t INVALID_ENTITY_ID = static_cast<entity_id_t>(-1);

}  // namespace rtype::ecs
