// Client-only tab ownership component for UI controls.
#pragma once

#include "engine/game/components/ui/ui_tab.hpp"

namespace client::components {

struct UITabOwner {
    engine::game::components::TabId tab{engine::game::components::TabId::Audio};
};

}  // namespace client::components
