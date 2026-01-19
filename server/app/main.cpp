#include <iostream>

#include <chrono>
#include <cstring>
#include <thread>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

#include "engine/core/engine_core.hpp"
#include "engine/core/registry.hpp"
#include "engine/game/game_api.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/velocity.hpp"
#include "engine/game/components/gameplay/faction.hpp"
#include "engine/game/components/gameplay/input_state.hpp"
#include "engine/game/components/gameplay/health.hpp"
#include "engine/game/components/network/owner.hpp"
#include "engine/game/systems/world/movement_system.hpp"
#include "engine/game/systems/gameplay/shooting_system.hpp"
#include "engine/game/systems/gameplay/projectile_system.hpp"
#include "engine/game/components/gameplay/health.hpp"
#include "engine/game/components/gameplay/lives.hpp"
#include "engine/game/components/gameplay/collider.hpp"
#include "engine/game/components/gameplay/game_stats.hpp"
#include "engine/game/components/gameplay/killer.hpp"
#include "engine/game/components/gameplay/spectator.hpp"
#include "engine/game/components/gameplay/ultimate_charge.hpp"
#include "engine/game/components/gameplay/ultimate_projectile.hpp"
#include "engine/game/components/network/owner.hpp"
#include "engine/game/game_settings.hpp"
#include "engine/game/systems/world/movement_system.hpp"
#include "engine/game/systems/gameplay/shooting_system.hpp"
#include "engine/game/systems/gameplay/projectile_system.hpp"
#include "engine/game/systems/gameplay/ultimate_activation_system.hpp"
#include "engine/game/systems/gameplay/collision_system.hpp"
#include "engine/game/systems/gameplay/health_system.hpp"
#include "engine/game/systems/gameplay/enemy_spawn_system.hpp"
#include "engine/game/systems/gameplay/enemy_shooting_system.hpp"
#include "engine/game/systems/gameplay/movement_pattern_system.hpp"
#include "engine/game/systems/gameplay/game_over_system.hpp"
#include "engine/game/systems/gameplay/level_manager.hpp"
#include "engine/game/systems/gameplay/lava_drop_spawn_system.hpp"
#include "engine/game/systems/gameplay/asteroid_spawn_system.hpp"
#include "engine/game/systems/gameplay/ice_enemy_spawn_system.hpp"
#include "engine/game/systems/gameplay/boss_spawn_system.hpp"
#include "engine/game/systems/gameplay/boss_behavior_system.hpp"
#include "engine/game/components/gameplay/enemy_type.hpp"
#include "engine/game/components/gameplay/boss_phase.hpp"
#include "engine/game/systems/network/network_send_system.hpp"
#include "engine/net/packet.hpp"
#include "network_server.hpp"
#include "apply_input_system.hpp"

