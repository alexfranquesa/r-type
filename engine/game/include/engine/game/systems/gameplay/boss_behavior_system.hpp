#pragma once

#include <cstdint>
#include <random>
#include "engine/core/registry.hpp"
#include "engine/game/game_settings.hpp"

namespace rtype::game {

/**
 * @brief System that controls boss behavior, phases, and attack patterns
 * 
 * This system manages the boss's AI:
 * - Phase transitions based on HP
 * - Movement patterns per phase
 * - Attack patterns (circular, fan, targeted)
 * - Minion spawning
 * - I-frames and damage flash
 * 
 * Architecture: Pure gameplay logic, no SFML, depends only on engine/core + engine/game
 */
class BossBehaviorSystem {
public:
    BossBehaviorSystem();

    /**
     * @brief Run boss behavior update
     * @param reg The ECS registry
     * @param dt Delta time
     * @param current_level Current game level
     */
    void run(rtype::ecs::registry& reg, float dt, std::uint16_t current_level);

    /**
     * @brief Reset system state
     */
    void reset();

private:
    std::mt19937 rng_;
    
    // Phase timing constants
    static constexpr float PHASE_1_SHOOT_INTERVAL = 2.5f;    // Slow shooting
    static constexpr float PHASE_2_SHOOT_INTERVAL = 1.5f;    // Faster shooting
    static constexpr float PHASE_3_SHOOT_INTERVAL = 0.8f;    // Rapid fire
    
    static constexpr float PHASE_2_MINION_INTERVAL = 12.0f;  // Spawn minions every 12s
    static constexpr float PHASE_3_MINION_INTERVAL = 8.0f;   // More frequent in phase 3
    
    static constexpr float MOVEMENT_INTERVAL = 4.0f;         // Change direction every 4s
    static constexpr float MOVEMENT_SPEED = 80.0f;           // Vertical movement speed
    
    static constexpr float IFRAME_DURATION = 0.1f;           // Brief i-frames
    static constexpr float FLASH_DURATION = 0.2f;            // Damage flash duration
    static constexpr float FLASH_INTERVAL = 0.05f;           // Flash on/off rate
    
    // Boss boundaries
    static constexpr float MIN_Y = 100.0f;
    static constexpr float MAX_Y = 620.0f;
    
    // Helper methods
    void updatePhaseTransitions(rtype::ecs::registry& reg, std::size_t boss_entity);
    void updateMovement(rtype::ecs::registry& reg, std::size_t boss_entity, float dt);
    void updateShooting(rtype::ecs::registry& reg, std::size_t boss_entity, float dt, std::uint8_t phase);
    void updateMinionSpawning(rtype::ecs::registry& reg, std::size_t boss_entity, float dt, std::uint8_t phase);
    void updateIFramesAndFlash(rtype::ecs::registry& reg, std::size_t boss_entity, float dt);
    
    void shootCircularPattern(rtype::ecs::registry& reg, float x, float y, int projectile_count);
    void shootFanPattern(rtype::ecs::registry& reg, float x, float y, float player_x, float player_y);
    void shootTargetedProjectile(rtype::ecs::registry& reg, float x, float y, float player_x, float player_y);
    void spawnMinion(rtype::ecs::registry& reg, float boss_x, float boss_y);
    
    void findPlayerPosition(rtype::ecs::registry& reg, float& out_x, float& out_y);
};

}  // namespace rtype::game
