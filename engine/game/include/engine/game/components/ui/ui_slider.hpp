// Slider control component for settings UI.
#pragma once

#include "engine/game/components/ui/ui_setting_key.hpp"

namespace engine::game::components {

struct UISlider {
    float x{};
    float y{};
    float width{};
    float height{};
    float min_value{};
    float max_value{1.0f};
    float value{};
    SettingKey key{SettingKey::None};
    bool dragging{false};
};

}  // namespace engine::game::components
