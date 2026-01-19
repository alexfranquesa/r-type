#include "client_context.hpp"

#include <algorithm>

#include "../ui/ui_helpers.hpp"

using namespace client::app;

static void handle_snapshots(ClientContext& ctx) {
    if (ctx.game_state != GameState::InGame || !ctx.net_client) return;
    while (auto snapshot = ctx.net_client->poll_snapshot()) {
        ctx.snapshot_system.apply(ctx.registry, *snapshot);
        
        // Client-side prediction reconciliation
        if (ctx.local_player_entity) {
            const std::uint16_t player_id = ctx.net_client->get_player_id();
            // Find our player entity in the snapshot to get server-authoritative position
            ctx.registry.view<engine::game::components::Owner, engine::game::components::Position>(
                [&](std::size_t eid, const auto& owner, const auto& pos) {
                    if (owner.player_id == player_id) {
                        // Reconcile prediction with server position
                        const float dt = 1.0f / 60.0f;  // Approximate server tick rate
                        ctx.prediction_system.reconcile(
                            ctx.registry,
                            static_cast<std::uint16_t>(eid),
                            pos.x,
                            pos.y,
                            snapshot->last_processed_input,
                            dt
                        );
                    }
                });
        }
    }
    ctx.registry.view<engine::game::components::GameStats>(
        [&](std::size_t /*eid*/, const auto& gs) {
            ctx.starfield.setLevel(gs.current_level);
        });
}

static void handle_fullscreen_and_vsync(ClientContext& ctx) {
    if (ctx.settings.fullscreen != ctx.last_fullscreen_state) {
        ctx.last_fullscreen_state = ctx.settings.fullscreen;
        ctx.window.close();
        const auto new_state = ctx.settings.fullscreen ? sf::State::Fullscreen : sf::State::Windowed;
        ctx.window.create(sf::VideoMode({1280U, 720U}), "R-TYPE Client", sf::Style::Close, new_state);
        ctx.window.setFramerateLimit(static_cast<unsigned>(ctx.settings.target_fps));
        ctx.window.setVerticalSyncEnabled(ctx.settings.vsync);
        ctx.window.setView(ctx.virtual_view);
    }
    if (ctx.settings.vsync != ctx.last_vsync_state) {
        ctx.last_vsync_state = ctx.settings.vsync;
        ctx.window.setVerticalSyncEnabled(ctx.settings.vsync);
    }
    if (ctx.settings.target_fps != ctx.last_target_fps) {
        ctx.last_target_fps = ctx.settings.target_fps;
        ctx.window.setFramerateLimit(static_cast<unsigned>(ctx.settings.target_fps));
    }
}

static void update_fps(ClientContext& ctx) {
    ctx.fps_counter++;
    if (ctx.fps_clock.getElapsedTime().asSeconds() >= 1.0f) {
        ctx.current_fps = ctx.fps_counter;
        ctx.fps_counter = 0;
        ctx.fps_clock.restart();
    }
}

