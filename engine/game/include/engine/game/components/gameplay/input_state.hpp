#pragma once

namespace engine::game::components {

    struct InputState {
        bool up{false};
        bool down{false};
        bool left{false};
        bool right{false};
        bool shoot{false};
        bool ultimate{false};
    };

}  // namespace engine::game::components
