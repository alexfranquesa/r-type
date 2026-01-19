#pragma once

#include <cstdint>
#include <random>
#include <optional>
#include "engine/core/registry.hpp"
#include "engine/game/game_settings.hpp"

namespace rtype::game {

/**
 * @brief System that spawns and manages the final boss in Level 5
 * 
 * The boss is a large enemy that stays in the bottom-right corner,
 * has 8-frame animation, and fires slow homing projectiles that deal 40 damage.
 */
class BossSpawnSystem {
public:
    BossSpawnSystem();

    /**
     * @brief Run the boss system
     * @param reg The ECS registry
     * @param dt Delta time since last frame
     * @param current_level Current game level
     * @param settings Game settings for multipliers
     */
    void run(rtype::ecs::registry& reg, float dt, std::uint16_t current_level, 
             const engine::game::GameSettings& settings);

    /**
     * @brief Check if the boss is currently alive
     */
    bool isBossAlive() const { return boss_entity_.has_value(); }

    /**
     * @brief Reset the boss state (for game restart)
     */
    void reset();

private:
    // Boss position - BOSS.png (2048x140, 8 frames of 256x140 each, scaled 1.5x = 384x210)
    static constexpr float BOSS_X = 1000.0f;
    static constexpr float BOSS_Y = 400.0f;
    // Hitbox only covers the blue sphere on top (weak point)
    // The blue sphere is at the top-center of the boss sprite
    // At 1.5x scale: sprite is 384x210, sphere is roughly 50x50 at top
    static constexpr float BOSS_HITBOX_WIDTH = 50.0f;    // Blue sphere width
    static constexpr float BOSS_HITBOX_HEIGHT = 50.0f;   // Blue sphere height
    static constexpr float BOSS_HITBOX_OFFSET_X = 0.0f;  // Centered horizontally
    static constexpr float BOSS_HITBOX_OFFSET_Y = -80.0f; // Far up to the blue sphere on top
    static constexpr int BOSS_HEALTH = 300;       // 30 projectiles × 10 damage = 300 HP
    static constexpr float SHOOT_COOLDOWN = 2.5f; // Initial shoot rate (changes per phase)
    static constexpr std::uint16_t BOSS_LEVEL = 5; // Only spawn in level 5

    // Boss projectile properties
    static constexpr float PROJECTILE_SPEED = 120.0f;  // Slightly faster homing projectile
    static constexpr int PROJECTILE_DAMAGE = 40;       // High damage
    static constexpr float HOMING_STRENGTH = 2.5f;     // Strong tracking for magnetic effect

    std::optional<std::size_t> boss_entity_;
    float shoot_timer_;
    std::mt19937 rng_;

    void spawnBoss(rtype::ecs::registry& reg, const engine::game::GameSettings& settings);
    void updateBossShooting(rtype::ecs::registry& reg, float dt);
    void findPlayerPosition(rtype::ecs::registry& reg, float& out_x, float& out_y);
};

}  // namespace rtype::game
