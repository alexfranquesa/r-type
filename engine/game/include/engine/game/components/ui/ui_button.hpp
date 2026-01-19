// Simple UI button component for ECS-driven UI.
#pragma once

#include <string>

namespace engine::game::components {

struct UIButton {
    std::string id;   // logical identifier (e.g., "connect", "quit")
    bool hovered{false};
    bool pressed{false};
};

}  // namespace engine::game::components
