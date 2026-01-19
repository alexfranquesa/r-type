#pragma once

namespace engine::game::components {

    /**
     * UITransform component for UI element positioning and sizing.
     */
    struct UITransform {
        float x{0.0F};           ///< X position in screen coordinates.
        float y{0.0F};           ///< Y position in screen coordinates.
        float width{0.0F};       ///< Width of the UI element.
        float height{0.0F};      ///< Height of the UI element.
        float anchor_x{0.0F};    ///< Anchor point X (0.0 = left, 0.5 = center, 1.0 = right).
        float anchor_y{0.0F};    ///< Anchor point Y (0.0 = top, 0.5 = center, 1.0 = bottom).
    };

}  // namespace engine::game::components
