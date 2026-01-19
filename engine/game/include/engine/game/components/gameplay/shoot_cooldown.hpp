#pragma once

namespace engine::game::components {

struct ShootCooldown {
    float cooldown_seconds{0.0f};
    float remaining_seconds{0.0f};
    bool enabled{true};

    bool ready() const {
        return enabled && remaining_seconds <= 0.0f;
    }

    void reset() {
        remaining_seconds = cooldown_seconds;
    }
};

}  // namespace engine::game::components