int main() {
    std::cout << "[rtype_server] Bootstrapping server...\n";
    engine::core::initialize();
    engine::game::initialize();

    server::NetworkServer server;
    server.start(4242);

    // Create ECS registry for game entities
    rtype::ecs::registry registry;

    // Register all component types used by the server
    registry.register_component<engine::game::components::Position>();
    registry.register_component<engine::game::components::Velocity>();
    registry.register_component<engine::game::components::InputState>();
    registry.register_component<engine::game::components::FactionComponent>();
    registry.register_component<engine::game::components::Collider>();
    registry.register_component<engine::game::components::Health>();
    registry.register_component<engine::game::components::Lives>();
    registry.register_component<engine::game::components::Owner>();
    registry.register_component<engine::game::components::GameStats>();
    registry.register_component<engine::game::components::Killer>();
    registry.register_component<engine::game::components::EnemyTypeComponent>();
    registry.register_component<engine::game::components::BossPhase>();
    registry.register_component<engine::game::components::Spectator>();
    registry.register_component<engine::game::components::UltimateCharge>();
    registry.register_component<engine::game::components::UltimateProjectile>();

    engine::game::GameSettings settings;
    settings.load_from_file();

    // Create systems
    server::systems::ApplyInputSystem input_system;
    rtype::game::MovementSystem movement_system;
    rtype::game::ShootingSystem shooting_system;
    rtype::game::ProjectileSystem projectile_system;
    rtype::game::CollisionSystem collision_system;
    rtype::game::UltimateActivationSystem ultimate_activation_system;
    rtype::game::EnemySpawnSystem enemy_spawn_system;
    rtype::game::EnemyShootingSystem enemy_shooting_system;
    rtype::game::MovementPatternSystem movement_pattern_system;
    rtype::game::NetworkSendSystem network_send_system;
    rtype::game::GameOverSystem game_over_system;
    rtype::game::LevelManager level_manager;
    rtype::game::LavaDropSpawnSystem lava_drop_spawn_system;
    rtype::game::AsteroidSpawnSystem asteroid_spawn_system;
    rtype::game::IceEnemySpawnSystem ice_enemy_spawn_system;
    rtype::game::BossSpawnSystem boss_spawn_system;
    rtype::game::BossBehaviorSystem boss_behavior_system;

    // Game stats entity (score, wave, level progression)
    auto stats_entity = registry.spawn_entity();
    auto initial_config = level_manager.getLevelConfig(1);
    registry.emplace_component<engine::game::components::GameStats>(stats_entity, 0u, 1u);
    std::uint16_t previous_total_kills = 0;  // Track kills to know when new kills happen
    if (auto* game_stats = registry.try_get<engine::game::components::GameStats>(stats_entity)) {
        game_stats->current_level = 1;
        game_stats->kills_this_level = 0;
        game_stats->kills_to_next_level = initial_config.kills_required;
        game_stats->total_kills = 0;
    }

    // Enable debug logging for first few ticks to verify it works
    network_send_system.set_debug_logging(true);
    // Set full snapshot every 3 ticks (~50ms) to catch entity removals very quickly
    network_send_system.set_full_snapshot_interval(3);

    // =========================
    // LOBBY STATE
    // =========================
    bool game_started = false;
    bool game_paused = false;
    std::unordered_set<std::uint16_t> connected_players;
    constexpr std::size_t REQUIRED_PLAYERS = 1;  // Start game with 1 player, but allow joining anytime

    // =========================
    // PLAYER CONNECT (SPAWN IMMEDIATELY OR IN LOBBY)
    // =========================
    server.set_on_player_connect(
        [&](std::uint16_t player_id, std::uint16_t start_level, std::uint8_t difficulty) -> std::uint16_t {
            std::cout << "[lobby] Player #" << player_id << " connected (requested level " << start_level << ", difficulty " << static_cast<int>(difficulty) << ")\n";
            connected_players.insert(player_id);

            // Apply difficulty to game settings
            settings.difficulty = difficulty;

            // If this is the first player, set the game level
            if (!game_started && connected_players.size() == 1) {
                auto new_config = level_manager.getLevelConfig(start_level);
                if (auto* game_stats = registry.try_get<engine::game::components::GameStats>(stats_entity)) {
                    game_stats->current_level = start_level;
                    game_stats->kills_this_level = 0;
                    game_stats->kills_to_next_level = new_config.kills_required;
                    std::cout << "[lobby] Game level set to " << start_level << "\n";
                }
            }

            // If game already started, spawn player immediately
            if (game_started) {                // Unpause the game when a player joins
                game_paused = false;
                                auto entity = registry.spawn_entity();
                auto entity_id = static_cast<std::uint16_t>(entity);

                // Offset spawn position based on number of players
                float spawn_y = 540.f - (static_cast<float>(connected_players.size() - 1) * 80.f);
                registry.emplace_component<engine::game::components::Position>(entity, 100.f, spawn_y);
                registry.emplace_component<engine::game::components::Velocity>(entity, 0.f, 0.f);
                registry.emplace_component<engine::game::components::InputState>(entity);
                registry.emplace_component<engine::game::components::FactionComponent>(
                    entity, engine::game::components::Faction::PLAYER
                );
                registry.emplace_component<engine::game::components::Collider>(entity, 32.f, 32.f, false);
                registry.emplace_component<engine::game::components::Health>(entity, 100, 100);
                registry.emplace_component<engine::game::components::Lives>(
                    entity,
                    std::max(0, settings.player_lives),
                    std::max(0, settings.player_lives));
                registry.emplace_component<engine::game::components::UltimateCharge>(entity);
                registry.emplace_component<engine::game::components::Owner>(entity, player_id);

                input_system.register_player_entity(player_id, entity_id);

                std::cout << "[game] Spawned player #" << player_id
                          << " as entity #" << entity_id << " (mid-game join)\n";

                return entity_id;
            }

            return 0; // In lobby, wait for game to start
        }
    );

    server.set_on_player_disconnect(
        [&](std::uint16_t player_id) {
            std::cout << "[lobby] Player #" << player_id << " disconnected\n";
            connected_players.erase(player_id);
            input_system.remove_player(player_id, registry);

            // If all players disconnect, reset the game
            if (connected_players.empty() && game_started) {
                std::cout << "[lobby] All players disconnected. Resetting game...\n";

                // Clear all game entities
                registry.clear();

                // Reset level manager and recreate stats entity with initial level config
                level_manager.reset();
                stats_entity = registry.spawn_entity();
                auto reset_config = level_manager.getLevelConfig(1);
                registry.emplace_component<engine::game::components::GameStats>(stats_entity, 0u, 1u);
                if (auto* reset_stats = registry.try_get<engine::game::components::GameStats>(stats_entity)) {
                    reset_stats->current_level = 1;
                    reset_stats->kills_this_level = 0;
                    reset_stats->kills_to_next_level = reset_config.kills_required;
                    reset_stats->total_kills = 0;
                }

                // Reset game over state
                game_over_system.reset();

                game_started = false;
                game_paused = false;
                std::cout << "[lobby] Game reset complete. Waiting for players...\n";
            }
        }
    );

    std::cout << "[rtype_server] Waiting in lobby...\n";

    std::uint32_t tick = 0;
    const auto tick_duration = std::chrono::milliseconds(16);
    const float delta_time = 0.016f;

    while (true) {
        auto tick_start = std::chrono::steady_clock::now();

        // =========================
        // LOBBY CHECK
        // =========================
        if (!game_started) {
            if (connected_players.size() >= REQUIRED_PLAYERS) {
                std::cout << "[lobby] Required players reached. Starting game!\n";

                std::uint16_t player_index = 0;
                for (auto player_id : connected_players) {
                    auto entity = registry.spawn_entity();
                    auto entity_id = static_cast<std::uint16_t>(entity);

                    // Offset each player's Y position so they don't overlap
                    float spawn_y = 540.f - (player_index * 80.f);
                    registry.emplace_component<engine::game::components::Position>(entity, 100.f, spawn_y);
                    registry.emplace_component<engine::game::components::Velocity>(entity, 0.f, 0.f);
                    registry.emplace_component<engine::game::components::InputState>(entity);
                    registry.emplace_component<engine::game::components::FactionComponent>(
                        entity, engine::game::components::Faction::PLAYER
                    );
                    registry.emplace_component<engine::game::components::Collider>(entity, 32.f, 32.f, false);
                    registry.emplace_component<engine::game::components::Health>(entity, 100, 100);
                    registry.emplace_component<engine::game::components::Lives>(
                        entity,
                        std::max(0, settings.player_lives),
                        std::max(0, settings.player_lives));
                    registry.emplace_component<engine::game::components::UltimateCharge>(entity);
                    registry.emplace_component<engine::game::components::Owner>(entity, player_id);

                    input_system.register_player_entity(player_id, entity_id);

                    std::cout << "[game] Spawned player #" << player_id
                              << " as entity #" << entity_id << "\n";
                    ++player_index;
                }

                game_started = true;
            }

            std::this_thread::sleep_for(tick_duration);
            continue; // IMPORTANT: skip gameplay systems
        }

        // =========================
        // INPUT
        // =========================
        while (auto cmd_opt = server.poll_input()) {
            constexpr std::uint16_t INPUT_PAUSE = 1 << 5;
            auto& cmd = cmd_opt.value();

            if (cmd.input_mask & INPUT_PAUSE) {
                game_paused = !game_paused;
                std::cout << (game_paused ? "[server] Paused\n" : "[server] Resumed\n");
                continue; // Do not forward pause to gameplay
            }

            input_system.set_player_input(
                cmd.player_id,
                cmd.input_mask,
                cmd.sequence
            );
        }

        // =========================
        // GAMEPLAY SYSTEMS
        // =========================

        // Only run gameplay systems if not paused
        if (!game_paused) {
            // Check game over condition first
            game_over_system.run(registry, delta_time);

            // Only spawn enemies and process game logic if not game over
            if (!game_over_system.is_game_over()) {
                // Update level multipliers based on current level
                if (auto* stats = registry.try_get<engine::game::components::GameStats>(stats_entity)) {
                    const auto& current_config = level_manager.getLevelConfig(stats->current_level);
                    settings.level_enemy_speed_mult = current_config.enemy_speed_multiplier;
                    settings.level_enemy_hp_mult = current_config.enemy_hp_multiplier;
                    settings.level_spawn_rate_mult = current_config.spawn_rate_multiplier;
                }
                
                // Input and core movement
                input_system.update(registry);
                movement_system.run(registry, delta_time);
                shooting_system.run(registry, delta_time, settings);
                ultimate_activation_system.run(registry);
                projectile_system.run(registry, delta_time);
                collision_system.run(registry, delta_time);

                // Run health system which handles deaths and updates kill counts
                engine::game::systems::health_system(registry, settings);

                // Update per-player kill counts in GameStats based on Killer component
                if (auto* stats = registry.try_get<engine::game::components::GameStats>(stats_entity)) {
                    registry.view<engine::game::components::FactionComponent, engine::game::components::Health>(
                        [&](std::size_t eid, auto& faction, auto& health) {
                            if (faction.faction_value == engine::game::components::Faction::ENEMY && health.current <= 0) {
                                auto killer = registry.try_get<engine::game::components::Killer>(
                                    registry.entity_from_index(static_cast<rtype::ecs::entity_id_t>(eid)));
                                if (killer && killer->player_id > 0) {
                                    stats->player_kills[killer->player_id]++;
                                }
                            }
                        });
                }

                // Enemy behavior systems
                enemy_shooting_system.run(registry, delta_time, settings);
                movement_pattern_system.run(registry, delta_time);

                // Spawn systems based on level
                if (auto* stats = registry.try_get<engine::game::components::GameStats>(stats_entity)) {
                    if (stats->current_level == 5) {
                        // Level 5: Final Boss fight only
                        boss_spawn_system.run(registry, delta_time, stats->current_level, settings);
                        boss_behavior_system.run(registry, delta_time, stats->current_level);
                    } else if (stats->current_level == 4) {
                        // Level 4: Ice enemies only (no basic spawns)
                        ice_enemy_spawn_system.run(registry, delta_time, stats->current_level, settings);
                        lava_drop_spawn_system.run(registry, delta_time, stats->current_level);
                    } else {
                        // Levels 1-3: Normal enemy spawning
                        enemy_spawn_system.run(registry, delta_time, stats->current_level, settings);
                        lava_drop_spawn_system.run(registry, delta_time, stats->current_level);
                    }
                }
            }
        }

        // Check level progression based on GameStats (updated by health_system)
        if (auto* stats = registry.try_get<engine::game::components::GameStats>(stats_entity)) {
            // Check if we should advance to next level (but not beyond level 5 - Final Boss)
            const std::uint16_t MAX_LEVEL = 5;
            if (stats->kills_this_level >= stats->kills_to_next_level && stats->current_level < MAX_LEVEL) {
                // Get next level config
                const auto& next_config = level_manager.getLevelConfig(stats->current_level + 1);

                // Clear all enemies and projectiles when changing level
                std::vector<rtype::ecs::entity_t> entities_to_remove;
                registry.view<engine::game::components::FactionComponent>(
                    [&](std::size_t eid, auto& faction) {
                        if (faction.faction_value == engine::game::components::Faction::ENEMY ||
                            faction.faction_value == engine::game::components::Faction::HAZARD) {
                            entities_to_remove.push_back(
                                registry.entity_from_index(static_cast<rtype::ecs::entity_id_t>(eid)));
                        }
                    });
                
                // Also remove enemy projectiles
                registry.view<engine::game::components::Projectile, engine::game::components::FactionComponent>(
                    [&](std::size_t eid, auto& /*proj*/, auto& faction) {
                        if (faction.faction_value == engine::game::components::Faction::ENEMY) {
                            auto entity = registry.entity_from_index(static_cast<rtype::ecs::entity_id_t>(eid));
                            if (std::find(entities_to_remove.begin(), entities_to_remove.end(), entity) == entities_to_remove.end()) {
                                entities_to_remove.push_back(entity);
                            }
                        }
                    });
                
                for (const auto& entity : entities_to_remove) {
                    registry.kill_entity(entity);
                }
                
                std::cout << "[server] Level transition: cleared " << entities_to_remove.size() << " entities" << std::endl;

                // Advance level
                stats->current_level++;
                stats->kills_this_level = 0;  // Reset kills for new level
                stats->kills_to_next_level = next_config.kills_required;
                stats->wave = stats->current_level;

                std::cout << "[server] LEVEL UP! Now at Level " << stats->current_level
                          << " (need " << stats->kills_to_next_level << " kills)" << std::endl;
            }

            // Add points for newly killed enemies (100 points per kill)
            // Score can also be decreased by collision_system when player takes damage
            if (stats->total_kills > previous_total_kills) {
                std::uint16_t new_kills = stats->total_kills - previous_total_kills;
                stats->score += (new_kills * 100);
                previous_total_kills = stats->total_kills;
            }
        }

        // Build and broadcast snapshot using NetworkSendSystem
        auto snapshot = network_send_system.build_snapshot(registry, tick++, game_paused);
        server.broadcast_snapshot(snapshot);

        auto tick_end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tick_end - tick_start);
        if (elapsed < tick_duration) {
            std::this_thread::sleep_for(tick_duration - elapsed);
        }
    }
}
