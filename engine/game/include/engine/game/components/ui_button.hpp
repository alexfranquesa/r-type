#pragma once

#include <functional>
#include <string>

namespace engine::game::components {

    /**
     * UIButton component for interactive UI elements.
     */
    struct UIButton {
        std::string label{};
        bool is_hovered{false};
        bool is_pressed{false};
        bool is_enabled{true};
        std::function<void()> on_click{};
    };

}  // namespace engine::game::components
