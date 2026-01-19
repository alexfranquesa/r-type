// Text input component for settings UI.
#pragma once

#include <cstddef>
#include <string>

#include "engine/game/components/ui/ui_setting_key.hpp"

namespace engine::game::components {

struct UITextInput {
    float x{};
    float y{};
    float width{};
    float height{};
    std::string text{};
    SettingKey key{SettingKey::None};
    bool focused{false};
    std::size_t cursor_index{};
};

}  // namespace engine::game::components
