#include "heart_display_system.hpp"

#include <algorithm>
#include <iostream>
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/sprite.hpp"
#include "engine/game/components/gameplay/lives.hpp"
#include "engine/game/components/network/owner.hpp"

namespace client::systems {

using engine::game::components::Position;
using engine::game::components::Sprite;
using engine::game::components::Lives;
using engine::game::components::Owner;

HeartDisplaySystem::HeartDisplaySystem() {}

void HeartDisplaySystem::update(
    rtype::ecs::registry& hud_reg,
    sf::RenderWindow& window,
    std::uint16_t local_player_id,
    const engine::game::GameSettings& settings
) {
    // Find the local player entity and get their lives
    int current_remaining = settings.player_lives;  // Default to max
    int max_lives = settings.player_lives;
    
    // If player not found or in infinite lives mode, show max lives
    if (settings.infinite_lives) {
        current_remaining = max_lives;
    }
    
    // Check if we need to recreate hearts (max lives changed)
    if (max_lives != last_max_lives_) {
        std::cout << "[HeartDisplay] Creating " << max_lives << " heart entities in HUD registry" << std::endl;
        clear_hearts(hud_reg);
        create_hearts(hud_reg, max_lives, static_cast<float>(window.getSize().x));
        last_max_lives_ = max_lives;
        last_remaining_lives_ = current_remaining;
    }
    
    // Update visibility based on remaining lives
    if (current_remaining != last_remaining_lives_) {
        std::cout << "[HeartDisplay] Updating visibility: " << current_remaining << " hearts visible" << std::endl;
        update_heart_visibility(hud_reg, current_remaining);
        last_remaining_lives_ = current_remaining;
    }
}

void HeartDisplaySystem::update(
    rtype::ecs::registry& hud_reg,
    rtype::ecs::registry& game_reg,
    sf::RenderWindow& window,
    std::uint16_t local_player_id,
    const engine::game::GameSettings& settings,
    float dt
) {
    // Find the local player entity in the GAME registry and get their lives
    int current_remaining = 0;
    int max_lives = settings.player_lives;
    
    bool player_found = false;
    game_reg.view<Owner, Lives>([&](std::size_t /*eid*/, Owner& owner, Lives& lives) {
        if (owner.player_id == local_player_id) {
            current_remaining = lives.remaining;
            player_found = true;
            std::cout << "[HeartDisplay] Found player " << local_player_id 
                      << " with " << lives.remaining << "/" << lives.max << " lives" << std::endl;
        }
    });
    
    // If player not found yet or in infinite lives mode, show max lives
    if (!player_found || settings.infinite_lives) {
        current_remaining = max_lives;
        if (!player_found) {
            std::cout << "[HeartDisplay] Player not found yet, showing max lives: " << max_lives << std::endl;
        }
    }
    
    // Check if we need to recreate hearts (max lives changed) - create in HUD registry
    if (max_lives != last_max_lives_) {
        std::cout << "[HeartDisplay] Creating " << max_lives << " heart entities in HUD registry" << std::endl;
        clear_hearts(hud_reg);
        create_hearts(hud_reg, max_lives, static_cast<float>(window.getSize().x));
        last_max_lives_ = max_lives;
        last_remaining_lives_ = current_remaining;
        
        // Initialize visibility without triggering animations
        update_heart_visibility(hud_reg, current_remaining);
        return; // Skip animation logic on first creation
    }
    
    // Detect if a life was lost and trigger animation (only if not first initialization)
    if (current_remaining < last_remaining_lives_ && last_remaining_lives_ > 0) {
        std::cout << "[HeartDisplay] Life lost! Starting animation for hearts " 
                  << current_remaining << " to " << (last_remaining_lives_ - 1) << std::endl;
        // Start animation for the hearts that were just lost
        for (int i = current_remaining; i < last_remaining_lives_; ++i) {
            if (static_cast<std::size_t>(i) < heart_animations_.size()) {
                heart_animations_[i].is_animating = true;
                heart_animations_[i].timer = 0.0f;
                heart_animations_[i].current_frame = 0;
            }
        }
        last_remaining_lives_ = current_remaining;
    }
    
    // Update animations
    update_animations(hud_reg, dt);
    
    // Update visibility based on remaining lives (only if lives changed and it's not an animation)
    if (current_remaining != last_remaining_lives_) {
        std::cout << "[HeartDisplay] Updating visibility: " << current_remaining << " hearts visible" << std::endl;
        update_heart_visibility(hud_reg, current_remaining);
        last_remaining_lives_ = current_remaining;
    }
}

void HeartDisplaySystem::create_hearts(rtype::ecs::registry& reg, int count, float window_width) {
    heart_entities_.clear();
    heart_entities_.reserve(static_cast<std::size_t>(count));
    heart_animations_.clear();
    heart_animations_.resize(count);
    
    // Position hearts on the left side
    float start_x = heart_start_x_offset_;
    
    for (int i = 0; i < count; ++i) {
        auto entity = reg.spawn_entity();
        
        // Add Position component
        Position pos;
        pos.x = start_x + (static_cast<float>(i) * heart_spacing_);
        pos.y = heart_y_position_;
        reg.add_component(entity, std::move(pos));
        
        // Add Sprite component
        Sprite sprite;
        sprite.texture_id = "Heard";  // The sprite filename without extension
        
        // Set texture rect to show only the first frame (assuming 32x32 per frame in a 128x32 spritesheet)
        sprite.texture_rect.left = 0;
        sprite.texture_rect.top = 0;
        sprite.texture_rect.width = 32;   // Width of one heart frame
        sprite.texture_rect.height = 32;  // Height of one heart frame
        
        sprite.scale_x = heart_scale_;
        sprite.scale_y = heart_scale_;
        sprite.z_index = 100.0f;  // High z-index to render on top
        sprite.visible = true;
        sprite.origin_x = 16.0f;  // Center of the 32x32 frame
        sprite.origin_y = 16.0f;
        reg.add_component(entity, std::move(sprite));
        
        std::cout << "[HeartDisplay] Created heart entity " << entity 
                  << " at position (" << pos.x << ", " << pos.y << ")" << std::endl;
        
        heart_entities_.push_back(entity);
    }
    std::cout << "[HeartDisplay] Total hearts created: " << heart_entities_.size() << std::endl;
}

void HeartDisplaySystem::update_heart_visibility(rtype::ecs::registry& reg, int remaining_lives) {
    for (std::size_t i = 0; i < heart_entities_.size(); ++i) {
        auto entity = heart_entities_[i];
        auto* sprite = reg.try_get<Sprite>(entity);
        
        if (sprite) {
            // Don't change visibility if the heart is currently animating
            if (i < heart_animations_.size() && heart_animations_[i].is_animating) {
                continue;
            }
            
            if (static_cast<int>(i) < remaining_lives) {
                // Heart is still alive - show full heart (first frame)
                sprite->texture_rect.left = 0;
                sprite->visible = true;
            } else {
                // Heart is lost - hide it
                sprite->visible = false;
            }
        }
    }
}

void HeartDisplaySystem::update_animations(rtype::ecs::registry& reg, float dt) {
    for (std::size_t i = 0; i < heart_animations_.size(); ++i) {
        auto& anim = heart_animations_[i];
        
        if (!anim.is_animating) {
            continue;
        }
        
        anim.timer += dt;
        
        // Check if we need to advance to the next frame
        if (anim.timer >= frame_duration_) {
            anim.timer -= frame_duration_;
            anim.current_frame++;
            
            // Update sprite frame
            if (i < heart_entities_.size()) {
                auto entity = heart_entities_[i];
                auto* sprite = reg.try_get<Sprite>(entity);
                
                if (sprite) {
                    if (anim.current_frame >= heart_frames_) {
                        // Animation complete - hide the heart
                        sprite->visible = false;
                        anim.is_animating = false;
                        std::cout << "[HeartDisplay] Animation complete for heart " << i << std::endl;
                    } else {
                        // Update to next frame
                        sprite->texture_rect.left = anim.current_frame * heart_frame_width_;
                        sprite->visible = true;
                    }
                }
            }
        }
    }
}

void HeartDisplaySystem::clear_hearts(rtype::ecs::registry& reg) {
    for (auto entity : heart_entities_) {
        reg.kill_entity(entity);
    }
    heart_entities_.clear();
    last_max_lives_ = 0;
    last_remaining_lives_ = 0;
}

}  // namespace client::systems
