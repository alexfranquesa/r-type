/*
** EPITECH PROJECT, 2025
** G-CPP-500-BAR-5-2-rtype-2
** File description:
** network_send_system
*/

#include "engine/game/systems/network/network_send_system.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/velocity.hpp"
#include "engine/game/components/core/sprite.hpp"
#include "engine/game/components/gameplay/health.hpp"
#include "engine/game/components/gameplay/lives.hpp"
#include "engine/game/components/gameplay/faction.hpp"
#include "engine/game/components/gameplay/projectile.hpp"
#include "engine/game/components/gameplay/game_stats.hpp"
#include "engine/game/components/gameplay/enemy_type.hpp"
#include "engine/game/components/gameplay/spectator.hpp"
#include "engine/game/components/gameplay/ultimate_charge.hpp"
#include "engine/game/components/gameplay/ultimate_projectile.hpp"
#include "engine/game/components/network/owner.hpp"
#include "engine/game/components/visual/sprite_id.hpp"
#include <iostream>
#include <cmath>
#include <unordered_map>

namespace rtype::game {

void NetworkSendSystem::serialize_uint16(std::vector<std::uint8_t>& blob, std::uint16_t value) {
    // Little-endian: LSB first, MSB second
    blob.push_back(static_cast<std::uint8_t>(value & 0xFF));
    blob.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFF));
}

void NetworkSendSystem::serialize_float(std::vector<std::uint8_t>& blob, float value) {
    // Convert float to 32-bit representation using memcpy (type-punning safe way)
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(float));

    // Serialize as 4 bytes, little-endian
    blob.push_back(static_cast<std::uint8_t>(bits & 0xFF));
    blob.push_back(static_cast<std::uint8_t>((bits >> 8) & 0xFF));
    blob.push_back(static_cast<std::uint8_t>((bits >> 16) & 0xFF));
    blob.push_back(static_cast<std::uint8_t>((bits >> 24) & 0xFF));
}

void NetworkSendSystem::serialize_int16(std::vector<std::uint8_t>& blob, std::int16_t value) {
    blob.push_back(static_cast<std::uint8_t>(value & 0xFF));
    blob.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFF));
}

void NetworkSendSystem::serialize_uint32(std::vector<std::uint8_t>& blob, std::uint32_t value) {
    blob.push_back(static_cast<std::uint8_t>(value & 0xFF));
    blob.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFF));
    blob.push_back(static_cast<std::uint8_t>((value >> 16) & 0xFF));
    blob.push_back(static_cast<std::uint8_t>((value >> 24) & 0xFF));
}

std::uint16_t NetworkSendSystem::pick_sprite_id(rtype::ecs::registry& reg, std::size_t entity_id) const {
    const rtype::ecs::entity_t ent{static_cast<rtype::ecs::entity_id_t>(entity_id)};
    // Simple IDs understood by the client:
    using engine::game::components::SpriteId;
    // 0 = player, 1 = enemy_basic, 2 = player_projectile, 3 = enemy_projectile,
    // 4 = lava_drop, 6 = asteroid, 7 = ice_enemy, 8 = ice_projectile
    // 9 = boss, 10 = boss_projectile, 11 = ultimate_projectile

    auto faction = reg.try_get<engine::game::components::FactionComponent>(ent);
    auto enemy_type = reg.try_get<engine::game::components::EnemyTypeComponent>(ent);
    auto sprite = reg.try_get<engine::game::components::Sprite>(ent);

    // Check for hazards first (lava drops, etc.)
    if (faction && faction->faction_value == engine::game::components::Faction::HAZARD) {
        return static_cast<std::uint16_t>(SpriteId::LavaDrop);
    }
    
    // Check for obstacles (asteroids) - can be OBSTACLE faction or ENEMY faction with "asteroid" texture
    if (faction && faction->faction_value == engine::game::components::Faction::OBSTACLE) {
        return static_cast<std::uint16_t>(SpriteId::Asteroid);
    }
    
    // Check if this is an asteroid by sprite texture_id (since we changed asteroids to ENEMY faction)
    if (sprite && sprite->texture_id == "asteroid") {
        return static_cast<std::uint16_t>(SpriteId::Asteroid);
    }

    if (reg.try_get<engine::game::components::UltimateProjectile>(ent)) {
        return static_cast<std::uint16_t>(SpriteId::UltimateProjectile);
    }

    auto projectile = reg.try_get<engine::game::components::Projectile>(ent);
    if (projectile) {
        if (faction && faction->faction_value == engine::game::components::Faction::ENEMY) {
            // Check if this is a boss projectile (homing)
            if (projectile->is_boss) {
                return static_cast<std::uint16_t>(SpriteId::BossProjectile);
            }
            // Check if this is an ice projectile (marked by the shooting system)
            if (projectile->is_ice) {
                return static_cast<std::uint16_t>(SpriteId::IceProjectile);
            }
            return static_cast<std::uint16_t>(SpriteId::EnemyProjectile);
        }
        return static_cast<std::uint16_t>(SpriteId::PlayerProjectile);
    }
    
    // Check for boss enemy
    if (faction && faction->faction_value == engine::game::components::Faction::ENEMY) {
        if (enemy_type && enemy_type->type == engine::game::components::EnemyType::Boss) {
            return static_cast<std::uint16_t>(SpriteId::Boss);
        }
        if (enemy_type && enemy_type->type == engine::game::components::EnemyType::IceCrab) {
            return static_cast<std::uint16_t>(SpriteId::IceEnemy);
        }
        // Also check sprite texture_id for ice_crab (backup check)
        if (sprite && sprite->texture_id == "ice_crab") {
            return static_cast<std::uint16_t>(SpriteId::IceEnemy);
        }
        return static_cast<std::uint16_t>(SpriteId::EnemyBasic);
    }
    // default/player
    return static_cast<std::uint16_t>(SpriteId::Player);
}

