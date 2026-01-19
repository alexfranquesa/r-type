#include "animation_system.hpp"

#include "engine/game/components/visual/animation.hpp"
#include "engine/game/components/core/sprite.hpp"

namespace client::systems {

void AnimationSystem::update(rtype::ecs::registry& registry, float dt) {
  using engine::game::components::Animation;
  using engine::game::components::Sprite;

  registry.view<Animation, Sprite>([&](std::size_t /*eid*/, Animation& anim, Sprite& sprite) {
    if (!anim.playing || anim.frames.empty() || anim.frame_duration_seconds <= 0.0f) {
      return;
    }

    anim.accumulator_seconds += dt;
    while (anim.accumulator_seconds >= anim.frame_duration_seconds) {
      anim.accumulator_seconds -= anim.frame_duration_seconds;
      ++anim.frame_index;

      if (anim.frame_index >= anim.frames.size()) {
        if (anim.loop) {
          anim.frame_index = 0;
        } else {
          anim.frame_index = anim.frames.size() - 1;
          anim.playing = false;
          break;
        }
      }
    }

    if (const auto* frame = anim.current_frame()) {
      sprite.texture_rect = {frame->x, frame->y, frame->w, frame->h};
    }
  });
}

}  // namespace client::systems
