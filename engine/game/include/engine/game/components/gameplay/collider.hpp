#pragma once

namespace engine::game::components {

    struct Collider {
        float width{};
        float height{};
        bool isTrigger{false};
        // Offset from entity position to hitbox center (useful for boss weak points)
        float offset_x{0.0f};
        float offset_y{0.0f};
    };

}  // namespace engine::game::components
