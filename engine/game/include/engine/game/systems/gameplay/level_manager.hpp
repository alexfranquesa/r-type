/**
 * @file level_manager.hpp
 * @brief Level progression system for R-Type
 * 
 * Manages level transitions, enemy configurations per level,
 * and background theme changes.
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace engine::game {
    struct GameSettings;
}

namespace rtype::ecs {
    class registry;
}

namespace rtype::game {

/**
 * @brief Configuration for a single level
 */
struct LevelConfig {
    std::uint16_t level_number{1};
    std::uint16_t kills_required{15};     // Kills needed to complete this level
    float enemy_speed_multiplier{1.0f};   // Enemy speed modifier
    float enemy_hp_multiplier{1.0f};      // Enemy HP modifier
    float spawn_rate_multiplier{1.0f};    // Spawn rate modifier
    std::string enemy_sprite_id{"enemy_basic"};  // Which enemy sprite to use
    std::string background_theme{"space"};       // Background theme identifier
    
    // Background theme colors (RGB values 0-255)
    std::uint8_t bg_color_r{0};
    std::uint8_t bg_color_g{0};
    std::uint8_t bg_color_b{20};   // Default: dark blue space
    
    // Star colors for this level
    std::uint8_t star_color_r{255};
    std::uint8_t star_color_g{255};
    std::uint8_t star_color_b{255};  // Default: white stars
};

/**
 * @brief Manages level progression and configuration
 * 
 * The LevelManager tracks player progress through levels,
 * provides level-specific configurations, and handles
 * transitions between levels.
 */
class LevelManager {
public:
    LevelManager();
    ~LevelManager() = default;

    /**
     * @brief Initialize default level configurations
     */
    void initialize();

    /**
     * @brief Get current level number (1-indexed)
     */
    std::uint16_t currentLevel() const { return current_level_; }

    /**
     * @brief Get kills in current level
     */
    std::uint16_t killsThisLevel() const { return kills_this_level_; }

    /**
     * @brief Get kills required to complete current level
     */
    std::uint16_t killsRequired() const;

    /**
     * @brief Get remaining kills to next level
     */
    std::uint16_t killsRemaining() const;

    /**
     * @brief Get total kills across all levels
     */
    std::uint16_t totalKills() const { return total_kills_; }

    /**
     * @brief Register a kill and check for level advancement
     * @return true if advanced to next level
     */
    bool registerKill();

    /**
     * @brief Get the configuration for current level
     */
    const LevelConfig& getCurrentLevelConfig() const;

    /**
     * @brief Get configuration for a specific level
     * @param level Level number (1-indexed)
     */
    const LevelConfig& getLevelConfig(std::uint16_t level) const;

    /**
     * @brief Check if current level was just completed
     */
    bool justAdvanced() const { return just_advanced_; }

    /**
     * @brief Clear the just advanced flag
     */
    void clearAdvancedFlag() { just_advanced_ = false; }

    /**
     * @brief Get total number of levels
     */
    std::size_t totalLevels() const { return level_configs_.size(); }

    /**
     * @brief Reset to level 1
     */
    void reset();

    /**
     * @brief Add a custom level configuration
     */
    void addLevelConfig(const LevelConfig& config);

private:
    std::vector<LevelConfig> level_configs_;
    std::uint16_t current_level_{1};
    std::uint16_t kills_this_level_{0};
    std::uint16_t total_kills_{0};
    bool just_advanced_{false};

    // Default level config for out-of-bounds access
    static const LevelConfig default_config_;
};

}  // namespace rtype::game
