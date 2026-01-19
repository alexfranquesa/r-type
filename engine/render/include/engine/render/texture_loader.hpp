#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include <SFML/Graphics/Texture.hpp>

namespace engine::render {

/**
 * Simple texture cache on top of SFML.
 * SpriteBank will call this to preload and retrieve textures by id.
 */
class TextureLoader {
public:
  explicit TextureLoader(std::filesystem::path root = {});

  // Load a texture and associate it with a string id. Returns false on failure.
  bool load(const std::string& id, const std::filesystem::path& relative_path);

  // Retrieve a texture; returns nullptr if not found.
  std::shared_ptr<sf::Texture> get(const std::string& id) const;

  // Remove a texture from the cache.
  void unload(const std::string& id);

  // Clear all cached textures.
  void clear();

  // Configure a fallback texture used when a requested id is missing.
  void set_fallback(std::shared_ptr<sf::Texture> texture);

  std::shared_ptr<sf::Texture> fallback() const;
  const std::filesystem::path& root() const { return root_; }

private:
  std::filesystem::path root_;
  std::unordered_map<std::string, std::shared_ptr<sf::Texture>> cache_;
  std::shared_ptr<sf::Texture> fallback_;
};

}  // namespace engine::render
