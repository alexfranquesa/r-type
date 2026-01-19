#pragma once

#include <cstdint>

namespace engine::game::components {

/**
 * @brief Movement pattern types for enemies.
 */
enum class MovementPatternType : std::uint8_t {
    LINEAR = 0,      ///< Straight line movement (default)
    SINE_WAVE,       ///< Sinusoidal up/down movement
    ZIGZAG,          ///< Sharp zigzag pattern
    DIVE,            ///< Dive towards player Y position then return
    CIRCLE           ///< Circular/spiral movement
};

/**
 * @brief Component that defines an entity's movement pattern.
 */
struct MovementPattern {
    MovementPatternType type = MovementPatternType::LINEAR;
    float amplitude = 80.0f;     ///< Amplitude of oscillation (pixels)
    float frequency = 0.6f;      ///< Frequency of oscillation (Hz)
    float phase = 0.0f;          ///< Current phase (radians)
    float base_y = 0.0f;         ///< Original Y position (for oscillation center)
    float elapsed = 0.0f;        ///< Time elapsed since spawn
    float offset_x = 0.0f;       ///< Last applied X offset (for patterns that move sideways)
    float offset_y = 0.0f;       ///< Last applied Y offset (for patterns that move vertically)
};

}  // namespace engine::game::components
