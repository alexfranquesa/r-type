#pragma once

#include "engine/core/registry.hpp"

namespace client::systems {

class AnimationSystem {
 public:
  void update(rtype::ecs::registry& registry, float dt);
};

}  // namespace client::systems
