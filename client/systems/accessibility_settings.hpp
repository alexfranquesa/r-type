#pragma once

#include <cstddef>
#include <filesystem>
#include <string>

namespace client::systems {

/**
 * Manages accessibility settings with persistence to file.
 * Settings include contrast mode, font scaling, and key remapping.
 */
class AccessibilitySettings {
public:
  AccessibilitySettings() = default;

  // Load settings from file (returns false if file doesn't exist or is invalid)
  bool load(const std::filesystem::path& config_path = "config/accessibility.cfg");

  // Save current settings to file
  bool save(const std::filesystem::path& config_path = "config/accessibility.cfg") const;

  // Getters
  bool high_contrast() const { return high_contrast_; }
  std::size_t font_scale_index() const { return font_scale_idx_; }
  float font_scale() const;

  // Setters
  void set_high_contrast(bool enabled) { high_contrast_ = enabled; }
  void set_font_scale_index(std::size_t idx);
  void cycle_font_scale();

  // Available font scales
  static constexpr std::size_t font_scale_count = 3;
  static constexpr float font_scales[font_scale_count] = {1.0F, 1.25F, 1.5F};

private:
  bool high_contrast_{false};
  std::size_t font_scale_idx_{0};
};

}  // namespace client::systems
