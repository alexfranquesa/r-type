#include "engine/render/sprite_bank.hpp"

#include <exception>
#include <filesystem>
#include <utility>

namespace engine::render {

namespace {
bool is_supported_extension(const std::filesystem::path& path) {
  const auto ext = path.extension().string();
  return ext == ".png" || ext == ".jpg" || ext == ".jpeg";
}
}  // namespace

SpriteBank::SpriteBank(std::filesystem::path root) : loader_(std::move(root)) {}

void SpriteBank::set_root(std::filesystem::path root) {
  loader_ = TextureLoader(std::move(root));
}

void SpriteBank::register_sprite(const std::string& id, const std::filesystem::path& relative_path) {
  registry_[id] = relative_path;
}

void SpriteBank::register_directory(const std::filesystem::path& directory, bool recursive) {
  std::filesystem::path base = loader_.root();
  if (base.empty()) {
    base = directory;
    loader_ = TextureLoader(base);
  }
  std::filesystem::path dir = directory;
  if (dir.is_relative()) {
    dir = base / dir;
  }

  auto handle_entry = [&](const std::filesystem::directory_entry& entry) {
    if (!entry.is_regular_file() || !is_supported_extension(entry.path())) {
      return;
    }
    const auto stem = entry.path().stem().string();
    std::filesystem::path rel = entry.path().filename();
    try {
      rel = std::filesystem::relative(entry.path(), base);
    } catch (const std::exception&) {
      rel = entry.path().filename();
    }
    registry_[stem] = rel;
  };

  if (recursive) {
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dir)) {
      handle_entry(entry);
    }
  } else {
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
      handle_entry(entry);
    }
  }
}

bool SpriteBank::preload(const std::string& id) {
  const auto it = registry_.find(id);
  if (it == registry_.end()) {
    return false;
  }
  return loader_.load(it->first, it->second);
}

std::size_t SpriteBank::preload_all() {
  std::size_t loaded = 0;
  for (const auto& [id, path] : registry_) {
    if (loader_.load(id, path)) {
      ++loaded;
    }
  }
  return loaded;
}

std::shared_ptr<sf::Texture> SpriteBank::get(const std::string& id) const {
  return loader_.get(id);
}

void SpriteBank::set_fallback(std::shared_ptr<sf::Texture> texture) {
  loader_.set_fallback(std::move(texture));
}

std::shared_ptr<sf::Texture> SpriteBank::fallback() const {
  return loader_.fallback();
}

}  // namespace engine::render
