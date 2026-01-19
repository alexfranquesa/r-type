#pragma once

#include <cstdint>

namespace engine::game::components {

    /**
     * ScoreValue component for entities that give score when destroyed.
     */
    struct ScoreValue {
        std::uint32_t value{100};  
    };

}  // namespace engine::game::components
