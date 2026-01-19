#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "engine/core/registry.hpp"

namespace client::systems {

/**
 * @brief Particle type for different level themes
 */
enum class ParticleType {
    Star,       // Classic white/colored stars (space levels)
    Ember,      // Red/orange falling embers (volcanic levels)
    Ash,        // Gray falling ash particles (volcanic levels)
    Debris      // Space debris particles (asteroid field levels)
};

/**
 * @brief Theme configuration for starfield per level
 */
struct StarfieldTheme {
    std::uint8_t star_r{255};
    std::uint8_t star_g{255};
    std::uint8_t star_b{255};
    float speed_multiplier{1.0f};
    float size_multiplier{1.0f};
    
    // Particle configuration
    ParticleType primary_particle{ParticleType::Star};
    bool has_embers{false};           // Add falling ember particles
    bool has_ash{false};              // Add falling ash particles
    bool has_debris{false};           // Add space debris particles
    float ember_spawn_rate{0.0f};     // Embers per second
    float debris_spawn_rate{0.0f};    // Debris per second
    float background_r{0.0f};         // Background color hint (0-1)
    float background_g{0.0f};
    float background_b{0.0f};
    
    // Background image (optional - empty string means no image)
    std::string background_image;     // Path to background texture
    float parallax_speed{0.5f};       // Parallax scroll speed multiplier
};

class StarfieldSystem {
 public:
  StarfieldSystem(rtype::ecs::registry& registry,
                  unsigned width,
                  unsigned height,
                  std::size_t star_count = 120);

  void resize(unsigned width, unsigned height);
  void update(rtype::ecs::registry& registry, float dt);
  void reinitialize(rtype::ecs::registry& registry);  // Recreate stars after registry clear
  
  /**
   * @brief Set the current level to change starfield theme
   * @param level Level number (1-indexed)
   */
  void setLevel(std::uint16_t level);
  
  /**
   * @brief Get the current theme
   */
  const StarfieldTheme& currentTheme() const { return current_theme_; }
  
  /**
   * @brief Get background color for current theme
   */
  sf::Color getBackgroundColor() const;
  
  /**
   * @brief Draw background image if one is set for current level
   * @param window The render window
   * @param dt Delta time for parallax scrolling
   */
  void drawBackground(sf::RenderWindow& window);
  
  /**
   * @brief Check if current theme has a background image
   */
  bool hasBackgroundImage() const;

 private:
  struct Star {
    rtype::ecs::entity_t entity;
    float speed;
    float base_scale;  // Store base scale for theme changes
  };
  
  struct EmberParticle {
    rtype::ecs::entity_t entity;
    float speed_x;      // Horizontal drift
    float speed_y;      // Vertical fall speed
    float flicker_phase; // For pulsing effect
    float lifetime;      // Current lifetime
    float max_lifetime;  // When to respawn
  };

  void respawn_star(rtype::ecs::registry& registry, Star& star, bool random_x);
  void initializeThemes();
  void applyThemeToStars(rtype::ecs::registry& registry);
  void loadBackgroundTexture(const std::string& path);
  
  // Ember system
  void initializeEmbers(rtype::ecs::registry& registry);
  void updateEmbers(rtype::ecs::registry& registry, float dt);
  void respawnEmber(rtype::ecs::registry& registry, EmberParticle& ember);
  void clearEmbers(rtype::ecs::registry& registry);

  unsigned width_;
  unsigned height_;
  std::vector<Star> stars_;
  std::vector<EmberParticle> embers_;
  std::mt19937 rng_;
  std::uniform_real_distribution<float> x_dist_;
  std::uniform_real_distribution<float> y_dist_;
  std::uniform_real_distribution<float> speed_dist_;
  std::uniform_real_distribution<float> scale_dist_;
  
  std::uint16_t current_level_{1};
  StarfieldTheme current_theme_;
  std::vector<StarfieldTheme> level_themes_;
  bool theme_changed_{false};  // Flag to apply theme on next update
  
  // Ember configuration
  static constexpr std::size_t MAX_EMBERS = 40;
  float ember_spawn_timer_{0.0f};
  
  // Background image system
  std::map<std::string, sf::Texture> background_textures_;  // Cache loaded textures
  std::optional<sf::Sprite> background_sprite_;  // Optional sprite (SFML 3 requires texture in constructor)
  float parallax_offset_{0.0f};  // Current horizontal scroll offset
  std::string current_bg_image_;  // Currently loaded background
};

}  // namespace client::systems
