#pragma once

#include "engine/core/registry.hpp"
#include "engine/game/components/gameplay/health.hpp"

namespace engine::game {
    struct GameSettings;
}

namespace engine::game::systems {

/**
 * Gameplay health system.
 *
 * - Recorre todas las entidades con Health
 * - Hace clamp de current a [0, max]
 * - Si current == 0, destruye la entidad
 */
void health_system(rtype::ecs::registry& registry, const engine::game::GameSettings& settings);

} // namespace engine::game::systems
