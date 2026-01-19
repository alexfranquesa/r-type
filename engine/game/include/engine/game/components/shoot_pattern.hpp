#pragma once

#include <cstdint>

namespace engine::game::components {

    /**
     * ShootPattern component to define shooting behavior.
     */
    enum class PatternType {
        SINGLE,         ///< Single projectile forward.
        SPREAD,         ///< Multiple projectiles in a spread.
        BURST,          ///< Rapid fire burst.
        CIRCULAR,       ///< Projectiles in a circle.
        AIMED           ///< Aimed at specific target.
    };

    struct ShootPattern {
        PatternType pattern{PatternType::SINGLE};
        float fire_rate{1.0F};          ///< Shots per second.
        float time_since_last_shot{0.0F};
        std::uint32_t projectile_count{1};  ///< Number of projectiles per shot.
        float spread_angle{0.0F};       ///< Angle spread in degrees for SPREAD pattern.
        float projectile_speed{500.0F};
        float damage{10.0F};
    };

}  // namespace engine::game::components
