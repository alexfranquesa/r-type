#pragma once

namespace engine::game::components {

struct ParticleEffect {
    float lifetime_seconds{0.0f};
    float elapsed_seconds{0.0f};
    bool one_shot{true};
};

}  // namespace engine::game::components
