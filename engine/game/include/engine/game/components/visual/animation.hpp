#pragma once

#include <cstddef>
#include <vector>

namespace engine::game::components {

struct Animation {
    struct IntRect {
        int x{};
        int y{};
        int w{};
        int h{};
    };

    std::vector<IntRect> frames{};
    float frame_duration_seconds{0.1f};
    float accumulator_seconds{0.0f};
    std::size_t frame_index{0};
    bool loop{true};
    bool playing{true};

    void reset() {
        accumulator_seconds = 0.0f;
        frame_index = 0;
    }

    const IntRect* current_frame() const {
        if (frames.empty()) {
            return nullptr;
        }
        return &frames[frame_index % frames.size()];
    }
};

}  // namespace engine::game::components
