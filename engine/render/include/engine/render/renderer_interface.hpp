#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace engine::render {

struct SpriteView {
    std::string texture_id;
    int rect_left{0};
    int rect_top{0};
    int rect_width{0};
    int rect_height{0};
    float x{0.f};
    float y{0.f};
    float scale_x{1.f};
    float scale_y{1.f};
    float origin_x{0.f};
    float origin_y{0.f};
    bool flip_x{false};
    bool flip_y{false};
    std::uint8_t color_r{255};
    std::uint8_t color_g{255};
    std::uint8_t color_b{255};
    std::uint8_t color_a{255};
    float z_index{0.f};
};

class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual void begin() = 0;
    virtual void draw(const SpriteView& sprite) = 0;
    virtual void draw_batch(const std::vector<SpriteView>& sprites) = 0;
    virtual void end() = 0;
};

}  // namespace engine::render

