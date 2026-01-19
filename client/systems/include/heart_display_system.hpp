#pragma once

#include <vector>
#include <cstdint>
#include <SFML/Graphics/RenderWindow.hpp>
#include "engine/core/registry.hpp"
#include "engine/game/game_settings.hpp"

namespace client::systems {

/**
 * @brief System that manages heart sprite entities to display player lives.
 * 
 * Creates and maintains heart sprite entities at the top of the screen.
 * Each heart represents one life. The number of hearts displayed is determined
 * by the game settings (player_lives).
 */
class HeartDisplaySystem {
public:
    explicit HeartDisplaySystem();

    /**
     * @brief Update the heart display based on player's current lives.
     * 
     * Creates, updates, or removes heart entities based on the player's
     * remaining lives and max lives from settings.
     * 
     * @param hud_reg ECS registry for HUD entities (client-only, not affected by server snapshots)
     * @param window Render window (used to get dimensions)
     * @param local_player_id ID of the local player
     * @param settings Game settings containing max lives configuration
     */
    void update(
        rtype::ecs::registry& hud_reg,
        sf::RenderWindow& window,
        std::uint16_t local_player_id,
        const engine::game::GameSettings& settings
    );
    
    /**
     * @brief Update the heart display based on player's current lives from game registry.
     * 
     * This overload searches for player lives in a separate game registry
     * (where server entities are) but creates hearts in the HUD registry.
     * 
     * @param hud_reg ECS registry for HUD entities (client-only)
     * @param game_reg ECS registry containing game entities (from server)
     * @param window Render window (used to get dimensions)
     * @param local_player_id ID of the local player
     * @param settings Game settings containing max lives configuration
     * @param dt Delta time for animations
     */
    void update(
        rtype::ecs::registry& hud_reg,
        rtype::ecs::registry& game_reg,
        sf::RenderWindow& window,
        std::uint16_t local_player_id,
        const engine::game::GameSettings& settings,
        float dt
    );

private:
    // Entity IDs of heart sprites
    std::vector<rtype::ecs::entity_t> heart_entities_;
    
    // Animation state for each heart
    struct HeartAnimation {
        bool is_animating = false;
        float timer = 0.0f;
        int current_frame = 0;
    };
    std::vector<HeartAnimation> heart_animations_;
    
    // Last known state to detect changes
    int last_max_lives_ = 0;
    int last_remaining_lives_ = 0;
    
    // Heart display configuration
    static constexpr float heart_scale_ = 2.5f;      // Increased scale for better visibility
    static constexpr float heart_spacing_ = 75.0f;   // More spacing between hearts
    static constexpr float heart_y_position_ = 60.0f; // Adjusted to show full heart (32*2.5/2 + margin)
    static constexpr float heart_start_x_offset_ = 160.0f; // Left side offset to avoid FPS overlap
    
    // Animation configuration
    static constexpr int heart_frames_ = 4;          // Number of frames in the spritesheet
    static constexpr int heart_frame_width_ = 32;    // Width of each frame
    static constexpr int heart_frame_height_ = 32;   // Height of each frame
    static constexpr float frame_duration_ = 0.1f;   // Duration of each frame in seconds
    
    /**
     * @brief Create heart entities in the registry
     * @param reg ECS registry
     * @param count Number of hearts to create
     * @param window_width Width of the window for positioning
     */
    void create_hearts(rtype::ecs::registry& reg, int count, float window_width);
    
    /**
     * @brief Update visibility of heart entities based on remaining lives
     * @param reg ECS registry
     * @param remaining_lives Number of lives remaining
     */
    void update_heart_visibility(rtype::ecs::registry& reg, int remaining_lives);
    
    /**
     * @brief Update animations for hearts
     * @param reg ECS registry
     * @param dt Delta time
     */
    void update_animations(rtype::ecs::registry& reg, float dt);
    
    /**
     * @brief Remove all heart entities from the registry
     * @param reg ECS registry
     */
    void clear_hearts(rtype::ecs::registry& reg);
};

}  // namespace client::systems
