#pragma once

#include <cstdint>
#include <string>

namespace engine::game::components {

/**
 * Sprite component describing how an entity should be rendered.
 * The rendering system will map `texture_id` to an actual texture
 * through the client SpriteBank/TextureLoader.
 */
struct Sprite {
  struct Rect {
    std::int32_t left = 0;
    std::int32_t top = 0;
    std::int32_t width = 0;
    std::int32_t height = 0;
  };

  std::string texture_id{};  ///< Identifier resolved by SpriteBank.
  Rect texture_rect{};       ///< Part of the texture atlas to render.
  float scale_x = 1.0F;
  float scale_y = 1.0F;
  float origin_x = 0.0F;     ///< Pixel origin inside texture_rect.
  float origin_y = 0.0F;
  float z_index = 0.0F;      ///< Higher z draws on top.
  bool visible = true;
  bool flip_x = false;
  bool flip_y = false;
  
  // Color tint (RGBA, 0-255)
  std::uint8_t color_r = 255;
  std::uint8_t color_g = 255;
  std::uint8_t color_b = 255;
  std::uint8_t color_a = 255;
};

}  // namespace engine::game::component
