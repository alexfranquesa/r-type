// Dropdown control component for settings UI.
#pragma once

#include <string>
#include <vector>

#include "engine/game/components/ui/ui_setting_key.hpp"

namespace engine::game::components {

struct UIDropdown {
    float x{};
    float y{};
    float width{};
    float height{};
    std::vector<std::string> options{};
    int selected_index{};
    SettingKey key{SettingKey::None};
    bool open{false};
};

}  // namespace engine::game::components
