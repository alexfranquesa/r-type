#include "particle_system.hpp"

#include <vector>

#include "engine/game/components/visual/particle_effect.hpp"

namespace client::systems {

void ParticleSystem::update(rtype::ecs::registry& registry, float dt) {
  using engine::game::components::ParticleEffect;

  std::vector<rtype::ecs::entity_t> to_kill;
  registry.view<ParticleEffect>([&](rtype::ecs::entity_t entity, ParticleEffect& effect) {
    effect.elapsed_seconds += dt;
    if (effect.lifetime_seconds > 0.0f && effect.elapsed_seconds >= effect.lifetime_seconds) {
      to_kill.push_back(entity);
    }
  });

  for (const auto entity : to_kill) {
    registry.kill_entity(entity);
  }
}

}  // namespace client::systems
