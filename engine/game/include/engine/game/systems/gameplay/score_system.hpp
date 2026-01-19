#pragma once

#include "engine/core/registry.hpp"
#include <cstdint>
#include "engine/game/components/gameplay/score_value.hpp"
#include "engine/game/components/gameplay/health.hpp"

namespace engine::game {
    struct GameSettings;
}

namespace engine::game::systems {

// Simple score accumulator; increments a passed reference when entities with ScoreValue die.
class ScoreSystem {
public:
    ScoreSystem(int& score_ref, std::uint16_t& wave_ref)
        : score_(score_ref)
        , wave_(wave_ref)
    {}
    void run(rtype::ecs::registry& registry, const engine::game::GameSettings& settings);

private:
    int& score_;
    std::uint16_t& wave_;
    int kills_since_wave_{0};
};

}  // namespace engine::game::systems