static void render_state(ClientContext& ctx, float dt) {
    ctx.window.setView(ctx.virtual_view);

    if (ctx.game_state == GameState::MainMenu) {
        ctx.background.render(ctx.window);
        ctx.render_system.draw(ctx.star_registry, ctx.window);
        client::systems::render_title_logo(ctx.window);
        ctx.ui_render->draw(ctx.menu_ui_registry, ctx.window);
    } else if (ctx.game_state == GameState::Settings) {
        ctx.background.render(ctx.window);
        ctx.render_system.draw(ctx.star_registry, ctx.window);
        ctx.ui_render->draw(ctx.settings_ui_registry, ctx.window);
    } else if (ctx.game_state == GameState::Scores) {
        ctx.background.render(ctx.window);
        ctx.render_system.draw(ctx.star_registry, ctx.window);
        ctx.ui_render->draw(ctx.scores_ui_registry, ctx.window);
    } else if (ctx.game_state == GameState::Help) {
        ctx.background.render(ctx.window);
        ctx.render_system.draw(ctx.star_registry, ctx.window);
        ctx.ui_render->draw(ctx.help_ui_registry, ctx.window);
    } else if (ctx.game_state == GameState::Connect) {
        ctx.background.render(ctx.window);
        ctx.render_system.draw(ctx.star_registry, ctx.window);
        ctx.ui_render->draw(ctx.connect_ui_registry, ctx.window);
    } else if (ctx.game_state == GameState::InGame) {
        ctx.animation_system.update(ctx.registry, dt);
        ctx.particle_system.update(ctx.registry, dt);
        ctx.hud->update(dt);
        ctx.starfield.drawBackground(ctx.window);
        ctx.render_system.draw(ctx.star_registry, ctx.window);
        ctx.render_system.draw(ctx.registry, ctx.window);

        const std::uint16_t my_player_id = ctx.net_client ? ctx.net_client->get_player_id() : 0;
        ctx.heart_display.update(ctx.hud_registry, ctx.registry, ctx.window, my_player_id, ctx.settings, dt);
        ctx.render_system.draw(ctx.hud_registry, ctx.window);
        if (ctx.renderer) {
            ctx.ultimate_display.draw(*ctx.renderer, ctx.registry, ctx.window, my_player_id);
        }
        ctx.hud->draw(ctx.registry, ctx.window, my_player_id);

        bool is_spectating = false;
        if (my_player_id > 0) {
            ctx.registry.view<engine::game::components::Owner, engine::game::components::Spectator>(
                [&](size_t /*eid*/, const auto& owner, const auto& spectator) {
                    if (owner.player_id == my_player_id && spectator.is_spectating) is_spectating = true;
                });
        }
        if (is_spectating) {
            sf::Text spectator_text(ctx.overlay_font, "SPECTATING", 48);
            spectator_text.setFillColor(sf::Color(255, 255, 255, 200));
            spectator_text.setOutlineColor(sf::Color(0, 0, 0, 255));
            spectator_text.setOutlineThickness(3.f);
            spectator_text.setStyle(sf::Text::Bold);
            sf::FloatRect bounds = spectator_text.getLocalBounds();
            spectator_text.setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
            spectator_text.setPosition({static_cast<float>(ctx.window.getSize().x) / 2.f, 100.f});
            ctx.window.draw(spectator_text);

            sf::Text subtitle_text(ctx.overlay_font, "You were eliminated. Watching other players...", 24);
            subtitle_text.setFillColor(sf::Color(200, 200, 200, 180));
            subtitle_text.setOutlineColor(sf::Color(0, 0, 0, 200));
            subtitle_text.setOutlineThickness(2.f);
            bounds = subtitle_text.getLocalBounds();
            subtitle_text.setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
            subtitle_text.setPosition({static_cast<float>(ctx.window.getSize().x) / 2.f, 160.f});
            ctx.window.draw(subtitle_text);
        }
        ctx.hud->update_fps(dt);
        ctx.hud->draw_fps(ctx.window, ctx.settings.show_fps);
    } else if (ctx.game_state == GameState::DeathMenu) {
        ctx.background.render(ctx.window);
        ctx.render_system.draw(ctx.star_registry, ctx.window);
        ctx.ui_render->draw(ctx.death_menu_ui_registry, ctx.window);
    } else if (ctx.game_state == GameState::GameOver) {
        ctx.background.render(ctx.window);
        ctx.render_system.draw(ctx.star_registry, ctx.window);
        ctx.ui_render->draw(ctx.game_over_ui_registry, ctx.window);
    }

    if (ctx.game_state != GameState::InGame && ctx.settings.show_fps) {
        sf::Text fps_text(ctx.overlay_font);
        fps_text.setString("FPS: " + std::to_string(ctx.current_fps));
        fps_text.setCharacterSize(20);
        fps_text.setFillColor(sf::Color::Yellow);
        fps_text.setPosition({10.f, 10.f});
        ctx.window.draw(fps_text);
    }
}

