#include "engine/render/texture_loader.hpp"

#include <SFML/Graphics/Texture.hpp>
#include <utility>

namespace engine::render {

TextureLoader::TextureLoader(std::filesystem::path root) : root_(std::move(root)) {}

bool TextureLoader::load(const std::string& id, const std::filesystem::path& relative_path) {
  const auto full_path = root_.empty() ? relative_path : (root_ / relative_path);

  auto texture = std::make_shared<sf::Texture>();
  if (!texture->loadFromFile(full_path.string())) {
    return false;
  }

  cache_[id] = std::move(texture);
  return true;
}

std::shared_ptr<sf::Texture> TextureLoader::get(const std::string& id) const {
  if (auto it = cache_.find(id); it != cache_.end()) {
    return it->second;
  }
  return fallback_;
}

void TextureLoader::unload(const std::string& id) {
  cache_.erase(id);
}

void TextureLoader::clear() {
  cache_.clear();
}

void TextureLoader::set_fallback(std::shared_ptr<sf::Texture> texture) {
  fallback_ = std::move(texture);
}

std::shared_ptr<sf::Texture> TextureLoader::fallback() const {
  return fallback_;
}

}  // namespace engine::render
