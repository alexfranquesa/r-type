#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace rtype::ecs { class registry; }

#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/sprite.hpp"
#include "engine/render/sprite_bank.hpp"

namespace client::systems {

/**
 * Client-side render system helper that owns the SpriteBank and preloads textures.
 * It also knows how to draw all entities that have Position + Sprite components.
 */
class RenderSystem {
public:
  /// Construct the render system with a root directory for sprite assets.
  explicit RenderSystem(std::filesystem::path sprites_root = "sprites");

  /// Preload every registered sprite (by default all files in sprites_root).
  void preload_all();

  /// Retrieve a texture for a given sprite id; returns fallback if missing.
  std::shared_ptr<sf::Texture> texture_for(const std::string& id) const;

  /// Access to the underlying sprite bank for advanced use.
  engine::render::SpriteBank& sprite_bank();

  /// Draw all entities with Position + Sprite as colored boxes (MVP debug view).
  /// Returns how many entities were drawn.
  std::size_t draw(rtype::ecs::registry& registry, sf::RenderWindow& window);

private:
  /// Initialize a 1x1 white pixel fallback texture.
  void init_fallback();

  engine::render::SpriteBank sprite_bank_;
};

}  // namespace client::systems
