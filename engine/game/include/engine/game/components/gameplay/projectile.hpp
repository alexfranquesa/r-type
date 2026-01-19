#pragma once

namespace engine::game::components {

    /**
     * @brief Component that marks an entity as a projectile
     * 
     * This component is used to identify projectile entities and store
     * projectile-specific data like damage and owner information.
     */
    struct Projectile {
        int damage{10};              // Damage dealt on impact
        int owner_id{-1};            // Entity ID of the shooter (-1 for no owner)
        float lifetime{5.0f};        // Time in seconds before auto-destruction
        float elapsed_time{0.0f};    // Time since projectile was created
        bool is_ice{false};          // True if this is an ice projectile (from IceCrab enemies)
        bool is_boss{false};         // True if this is a boss projectile (homing, high damage)
        bool is_homing{false};       // True if projectile tracks the player
        float homing_strength{0.0f}; // How strongly the projectile tracks (radians/sec)
    };

}  // namespace engine::game::components