bool NetworkSendSystem::entity_changed(std::uint16_t entity_id, const EntitySnapshot& current) const {
    auto it = previous_state_.find(entity_id);
    if (it == previous_state_.end()) {
        return true;  // New entity, include it
    }
    
    const auto& prev = it->second;
    const float position_threshold = 0.5f;  // 0.5 pixels
    
    // Check if any significant values changed
    if (std::abs(current.x - prev.x) > position_threshold ||
        std::abs(current.y - prev.y) > position_threshold ||
        std::abs(current.vx - prev.vx) > position_threshold ||
        std::abs(current.vy - prev.vy) > position_threshold ||
        current.hp_cur != prev.hp_cur ||
        current.hp_max != prev.hp_max ||
        current.sprite_id != prev.sprite_id ||
        current.owner_id != prev.owner_id ||
        current.lives_remaining != prev.lives_remaining ||
        current.lives_max != prev.lives_max ||
        current.is_spectating != prev.is_spectating ||
        current.ultimate_frame != prev.ultimate_frame ||
        current.ultimate_ready != prev.ultimate_ready) {
        return true;
    }
    
    return false;  // No significant change
}

engine::net::SnapshotMessage NetworkSendSystem::build_snapshot(
    rtype::ecs::registry& reg,
    std::uint32_t tick,
    bool paused
) {
    engine::net::SnapshotMessage snapshot;
    snapshot.tick = tick;
    snapshot.paused = paused;
    
    // Determine if we should send a full snapshot (baseline) or delta
    bool send_full_snapshot = !delta_compression_enabled_ || 
                             (tick - last_full_snapshot_tick_) >= full_snapshot_interval_;
    
    if (send_full_snapshot) {
        last_full_snapshot_tick_ = tick;
        snapshot.flags = 1;  // 1 = full snapshot (client can clean missing entities)
    } else {
        snapshot.flags = 0;  // 0 = delta snapshot (only changed entities)
    }
    
    snapshot.blob.clear();

    // Collect entities to send (all for full snapshot, only changed for delta)
    std::vector<std::pair<std::uint16_t, EntitySnapshot>> entities_to_send;
    
    reg.view<engine::game::components::Position>(
        [&](size_t entity_id, auto& pos) {
            const rtype::ecs::entity_t ent{static_cast<rtype::ecs::entity_id_t>(entity_id)};
            const auto vel = reg.try_get<engine::game::components::Velocity>(ent);
            const auto hp  = reg.try_get<engine::game::components::Health>(ent);
            const auto owner = reg.try_get<engine::game::components::Owner>(ent);
            const auto lives = reg.try_get<engine::game::components::Lives>(ent);
            const auto spectator = reg.try_get<engine::game::components::Spectator>(ent);
            const auto projectile = reg.try_get<engine::game::components::Projectile>(ent);
            const auto ultimate_charge = reg.try_get<engine::game::components::UltimateCharge>(ent);

            EntitySnapshot current;
            current.x = pos.x;
            current.y = pos.y;
            current.vx = vel ? vel->vx : 0.0f;
            current.vy = vel ? vel->vy : 0.0f;
            current.hp_cur = hp ? static_cast<std::int16_t>(hp->current) : static_cast<std::int16_t>(-1);
            current.hp_max = hp ? static_cast<std::int16_t>(hp->max) : static_cast<std::int16_t>(-1);
            current.sprite_id = pick_sprite_id(reg, entity_id);
            current.owner_id = owner ? owner->player_id : 0;
            current.lives_remaining = lives ? static_cast<std::int16_t>(lives->remaining) : static_cast<std::int16_t>(-1);
            current.lives_max = lives ? static_cast<std::int16_t>(lives->max) : static_cast<std::int16_t>(-1);
            current.is_spectating = (spectator && spectator->is_spectating) ? 1 : 0;
            current.ultimate_frame = ultimate_charge ? ultimate_charge->ui_frame : 0;
            current.ultimate_ready = (ultimate_charge && ultimate_charge->ready) ? 1 : 0;
            
            // Projectiles are always sent (critical for gameplay, fast-moving)
            bool always_send = (projectile != nullptr);
            
            // For delta snapshots, only include if changed (or always send projectiles)
            if (send_full_snapshot || always_send || entity_changed(static_cast<std::uint16_t>(entity_id), current)) {
                entities_to_send.emplace_back(static_cast<std::uint16_t>(entity_id), current);
                // Update previous state
                previous_state_[static_cast<std::uint16_t>(entity_id)] = current;
            }
        }
    );
    
    // Clean up previous_state_ for deleted entities (on full snapshots)
    if (send_full_snapshot) {
        std::unordered_map<std::uint16_t, EntitySnapshot> new_state;
        for (const auto& [entity_id, _] : entities_to_send) {
            auto it = previous_state_.find(entity_id);
            if (it != previous_state_.end()) {
                new_state[entity_id] = it->second;
            }
        }
        previous_state_ = std::move(new_state);
    }

    std::uint16_t entity_count = static_cast<std::uint16_t>(entities_to_send.size());
    
    if (debug_logging_) {
        std::cout << "[NetworkSendSystem] Tick=" << tick
                  << " type=" << (send_full_snapshot ? "FULL" : "DELTA")
                  << " entities=" << entity_count << std::endl;
    }

    // Write entity count
    serialize_uint16(snapshot.blob, entity_count);

    // Serialize entities
    for (const auto& [entity_id, entity_data] : entities_to_send) {
        serialize_uint16(snapshot.blob, entity_id);
        serialize_float(snapshot.blob, entity_data.x);
        serialize_float(snapshot.blob, entity_data.y);
        serialize_float(snapshot.blob, entity_data.vx);
        serialize_float(snapshot.blob, entity_data.vy);
        serialize_int16(snapshot.blob, entity_data.hp_cur);
        serialize_int16(snapshot.blob, entity_data.hp_max);
        serialize_uint16(snapshot.blob, entity_data.sprite_id);
        serialize_uint16(snapshot.blob, entity_data.owner_id);
        serialize_int16(snapshot.blob, entity_data.lives_remaining);
        serialize_int16(snapshot.blob, entity_data.lives_max);
        snapshot.blob.push_back(entity_data.is_spectating);
        snapshot.blob.push_back(entity_data.ultimate_frame);
        snapshot.blob.push_back(entity_data.ultimate_ready);

        if (debug_logging_) {
            std::cout << "  Entity #" << entity_id
                      << " pos=(" << entity_data.x << ", " << entity_data.y << ")"
                      << " vel=(" << entity_data.vx << ", " << entity_data.vy << ")"
                      << " hp=(" << entity_data.hp_cur << "/" << entity_data.hp_max << ")"
                      << " sprite_id=" << entity_data.sprite_id << std::endl;
        }
    }

    // Append global stats (score + wave + level info) if present
    std::uint32_t score = 0;
    std::uint16_t wave = 1;
    std::uint16_t current_level = 1;
    std::uint16_t kills_this_level = 0;
    std::uint16_t kills_to_next_level = 15;
    std::uint16_t total_kills = 0;

    const auto& stats = reg.get_components<engine::game::components::GameStats>();
    for (std::size_t idx = 0; idx < stats.size(); ++idx) {
        if (stats[idx].has_value()) {
            score = stats[idx]->score;
            wave = stats[idx]->wave;
            current_level = stats[idx]->current_level;
            kills_this_level = stats[idx]->kills_this_level;
            kills_to_next_level = stats[idx]->kills_to_next_level;
            total_kills = stats[idx]->total_kills;
            break;
        }
    }
    serialize_uint32(snapshot.blob, score);
    serialize_uint16(snapshot.blob, wave);
    serialize_uint16(snapshot.blob, current_level);
    serialize_uint16(snapshot.blob, kills_this_level);
    serialize_uint16(snapshot.blob, kills_to_next_level);
    serialize_uint16(snapshot.blob, total_kills);

    if (debug_logging_) {
        std::cout << "[NetworkSendSystem] Snapshot size: " << snapshot.blob.size()
                  << " bytes, Level=" << current_level
                  << " Kills=" << kills_this_level << "/" << kills_to_next_level << std::endl;
    }

    // STEP 4: Return the complete snapshot
    return snapshot;
}

}  // namespace rtype::game
