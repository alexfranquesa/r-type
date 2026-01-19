#pragma once

#include <cstdint>

namespace engine::game::components {

    /**
     * NetworkID component for networked entities.
     * Uniquely identifies entities across the network.
     */
    struct NetworkID {
        std::uint64_t id{0};       ///< Unique network identifier.
        bool is_local{false};       ///< True if this entity is controlled locally.
        bool replicated{true};      ///< True if this entity should be replicated over the network.
    };

}  // namespace engine::game::components
