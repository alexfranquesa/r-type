#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include <SFML/Graphics/Texture.hpp>

#include "engine/render/texture_loader.hpp"

namespace engine::render {

/**
 * SpriteBank maps sprite ids to filesystem paths and preloads them via TextureLoader.
 * Typical usage (client side):
 *   SpriteBank bank{"sprites"};
 *   bank.register_directory(".", true);  // register every image in sprites/
 *   bank.preload_all();
 *   auto tex = bank.get("r-typesheet1");
 */
class SpriteBank {
public:
  explicit SpriteBank(std::filesystem::path root = {});

  // Change the root directory used to resolve relative paths.
  void set_root(std::filesystem::path root);

  // Register a sprite id to a relative path (under root).
  void register_sprite(const std::string& id, const std::filesystem::path& relative_path);

  // Register all image files in a directory (optionally recursive). id = filename stem.
  void register_directory(const std::filesystem::path& directory, bool recursive = false);

  // Preload a single sprite by id. Returns false if not registered or failed to load.
  bool preload(const std::string& id);

  // Preload all registered sprites. Returns number of successful loads.
  std::size_t preload_all();

  // Get a texture already loaded; returns fallback if missing/not loaded.
  std::shared_ptr<sf::Texture> get(const std::string& id) const;

  void set_fallback(std::shared_ptr<sf::Texture> texture);
  std::shared_ptr<sf::Texture> fallback() const;

private:
  TextureLoader loader_;
  std::unordered_map<std::string, std::filesystem::path> registry_;
};

}  // namespace engine::render
