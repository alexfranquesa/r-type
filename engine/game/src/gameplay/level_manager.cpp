/**
 * @file level_manager.cpp
 * @brief Implementation of level progression system
 */

#include "engine/game/systems/gameplay/level_manager.hpp"
#include <iostream>

namespace rtype::game {

// Default config for invalid level access
const LevelConfig LevelManager::default_config_{};

LevelManager::LevelManager() {
    initialize();
}

void LevelManager::initialize() {
    level_configs_.clear();
    
    // Level 1: Space - Easy introduction
    // Very slow spawn rate to ease player in, few enemies
    level_configs_.push_back(LevelConfig{
        .level_number = 1,
        .kills_required = 10,
        .enemy_speed_multiplier = 0.8f,      // Slower enemies
        .enemy_hp_multiplier = 1.0f,
        .spawn_rate_multiplier = 0.3f,       // Very slow spawning - 1 enemy every ~3.3 seconds
        .enemy_sprite_id = "enemy_basic",
        .background_theme = "space",
        .bg_color_r = 0,
        .bg_color_g = 0,
        .bg_color_b = 20,
        .star_color_r = 255,
        .star_color_g = 255,
        .star_color_b = 255
    });

    // Level 2: VOLCANIC PLANET - Fire theme with falling lava hazards
    // Tougher enemies, more health, different sprites
    level_configs_.push_back(LevelConfig{
        .level_number = 2,
        .kills_required = 15,
        .enemy_speed_multiplier = 1.0f,
        .enemy_hp_multiplier = 2.0f,         // 2 shots to kill (10 base HP * 2 = 20 HP)
        .spawn_rate_multiplier = 0.6f,       // Moderate spawning
        .enemy_sprite_id = "enemy_volcanic",
        .background_theme = "volcanic",
        .bg_color_r = 40,
        .bg_color_g = 10,
        .bg_color_b = 0,
        .star_color_r = 255,
        .star_color_g = 100,
        .star_color_b = 50
    });

    // Level 3: Asteroid Field - Orange/brown, normal speed
    level_configs_.push_back(LevelConfig{
        .level_number = 3,
        .kills_required = 20,
        .enemy_speed_multiplier = 1.1f,
        .enemy_hp_multiplier = 3.0f,         // 3 shots to kill
        .spawn_rate_multiplier = 1.5f,       // Fast spawning in asteroid field
        .enemy_sprite_id = "enemy_basic",
        .background_theme = "asteroid",
        .bg_color_r = 30,
        .bg_color_g = 15,
        .bg_color_b = 5,
        .star_color_r = 255,
        .star_color_g = 200,
        .star_color_b = 100
    });

    // Level 4: Ice Field - Blue/cyan theme
    level_configs_.push_back(LevelConfig{
        .level_number = 4,
        .kills_required = 15,
        .enemy_speed_multiplier = 1.0f,
        .enemy_hp_multiplier = 1.0f,         // Ice crabs have their own HP (60 base)
        .spawn_rate_multiplier = 1.0f,       // Moderate spawning
        .enemy_sprite_id = "enemy_basic",
        .background_theme = "ice",
        .bg_color_r = 0,
        .bg_color_g = 20,
        .bg_color_b = 40,
        .star_color_r = 150,
        .star_color_g = 220,
        .star_color_b = 255
    });

    // Level 5: BOSS STAGE - Final battle
    // No regular enemy spawning, just the boss
    level_configs_.push_back(LevelConfig{
        .level_number = 5,
        .kills_required = 1,                 // Just kill the boss
        .enemy_speed_multiplier = 1.0f,
        .enemy_hp_multiplier = 1.0f,         // Boss has its own HP
        .spawn_rate_multiplier = 0.0f,       // No regular enemies
        .enemy_sprite_id = "boss",
        .background_theme = "boss",
        .bg_color_r = 10,
        .bg_color_g = 0,
        .bg_color_b = 10,
        .star_color_r = 255,
        .star_color_g = 50,
        .star_color_b = 50
    });

    reset();
}

void LevelManager::reset() {
    current_level_ = 1;
    kills_this_level_ = 0;
    total_kills_ = 0;
    just_advanced_ = false;
}

std::uint16_t LevelManager::killsRequired() const {
    if (current_level_ <= level_configs_.size()) {
        return level_configs_[current_level_ - 1].kills_required;
    }
    return 15;  // Default
}

std::uint16_t LevelManager::killsRemaining() const {
    const auto required = killsRequired();
    return (required > kills_this_level_) ? (required - kills_this_level_) : 0;
}

bool LevelManager::registerKill() {
    ++kills_this_level_;
    ++total_kills_;
    just_advanced_ = false;

    const auto required = killsRequired();
    
    if (kills_this_level_ >= required) {
        // Check if there's a next level
        if (current_level_ < level_configs_.size()) {
            ++current_level_;
            kills_this_level_ = 0;
            just_advanced_ = true;
            
            std::cout << "[LevelManager] Advanced to Level " << current_level_ 
                      << " (Total kills: " << total_kills_ << ")" << std::endl;
            return true;
        }
        // At max level, just keep counting kills
    }
    
    return false;
}

const LevelConfig& LevelManager::getCurrentLevelConfig() const {
    return getLevelConfig(current_level_);
}

const LevelConfig& LevelManager::getLevelConfig(std::uint16_t level) const {
    if (level > 0 && level <= level_configs_.size()) {
        return level_configs_[level - 1];
    }
    // Return last level config if beyond max, or default if empty
    if (!level_configs_.empty()) {
        return level_configs_.back();
    }
    return default_config_;
}

void LevelManager::addLevelConfig(const LevelConfig& config) {
    level_configs_.push_back(config);
}

}  // namespace rtype::game