static void detect_game_over(ClientContext& ctx) {
    if (ctx.game_state != GameState::InGame) return;
    const std::uint16_t my_player_id = ctx.net_client ? ctx.net_client->get_player_id() : 0;
    bool local_player_exists = false;
    bool local_player_dead = false;
    bool local_player_spectating = false;
    bool has_alive_players = false;

    if (my_player_id > 0) {
        ctx.registry.view<engine::game::components::Owner>(
            [&](size_t eid, const auto& owner) {
                if (owner.player_id == my_player_id) {
                    local_player_exists = true;
                    ctx.local_player_entity = rtype::ecs::entity_t(static_cast<rtype::ecs::entity_id_t>(eid));
                    auto* spectator = ctx.registry.try_get<engine::game::components::Spectator>(
                        rtype::ecs::entity_t(static_cast<rtype::ecs::entity_id_t>(eid)));
                    if (spectator && spectator->is_spectating) local_player_spectating = true;
                    auto* lives = ctx.registry.try_get<engine::game::components::Lives>(
                        rtype::ecs::entity_t(static_cast<rtype::ecs::entity_id_t>(eid)));
                    if (lives && lives->remaining <= 0) local_player_dead = true;
                } else {
                    // Check if any other player is alive
                    auto* lives = ctx.registry.try_get<engine::game::components::Lives>(
                        rtype::ecs::entity_t(static_cast<rtype::ecs::entity_id_t>(eid)));
                    auto* spectator = ctx.registry.try_get<engine::game::components::Spectator>(
                        rtype::ecs::entity_t(static_cast<rtype::ecs::entity_id_t>(eid)));
                    bool is_spectating = spectator && spectator->is_spectating;
                    bool is_alive = lives && lives->remaining > 0;
                    if (is_alive && !is_spectating) {
                        has_alive_players = true;
                    }
                }
            });
    }
    
    // Show death menu if player died and hasn't seen it yet
    bool game_over = (ctx.had_player_before && !local_player_exists) ||
                     (local_player_dead && !ctx.death_menu_shown);
    
    // If player is spectating and all other players are now dead, show menu (even if they saw it before)
    if (local_player_spectating && !has_alive_players && ctx.game_state == GameState::InGame) {
        game_over = true;
    }
    
    if (local_player_exists) ctx.had_player_before = true;
    if (game_over) {
        // Get score and level from game stats
        std::uint32_t score = 0;
        std::uint16_t level = 1;
        std::uint16_t wave = 0;
        ctx.registry.view<engine::game::components::GameStats>(
            [&](size_t /*eid*/, const auto& stats) {
                score = stats.score;
                level = stats.current_level;
                wave = stats.wave;
            });
        
        // Save score to leaderboard (only once per death and only if score > 0)
        if (!ctx.score_saved && score > 0) {
            engine::game::ScoreEntry entry;
            entry.player_name = ctx.lobby_data.player_name;
            entry.score = score;
            entry.wave_reached = wave;
            entry.level_reached = level;
            ctx.leaderboard.add_entry(entry);
            ctx.leaderboard.save("config/leaderboard.cfg");
            ctx.score_saved = true;
        }
        
        ctx.game_state = GameState::DeathMenu;
        ctx.death_menu_shown = true;
        client::systems::build_death_menu_ui(ctx.death_menu_ui_registry, ctx.window.getSize(), has_alive_players, score, level);
        auto entity = ctx.death_menu_ui_registry.spawn_entity();
        engine::game::components::AccessibilityConfig cfg{};
        cfg.high_contrast = ctx.settings.high_contrast;
        cfg.font_scale_index = static_cast<std::size_t>(std::max(0, std::min(2, ctx.settings.font_scale_index)));
        ctx.death_menu_ui_registry.emplace_component<engine::game::components::AccessibilityConfig>(entity, cfg);
    }
}

bool process_frame(ClientContext& ctx) {
    float dt = ctx.delta_clock.restart().asSeconds();

    handle_snapshots(ctx);
    handle_fullscreen_and_vsync(ctx);

    ctx.starfield.update(ctx.star_registry, dt);
    ctx.background.update(dt);

    ctx.window.clear(ctx.game_state == GameState::InGame ? ctx.starfield.getBackgroundColor() : sf::Color::Black);
    render_state(ctx, dt);

    detect_game_over(ctx);
    ctx.audio_manager->update();
    ctx.window.display();
    update_fps(ctx);

    return ctx.window.isOpen();
}
