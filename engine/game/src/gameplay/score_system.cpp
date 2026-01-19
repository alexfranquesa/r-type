#include "engine/game/systems/gameplay/score_system.hpp"
#include "engine/game/game_settings.hpp"

namespace engine::game::systems {

using engine::game::components::Health;
using engine::game::components::ScoreValueComponent;

void ScoreSystem::run(rtype::ecs::registry& registry, const engine::game::GameSettings& settings) {
    auto& scores = registry.get_components<ScoreValueComponent>();
    auto& healths = registry.get_components<Health>();

    for (std::size_t i = 0; i < scores.size(); ++i) {
        if (!scores[i].has_value()) {
            continue;
        }
        if (i >= healths.size() || !healths[i].has_value()) {
            continue;
        }
        auto& h = healths[i].value();
        if (h.current <= 0) {
            score_ += scores[i]->points;
            if (settings.kills_per_wave > 0) {
                kills_since_wave_ += 1;
                if (kills_since_wave_ >= settings.kills_per_wave) {
                    wave_ = static_cast<std::uint16_t>(wave_ + 1);
                    kills_since_wave_ = 0;
                }
            }
            registry.kill_entity(registry.entity_from_index(static_cast<rtype::ecs::entity_id_t>(i)));
        }
    }
}

}  // namespace engine::game::systems
