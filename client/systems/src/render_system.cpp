#include "render_system.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Image.hpp>
#include "engine/core/registry.hpp"

namespace client::systems {

using engine::game::components::Position;
using engine::game::components::Sprite;

RenderSystem::RenderSystem(std::filesystem::path sprites_root)
    : sprite_bank_(std::move(sprites_root)) {
  init_fallback();
  sprite_bank_.register_directory(".", true);
}

void RenderSystem::preload_all() {
  const auto count = sprite_bank_.preload_all();
  std::cout << "[render] preloaded " << count << " sprites" << std::endl;
}

std::shared_ptr<sf::Texture> RenderSystem::texture_for(const std::string& id) const {
  return sprite_bank_.get(id);
}

engine::render::SpriteBank& RenderSystem::sprite_bank() {
  return sprite_bank_;
}

void RenderSystem::init_fallback() {
  auto tex = std::make_shared<sf::Texture>();
  sf::Image img;
  img.resize({1U, 1U});
  img.setPixel({0U, 0U}, sf::Color::White);
  if (tex->loadFromImage(img)) {
    sprite_bank_.set_fallback(tex);
  }
}

std::size_t RenderSystem::draw(rtype::ecs::registry& registry, sf::RenderWindow& window) {
  auto& positions = registry.get_components<Position>();
  auto& sprites   = registry.get_components<Sprite>();

  struct DrawCommand {
    float z_index;
    std::size_t index;
  };

  std::vector<DrawCommand> commands;
  const std::size_t size = std::min(positions.size(), sprites.size());
  commands.reserve(size);

  for (std::size_t i = 0; i < size; ++i) {
    if (!positions[i].has_value() || !sprites[i].has_value())
      continue;

    const auto& spr = sprites[i].value();
    if (!spr.visible)
      continue;

    commands.push_back(DrawCommand{
        .z_index = spr.z_index,
        .index   = i,
    });
  }

  std::sort(commands.begin(), commands.end(),
            [](const DrawCommand& a, const DrawCommand& b) {
              return a.z_index < b.z_index;
            });

  std::size_t drawn = 0;
  for (const auto& cmd : commands) {
    const auto& pos = positions[cmd.index].value();
    const auto& spr = sprites[cmd.index].value();

    auto tex = sprite_bank_.get(spr.texture_id);
    if (!tex) tex = sprite_bank_.fallback();
    if (!tex) continue;

    sf::Sprite drawable(*tex);

    engine::game::components::Sprite::Rect rect = spr.texture_rect;
    bool anim_flip_x = spr.flip_x;

    if (rect.width > 0 && rect.height > 0) {
      drawable.setTextureRect(sf::IntRect{{rect.left, rect.top}, {rect.width, rect.height}});
    }

    const int rect_w = (spr.texture_rect.width > 0) ? spr.texture_rect.width : static_cast<int>(tex->getSize().x);
    const int rect_h = (spr.texture_rect.height > 0) ? spr.texture_rect.height : static_cast<int>(tex->getSize().y);
    sf::Vector2f origin{spr.origin_x, spr.origin_y};
    if (anim_flip_x) origin.x = static_cast<float>(rect_w) - origin.x;
    if (spr.flip_y) origin.y = static_cast<float>(rect_h) - origin.y;
    drawable.setOrigin(origin);

    drawable.setPosition(sf::Vector2f{pos.x, pos.y});
    drawable.setScale(sf::Vector2f{
        spr.scale_x * (anim_flip_x ? -1.0f : 1.0f),
        spr.scale_y * (spr.flip_y ? -1.0f : 1.0f)});
    
    // Apply color tint
    drawable.setColor(sf::Color(spr.color_r, spr.color_g, spr.color_b, spr.color_a));

    window.draw(drawable);
    ++drawn;
  }

  return drawn;
}

}  // namespace client::systems
