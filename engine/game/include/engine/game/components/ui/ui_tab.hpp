// Tab component for settings UI.
#pragma once

#include <string>

namespace engine::game::components {

enum class TabId {
    Audio,
    Gameplay,
    Graphics,
    Network,
    Accessibility
};

struct UITab {
    std::string name{};
    float x{};
    float y{};
    float width{};
    float height{};
    TabId id{TabId::Audio};
    bool active{false};
};

}  // namespace engine::game::components
