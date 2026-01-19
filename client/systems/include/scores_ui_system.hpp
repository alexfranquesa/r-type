#pragma once

#include "engine/core/registry.hpp"
#include "engine/game/leaderboard.hpp"

namespace client::systems {

class ScoresUISystem {
public:
    ScoresUISystem() = default;

    void build_ui(rtype::ecs::registry& reg, const engine::game::Leaderboard& leaderboard);
};

}  // namespace client::systems
