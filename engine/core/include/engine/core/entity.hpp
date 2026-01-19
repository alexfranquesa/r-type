#pragma once

#include "types.hpp"

namespace rtype::ecs {

class entity {
public:
    using id_type = entity_id_t;

    // Explicit constructor: prevents implicit entity_id_t -> entity conversion
    explicit entity(id_type id = INVALID_ENTITY_ID) noexcept
        : _id{id}
    {}

    // Implicit conversion entity -> entity_id_t
    operator id_type() const noexcept {
        return _id;
    }

    // Get the underlying ID
    id_type id() const noexcept {
        return _id;
    }

    // Comparison operators
    friend bool operator==(entity lhs, entity rhs) noexcept {
        return lhs._id == rhs._id;
    }

    friend bool operator!=(entity lhs, entity rhs) noexcept {
        return !(lhs == rhs);
    }

    friend bool operator<(entity lhs, entity rhs) noexcept {
        return lhs._id < rhs._id;
    }

private:
    id_type _id;
};

using entity_t = entity;

}  // namespace rtype::ecs
