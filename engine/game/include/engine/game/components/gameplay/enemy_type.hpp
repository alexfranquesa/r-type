#pragma once

#include <cstdint>

namespace engine::game::components {

/**
 * @brief Enum for different enemy types
 */
enum class EnemyType : std::uint8_t {
    Basic = 0,      // Standard enemy (green, red, orange based on level)
    IceCrab = 1,    // Level 4 ice crab (animated, stronger)
    Boss = 2,       // Level 5 final boss
};

/**
 * @brief Component to tag enemy type for sprite selection
 */
struct EnemyTypeComponent {
    EnemyType type = EnemyType::Basic;
};

}  // namespace engine::game::components
