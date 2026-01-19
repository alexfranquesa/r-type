#include "starfield_system.hpp"

#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/sprite.hpp"
#include <cmath>
#include <set>

namespace client::systems {

StarfieldSystem::StarfieldSystem(rtype::ecs::registry& registry,
                                 unsigned width,
                                 unsigned height,
                                 std::size_t star_count)
    : width_(width)
    , height_(height)
    , rng_(std::random_device{}())
    , x_dist_(0.0f, static_cast<float>(width_))
    , y_dist_(0.0f, static_cast<float>(height_))
    , speed_dist_(20.0f, 90.0f)
    , scale_dist_(1.0f, 2.4f)
    , current_level_(1) {
  initializeThemes();
  current_theme_ = level_themes_[0];
  
  stars_.reserve(star_count);
  for (std::size_t i = 0; i < star_count; ++i) {
    const auto entity = registry.spawn_entity();
    registry.emplace_component<engine::game::components::Position>(entity, 0.0f, 0.0f);

    engine::game::components::Sprite sprite{};
    sprite.texture_id = "star";
    sprite.texture_rect = {0, 0, 1, 1};
    sprite.scale_x = 1.0f;
    sprite.scale_y = 1.0f;
    sprite.z_index = 0.1f;
    sprite.visible = true;
    registry.add_component(entity, std::move(sprite));

    const float base_scale = scale_dist_(rng_);
    Star star{entity, speed_dist_(rng_), base_scale};
    respawn_star(registry, star, true);
    stars_.push_back(star);
  }
}

void StarfieldSystem::initializeThemes() {
  level_themes_.clear();
  
  // Level 1: Classic white stars in deep space
  level_themes_.push_back(StarfieldTheme{
    .star_r = 255,
    .star_g = 255,
    .star_b = 255,
    .speed_multiplier = 1.0f,
    .size_multiplier = 1.0f,
    .primary_particle = ParticleType::Star,
    .has_embers = false,
    .has_ash = false,
    .has_debris = false,
    .ember_spawn_rate = 0.0f,
    .debris_spawn_rate = 0.0f,
    .background_r = 0.0f,
    .background_g = 0.0f,
    .background_b = 0.08f  // Dark blue
  });
  
  // Level 2: VOLCANIC PLANET - Red/orange with embers and ash
  level_themes_.push_back(StarfieldTheme{
    .star_r = 255,
    .star_g = 100,
    .star_b = 50,
    .speed_multiplier = 0.6f,   // Slower stars (we're on a planet)
    .size_multiplier = 2.0f,    // Larger, more like distant lava glow
    .primary_particle = ParticleType::Ember,
    .has_embers = true,
    .has_ash = true,
    .has_debris = false,
    .ember_spawn_rate = 15.0f,  // 15 embers per second
    .debris_spawn_rate = 0.0f,
    .background_r = 0.15f,      // Dark red/brown volcanic
    .background_g = 0.03f,
    .background_b = 0.0f,
    // Background image: volcanic background
    .background_image = "client/assets/backgrounds/volcanic_bg.png",
    .parallax_speed = 0.3f
  });
  
  // Level 3: Asteroid Field - Space debris with rocky colors
  level_themes_.push_back(StarfieldTheme{
    .star_r = 180,
    .star_g = 160,
    .star_b = 140,
    .speed_multiplier = 1.5f,       // Faster movement through field
    .size_multiplier = 1.8f,        // Larger debris particles
    .primary_particle = ParticleType::Debris,
    .has_embers = false,
    .has_ash = false,
    .has_debris = true,             // Enable space debris particles
    .ember_spawn_rate = 0.0f,
    .debris_spawn_rate = 8.0f,      // 8 debris particles per second
    .background_r = 0.02f,          // Very dark space
    .background_g = 0.02f,
    .background_b = 0.04f,
    // Background image: asteroid field background
    .background_image = "client/assets/backgrounds/asteroid_bg.png",
    .parallax_speed = 0.5f
  });
  
  // Level 4: Ice blue cold space with frozen background
  level_themes_.push_back(StarfieldTheme{
    .star_r = 150,
    .star_g = 220,
    .star_b = 255,
    .speed_multiplier = 0.8f,
    .size_multiplier = 1.2f,
    .primary_particle = ParticleType::Star,
    .has_embers = false,
    .has_ash = false,
    .has_debris = false,
    .ember_spawn_rate = 0.0f,
    .debris_spawn_rate = 0.0f,
    .background_r = 0.0f,
    .background_g = 0.05f,
    .background_b = 0.15f,       // Deeper blue for ice
    // Background image: frozen ice background
    .background_image = "client/assets/backgrounds/Ice_bg.png",
    .parallax_speed = 0.4f
  });
  
  // Level 5: BOSS STAGE - Final battle with dark menacing background
  level_themes_.push_back(StarfieldTheme{
    .star_r = 255,
    .star_g = 50,
    .star_b = 50,
    .speed_multiplier = 0.3f,
    .size_multiplier = 2.0f,
    .primary_particle = ParticleType::Star,
    .has_embers = true,
    .has_ash = false,
    .has_debris = false,
    .ember_spawn_rate = 5.0f,
    .debris_spawn_rate = 0.0f,
    .background_r = 0.05f,
    .background_g = 0.0f,
    .background_b = 0.05f,
    // Boss background
    .background_image = "client/assets/backgrounds/boss_bg.png",
    .parallax_speed = 0.0f  // No scrolling for boss fight
  });
  
  // Level 6: Victory/Endless - Dark red pulsating
  level_themes_.push_back(StarfieldTheme{
    .star_r = 255,
    .star_g = 50,
    .star_b = 50,
    .speed_multiplier = 0.5f,
    .size_multiplier = 2.5f,
    .primary_particle = ParticleType::Star,
    .has_embers = true,
    .has_ash = false,
    .has_debris = false,
    .ember_spawn_rate = 8.0f,
    .debris_spawn_rate = 0.0f,
    .background_r = 0.05f,
    .background_g = 0.0f,
    .background_b = 0.05f
  });
}

void StarfieldSystem::setLevel(std::uint16_t level) {
  if (level == current_level_) {
    return;
  }
  
  current_level_ = level;
  
  // Get theme for this level (0-indexed, clamp to available themes)
  std::size_t theme_idx = (level > 0 && level <= level_themes_.size()) 
    ? (level - 1) 
    : (level_themes_.empty() ? 0 : level_themes_.size() - 1);
  
  if (theme_idx < level_themes_.size()) {
    current_theme_ = level_themes_[theme_idx];
    theme_changed_ = true;  // Apply on next update
  }
}

void StarfieldSystem::applyThemeToStars(rtype::ecs::registry& registry) {
  for (auto& star : stars_) {
    auto* sprite = registry.try_get<engine::game::components::Sprite>(star.entity);
    if (!sprite) {
      continue;
    }
    
    // Apply theme-based scaling
    const float themed_scale = star.base_scale * current_theme_.size_multiplier;
    sprite->scale_x = themed_scale;
    sprite->scale_y = themed_scale;
    
    // Apply theme colors
    sprite->color_r = current_theme_.star_r;
    sprite->color_g = current_theme_.star_g;
    sprite->color_b = current_theme_.star_b;
  }
}

void StarfieldSystem::resize(unsigned width, unsigned height) {
  width_ = width;
  height_ = height;
  x_dist_ = std::uniform_real_distribution<float>(0.0f, static_cast<float>(width_));
  y_dist_ = std::uniform_real_distribution<float>(0.0f, static_cast<float>(height_));
}

void StarfieldSystem::reinitialize(rtype::ecs::registry& registry) {
  // Clear old star references and embers
  stars_.clear();
  clearEmbers(registry);
  current_level_ = 1;
  ember_spawn_timer_ = 0.0f;
  if (!level_themes_.empty()) {
    current_theme_ = level_themes_[0];
  }
  
  // Recreate stars (same logic as constructor)
  const std::size_t star_count = 120;
  stars_.reserve(star_count);
  for (std::size_t i = 0; i < star_count; ++i) {
    const auto entity = registry.spawn_entity();
    registry.emplace_component<engine::game::components::Position>(entity, 0.0f, 0.0f);

    engine::game::components::Sprite sprite{};
    sprite.texture_id = "star";
    sprite.texture_rect = {0, 0, 1, 1};
    sprite.scale_x = 1.0f;
    sprite.scale_y = 1.0f;
    sprite.z_index = 0.1f;
    sprite.visible = true;
    registry.add_component(entity, std::move(sprite));

    const float base_scale = scale_dist_(rng_);
    Star star{entity, speed_dist_(rng_), base_scale};
    respawn_star(registry, star, true);
    stars_.push_back(star);
  }
}

void StarfieldSystem::update(rtype::ecs::registry& registry, float dt) {
  if (width_ == 0 || height_ == 0) {
    return;
  }
  
  // Apply theme changes if level changed
  if (theme_changed_) {
    applyThemeToStars(registry);
    // Initialize embers if theme has them
    if (current_theme_.has_embers && embers_.empty()) {
      initializeEmbers(registry);
    } else if (!current_theme_.has_embers && !embers_.empty()) {
      clearEmbers(registry);
    }
    theme_changed_ = false;
  }

  const float speed_mult = current_theme_.speed_multiplier;

  // Update stars
  for (auto& star : stars_) {
    auto* pos = registry.try_get<engine::game::components::Position>(star.entity);
    if (!pos) {
      continue;
    }

    pos->x -= star.speed * speed_mult * dt;
    if (pos->x < -2.0f) {
      respawn_star(registry, star, false);
    }
  }
  
  // Update embers if theme has them
  if (current_theme_.has_embers) {
    updateEmbers(registry, dt);
  }
}

void StarfieldSystem::respawn_star(rtype::ecs::registry& registry, Star& star, bool random_x) {
  auto* pos = registry.try_get<engine::game::components::Position>(star.entity);
  auto* sprite = registry.try_get<engine::game::components::Sprite>(star.entity);
  if (!pos || !sprite) {
    return;
  }

  star.base_scale = scale_dist_(rng_);
  star.speed = speed_dist_(rng_);
  pos->y = y_dist_(rng_);
  
  const float themed_scale = star.base_scale * current_theme_.size_multiplier;
  pos->x = random_x ? x_dist_(rng_) : static_cast<float>(width_) + themed_scale;

  sprite->texture_id = "star";
  sprite->texture_rect = {0, 0, 1, 1};
  sprite->scale_x = themed_scale;
  sprite->scale_y = themed_scale;
  sprite->z_index = 0.1f;
  sprite->visible = true;
  
  // Apply star color from theme
  sprite->color_r = current_theme_.star_r;
  sprite->color_g = current_theme_.star_g;
  sprite->color_b = current_theme_.star_b;
  sprite->color_a = 255;
}

sf::Color StarfieldSystem::getBackgroundColor() const {
  return sf::Color(
    static_cast<std::uint8_t>(current_theme_.background_r * 255.0f),
    static_cast<std::uint8_t>(current_theme_.background_g * 255.0f),
    static_cast<std::uint8_t>(current_theme_.background_b * 255.0f)
  );
}

void StarfieldSystem::initializeEmbers(rtype::ecs::registry& registry) {
  embers_.clear();
  embers_.reserve(MAX_EMBERS);
  
  // Create initial ember particles
  std::uniform_real_distribution<float> lifetime_dist(2.0f, 5.0f);
  
  for (std::size_t i = 0; i < MAX_EMBERS / 2; ++i) {
    const auto entity = registry.spawn_entity();
    registry.emplace_component<engine::game::components::Position>(entity, 0.0f, 0.0f);

    engine::game::components::Sprite sprite{};
    sprite.texture_id = "star";  // Use star texture, tint it orange/red
    sprite.texture_rect = {0, 0, 1, 1};
    sprite.scale_x = 3.0f;
    sprite.scale_y = 3.0f;
    sprite.z_index = 0.15f;  // Slightly above stars
    sprite.visible = true;
    sprite.color_r = 255;
    sprite.color_g = static_cast<std::uint8_t>(std::uniform_int_distribution<int>(80, 180)(rng_));
    sprite.color_b = 0;
    sprite.color_a = 200;
    registry.add_component(entity, std::move(sprite));

    EmberParticle ember;
    ember.entity = entity;
    ember.max_lifetime = lifetime_dist(rng_);
    ember.lifetime = std::uniform_real_distribution<float>(0.0f, ember.max_lifetime)(rng_);
    respawnEmber(registry, ember);
    embers_.push_back(ember);
  }
}

void StarfieldSystem::updateEmbers(rtype::ecs::registry& registry, float dt) {
  std::uniform_real_distribution<float> lifetime_dist(2.0f, 5.0f);
  
  for (auto& ember : embers_) {
    auto* pos = registry.try_get<engine::game::components::Position>(ember.entity);
    auto* sprite = registry.try_get<engine::game::components::Sprite>(ember.entity);
    if (!pos || !sprite) {
      continue;
    }
    
    // Update position (falling with horizontal drift)
    pos->x += ember.speed_x * dt;
    pos->y += ember.speed_y * dt;
    
    // Update lifetime
    ember.lifetime += dt;
    ember.flicker_phase += dt * 8.0f;  // Fast flicker
    
    // Pulsing/flickering effect
    float flicker = 0.7f + 0.3f * std::sin(ember.flicker_phase);
    sprite->color_a = static_cast<std::uint8_t>(200.0f * flicker);
    
    // Respawn if off-screen or lifetime expired
    if (pos->y > static_cast<float>(height_) + 20.0f || 
        pos->x < -20.0f || 
        pos->x > static_cast<float>(width_) + 20.0f ||
        ember.lifetime >= ember.max_lifetime) {
      ember.max_lifetime = lifetime_dist(rng_);
      respawnEmber(registry, ember);
    }
  }
  
  // Spawn new embers based on spawn rate
  if (embers_.size() < MAX_EMBERS && current_theme_.ember_spawn_rate > 0.0f) {
    ember_spawn_timer_ += dt;
    float spawn_interval = 1.0f / current_theme_.ember_spawn_rate;
    
    while (ember_spawn_timer_ >= spawn_interval && embers_.size() < MAX_EMBERS) {
      ember_spawn_timer_ -= spawn_interval;
      
      const auto entity = registry.spawn_entity();
      registry.emplace_component<engine::game::components::Position>(entity, 0.0f, 0.0f);

      engine::game::components::Sprite sprite{};
      sprite.texture_id = "star";
      sprite.texture_rect = {0, 0, 1, 1};
      sprite.scale_x = 3.0f;
      sprite.scale_y = 3.0f;
      sprite.z_index = 0.15f;
      sprite.visible = true;
      sprite.color_r = 255;
      sprite.color_g = static_cast<std::uint8_t>(std::uniform_int_distribution<int>(80, 180)(rng_));
      sprite.color_b = 0;
      sprite.color_a = 200;
      registry.add_component(entity, std::move(sprite));

      EmberParticle ember;
      ember.entity = entity;
      ember.max_lifetime = lifetime_dist(rng_);
      ember.lifetime = 0.0f;
      respawnEmber(registry, ember);
      embers_.push_back(ember);
    }
  }
}

void StarfieldSystem::respawnEmber(rtype::ecs::registry& registry, EmberParticle& ember) {
  auto* pos = registry.try_get<engine::game::components::Position>(ember.entity);
  auto* sprite = registry.try_get<engine::game::components::Sprite>(ember.entity);
  if (!pos || !sprite) {
    return;
  }
  
  // Spawn at top of screen with random X
  pos->x = x_dist_(rng_);
  pos->y = std::uniform_real_distribution<float>(-50.0f, -10.0f)(rng_);
  
  // Random fall speed and horizontal drift
  ember.speed_y = std::uniform_real_distribution<float>(80.0f, 200.0f)(rng_);
  ember.speed_x = std::uniform_real_distribution<float>(-30.0f, 30.0f)(rng_);
  ember.flicker_phase = std::uniform_real_distribution<float>(0.0f, 6.28f)(rng_);
  ember.lifetime = 0.0f;
  
  // Random size
  float scale = std::uniform_real_distribution<float>(2.0f, 5.0f)(rng_);
  sprite->scale_x = scale;
  sprite->scale_y = scale;
  
  // Random orange-red color
  sprite->color_r = 255;
  sprite->color_g = static_cast<std::uint8_t>(std::uniform_int_distribution<int>(60, 160)(rng_));
  sprite->color_b = static_cast<std::uint8_t>(std::uniform_int_distribution<int>(0, 40)(rng_));
  sprite->color_a = 200;
}

void StarfieldSystem::clearEmbers(rtype::ecs::registry& registry) {
  for (auto& ember : embers_) {
    registry.kill_entity(ember.entity);
  }
  embers_.clear();
  ember_spawn_timer_ = 0.0f;
}

bool StarfieldSystem::hasBackgroundImage() const {
  return !current_theme_.background_image.empty();
}

void StarfieldSystem::loadBackgroundTexture(const std::string& path) {
  if (path.empty() || path == current_bg_image_) {
    return;  // Nothing to load or already loaded
  }
  
  // Check if we already tried and failed to load this path
  static std::set<std::string> failed_paths;
  if (failed_paths.count(path) > 0) {
    return;  // Don't spam errors
  }
  
  // Check cache first
  auto it = background_textures_.find(path);
  if (it != background_textures_.end()) {
    // Create sprite from cached texture
    background_sprite_.emplace(it->second);
    current_bg_image_ = path;
  } else {
    // Load new texture
    sf::Texture texture;
    if (texture.loadFromFile(path)) {
      texture.setRepeated(false);  // Don't repeat - we'll scale to fit
      background_textures_[path] = std::move(texture);
      // Create sprite from newly loaded texture
      background_sprite_.emplace(background_textures_[path]);
      current_bg_image_ = path;
    } else {
      // Mark as failed so we don't spam
      failed_paths.insert(path);
    }
  }
  
  // Scale sprite to fill the entire screen (cover mode)
  if (!current_bg_image_.empty() && background_sprite_.has_value()) {
    const auto& tex = background_sprite_->getTexture();
    const float tex_height = static_cast<float>(tex.getSize().y);
    const float tex_width = static_cast<float>(tex.getSize().x);
    
    // Calculate scale to cover the entire screen (may crop edges)
    const float scale_x = static_cast<float>(width_) / tex_width;
    const float scale_y = static_cast<float>(height_) / tex_height;
    const float scale = std::max(scale_x, scale_y);  // Cover mode: use larger scale
    
    background_sprite_->setScale({scale, scale});
    
    // Center the image if it's larger than the screen
    const float scaled_width = tex_width * scale;
    const float scaled_height = tex_height * scale;
    const float offset_x = (static_cast<float>(width_) - scaled_width) / 2.0f;
    const float offset_y = (static_cast<float>(height_) - scaled_height) / 2.0f;
    background_sprite_->setPosition({offset_x, offset_y});
    
    // Reset texture rect to full image
    background_sprite_->setTextureRect(sf::IntRect({0, 0}, {static_cast<int>(tex_width), static_cast<int>(tex_height)}));
  }
}

void StarfieldSystem::drawBackground(sf::RenderWindow& window) {
  if (!hasBackgroundImage()) {
    return;
  }
  
  // Load texture if needed
  if (current_bg_image_ != current_theme_.background_image) {
    loadBackgroundTexture(current_theme_.background_image);
  }
  
  if (current_bg_image_.empty() || !background_sprite_.has_value()) {
    return;  // Failed to load
  }
  
  // Draw the static background (no parallax scrolling to avoid edge issues)
  window.draw(*background_sprite_);
}

}  // namespace client::systems
