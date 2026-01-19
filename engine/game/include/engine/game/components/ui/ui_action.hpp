// Action button component for settings UI.
#pragma once

namespace engine::game::components {

enum class ActionType {
    Apply,
    Save,
    Cancel,
    Back
};

struct UIAction {
    ActionType type{ActionType::Apply};
    bool pressed{false};
    float x{};
    float y{};
    float width{};
    float height{};
};

}  // namespace engine::game::components
