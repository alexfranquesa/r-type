#include "engine/game/systems/gameplay/movement_pattern_system.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/velocity.hpp"
#include "engine/game/components/gameplay/movement_pattern.hpp"
#include <algorithm>
#include <cmath>

namespace {
constexpr float PI  = 3.14159265f;
constexpr float TAU = 6.28318531f;

// Triangle wave normalized to [-1, 1] for the given phase
float triangle_wave(float phase) {
    float wrapped = std::fmod(phase, TAU);
    if (wrapped < 0.0f) {
        wrapped += TAU;
    }
    float t = wrapped / PI;  // 0 -> 2
    return (t < 1.0f) ? (t * 2.0f - 1.0f) : (3.0f - t * 2.0f);
}
}  // namespace

namespace rtype::game {

void MovementPatternSystem::run(rtype::ecs::registry& reg, float dt) {
    reg.view<engine::game::components::Position, engine::game::components::MovementPattern>(
        [&](size_t /*entity_id*/, auto& pos, auto& pattern) {
            // Update time and keep phase wrapped
            pattern.elapsed += dt;
            pattern.phase = std::fmod(pattern.phase + pattern.frequency * TAU * dt, TAU);
            if (pattern.phase < 0.0f) {
                pattern.phase += TAU;
            }

            // Remove last frame's offset so we don't accumulate drift
            const float base_x = pos.x - pattern.offset_x;
            const float base_y = pos.y - pattern.offset_y;

            float offset_x = 0.0f;
            float offset_y = 0.0f;
            
            switch (pattern.type) {
                case engine::game::components::MovementPatternType::LINEAR:
                    // Straight line, no additional offsets
                    break;
                    
                case engine::game::components::MovementPatternType::SINE_WAVE:
                    offset_y = pattern.amplitude * std::sin(pattern.phase);
                    break;
                    
                case engine::game::components::MovementPatternType::ZIGZAG:
                    offset_y = pattern.amplitude * triangle_wave(pattern.phase);
                    break;
                    
                case engine::game::components::MovementPatternType::DIVE: {
                    // Repeating dive: down then back up over a full cycle
                    const float freq = std::max(pattern.frequency, 0.25f);
                    const float cycle = std::fmod(pattern.elapsed * freq, 1.0f);
                    const float dive_phase = (cycle < 0.5f) ? (cycle * 2.0f) : (2.0f - cycle * 2.0f);
                    offset_y = pattern.amplitude * std::sin(dive_phase * PI);
                    break;
                }
                    
                case engine::game::components::MovementPatternType::CIRCLE: {
                    // Circular movement around the current trajectory baseline
                    const float radius = pattern.amplitude * 0.6f;
                    offset_x = radius * std::cos(pattern.phase);
                    offset_y = radius * std::sin(pattern.phase);
                    break;
                }
            }

            pos.x = base_x + offset_x;
            pos.y = base_y + offset_y;

            pattern.offset_x = offset_x;
            pattern.offset_y = offset_y;
            pattern.base_y = base_y;  // keep anchor in sync if other systems move the entity
        });
}

}  // namespace rtype::game
