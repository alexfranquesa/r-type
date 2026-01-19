#include "client_context.hpp"

#include <SFML/Window.hpp>
#include <algorithm>

#include "../ui/ui_helpers.hpp"

using namespace client::app;

namespace {

void add_accessibility(rtype::ecs::registry& reg, const client::systems::GameSettings& settings) {
    auto entity = reg.spawn_entity();
    engine::game::components::AccessibilityConfig cfg{};
    cfg.high_contrast = settings.high_contrast;
    cfg.font_scale_index = static_cast<std::size_t>(std::max(0, std::min(2, settings.font_scale_index)));
    reg.emplace_component<engine::game::components::AccessibilityConfig>(entity, cfg);
}

void send_input_with_prediction(ClientContext& ctx, std::uint16_t input_mask, float dt) {
    if (!ctx.net_client) return;
    
    ctx.net_client->send_input(input_mask);
    
    // Apply client-side prediction for local player
    if (ctx.local_player_entity) {
        const auto* net_client = dynamic_cast<NetworkClient*>(ctx.net_client.get());
        if (net_client) {
            const std::uint32_t input_sequence = net_client->get_last_input_sequence();
            ctx.prediction_system.apply_input_locally(
                ctx.registry,
                static_cast<std::uint16_t>(ctx.local_player_entity->id()),
                input_mask,
                input_sequence,
                dt
            );
        }
    }
}

void apply_accessibility_to_all(ClientContext& ctx) {
    add_accessibility(ctx.menu_ui_registry, ctx.settings);
    add_accessibility(ctx.connect_ui_registry, ctx.settings);
    add_accessibility(ctx.settings_ui_registry, ctx.settings);
    add_accessibility(ctx.registry, ctx.settings);
}

void log_unhandled_button(const char* state, const std::string& id) {
    std::cout << "[ui] Unhandled button id '" << id << "' in state " << state << std::endl;
}

}  // namespace

static void cleanup_and_return_to_menu(ClientContext& ctx, const char* reason) {
    std::cout << "\n========== CLEANUP START (" << reason << ") ==========" << std::endl;

    ctx.local_player_entity.reset();
    ctx.had_player_before = false;
    ctx.death_menu_shown = false;
    ctx.score_saved = false;

    if (ctx.net_client) {
        std::cout << "[rtype_client] Shutting down network..." << std::endl;
        ctx.net_client->shutdown();
        ctx.net_client.reset();
        std::cout << "[rtype_client] Network shutdown complete" << std::endl;
    }

    ctx.input_mask = 0;

    ctx.registry.clear();
    ctx.star_registry.clear();
    ctx.starfield.reinitialize(ctx.star_registry);
    ctx.snapshot_system.reset();
    ctx.hud->reset_transition();

    // Clear UI system state to prevent button events from carrying over
    ctx.ui_system.consume_pressed();

    ctx.game_state = GameState::MainMenu;
    ctx.audio_manager->play_menu_music();
    std::cout << "[rtype_client] Cleanup complete, back to main menu" << std::endl;
    std::cout << "========== CLEANUP END ==========\n" << std::endl;
}

bool handle_events(ClientContext& ctx) {
    [[maybe_unused]] MenuInput menu_input{};
    MainMenuAction menu_action = MainMenuAction::None;
    client::systems::SettingsInputState settings_input_state{};
    settings_input_state.mouse_pos = ctx.window.mapPixelToCoords(sf::Mouse::getPosition(ctx.window));
    settings_input_state.mouse_down = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

    while (auto event = ctx.window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            ctx.window.close();
            return false;
        }
        if (const auto* focus_lost = event->getIf<sf::Event::FocusLost>()) {
            (void)focus_lost;
            if (ctx.game_state == GameState::InGame && ctx.net_client) {
                ctx.input_mask = 0;
                ctx.net_client->send_input(ctx.input_mask);
            }
        }
        if (const auto* focus_gained = event->getIf<sf::Event::FocusGained>()) {
            (void)focus_gained;
            if (ctx.game_state == GameState::InGame && ctx.net_client) {
                ctx.net_client->send_input(ctx.input_mask);
            }
        }

        if (ctx.game_state == GameState::MainMenu) {
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                sf::Vector2u new_size{resized->size.x, resized->size.y};
                ctx.window.setView(ctx.virtual_view);
                client::systems::build_main_menu_ui(ctx.menu_ui_registry, new_size);
                add_accessibility(ctx.menu_ui_registry, ctx.settings);
                set_main_menu_selection(ctx.menu_ui_registry, ctx.menu_selected_index, false);
            }
            if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
                if (key_pressed->code == sf::Keyboard::Key::Up) menu_input.up = true;
                else if (key_pressed->code == sf::Keyboard::Key::Down) menu_input.down = true;
                else if (key_pressed->code == sf::Keyboard::Key::Enter) menu_input.enter = true;
            }
        } else if (ctx.game_state == GameState::Connect) {
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                sf::Vector2u new_size{resized->size.x, resized->size.y};
                ctx.window.setView(ctx.virtual_view);
                build_connect_ui(ctx.connect_ui_registry, new_size, ctx.lobby_data);
                add_accessibility(ctx.connect_ui_registry, ctx.settings);
            }
            if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
                if (key_pressed->code == sf::Keyboard::Key::Escape) ctx.game_state = GameState::MainMenu;
            }
        } else if (ctx.game_state == GameState::Settings) {
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                sf::Vector2u new_size{resized->size.x, resized->size.y};
                (void)new_size;
                ctx.window.setView(ctx.virtual_view);
                ctx.settings_ui_dirty = true;
            }
            if (const auto* mouse_pressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouse_pressed->button == sf::Mouse::Button::Left) settings_input_state.mouse_pressed = true;
            }
            if (const auto* mouse_released = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouse_released->button == sf::Mouse::Button::Left) settings_input_state.mouse_released = true;
            }
            if (const auto* text_entered = event->getIf<sf::Event::TextEntered>()) {
                settings_input_state.text_entered.push_back(text_entered->unicode);
            }
            if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
                if (key_pressed->code == sf::Keyboard::Key::Escape) settings_input_state.escape = true;
                if (key_pressed->code == sf::Keyboard::Key::Backspace) settings_input_state.backspace = true;
                else if (key_pressed->code == sf::Keyboard::Key::Left) settings_input_state.left = true;
                else if (key_pressed->code == sf::Keyboard::Key::Right) settings_input_state.right = true;
                else if (key_pressed->code == sf::Keyboard::Key::Enter) settings_input_state.enter = true;
            }
        } else if (ctx.game_state == GameState::InGame) {
            if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
                if (key_pressed->code == sf::Keyboard::Key::Escape) {
                    cleanup_and_return_to_menu(ctx, "escape key");
                    break;
                } else {
                    auto bit = key_to_mask_bit(key_pressed->code);
                    if (bit != 0) {
                        ctx.input_mask |= bit;
                        const float dt = ctx.delta_clock.restart().asSeconds();
                        send_input_with_prediction(ctx, ctx.input_mask, dt);
                        std::cout << "[input] Key pressed, mask=" << ctx.input_mask << std::endl;
                    }
                }
            }
            if (const auto* key_released = event->getIf<sf::Event::KeyReleased>()) {
                auto bit = key_to_mask_bit(key_released->code);
                if (bit != 0) {
                    ctx.input_mask &= static_cast<std::uint16_t>(~bit);
                    const float dt = ctx.delta_clock.restart().asSeconds();
                    send_input_with_prediction(ctx, ctx.input_mask, dt);
                    std::cout << "[input] Key released, mask=" << ctx.input_mask << std::endl;
                }
            }
            if (const auto* mouse_pressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouse_pressed->button == sf::Mouse::Button::Left) {
                    const auto mouse_pixel_pos = sf::Mouse::getPosition(ctx.window);
                    const auto mouse_world_pos = ctx.window.mapPixelToCoords(mouse_pixel_pos);
                    if (ctx.hud->is_pause_button_clicked(mouse_world_pos)) {
                        constexpr std::uint16_t INPUT_PAUSE = 1 << 5;
                        ctx.input_mask |= INPUT_PAUSE;
                        const float dt = ctx.delta_clock.restart().asSeconds();
                        send_input_with_prediction(ctx, ctx.input_mask, dt);
                        std::cout << "[input] Pause button clicked, mask=" << ctx.input_mask << std::endl;
                        ctx.input_mask &= static_cast<std::uint16_t>(~INPUT_PAUSE);
                    }
                }
            }
        } else if (ctx.game_state == GameState::DeathMenu) {
            if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
                if (key_pressed->code == sf::Keyboard::Key::Escape) {
                    cleanup_and_return_to_menu(ctx, "death menu - return to menu");
                    break;
                }
            }
        } else if (ctx.game_state == GameState::GameOver) {
            if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
                if (key_pressed->code == sf::Keyboard::Key::Escape) {
                    cleanup_and_return_to_menu(ctx, "game over - return to menu");
                    break;
                }
            }
        } else if (ctx.game_state == GameState::Scores) {
            if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
                if (key_pressed->code == sf::Keyboard::Key::Escape) {
                    ctx.audio_manager->play_button_click();
                    ctx.game_state = GameState::MainMenu;
                    break;
                }
            }
        } else if (ctx.game_state == GameState::Help) {
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                sf::Vector2u new_size{resized->size.x, resized->size.y};
                ctx.window.setView(ctx.virtual_view);
                client::systems::build_help_ui(ctx.help_ui_registry, new_size);
                add_accessibility(ctx.help_ui_registry, ctx.settings);
            }
            if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
                if (key_pressed->code == sf::Keyboard::Key::Escape) {
                    ctx.audio_manager->play_button_click();
                    ctx.game_state = GameState::MainMenu;
                    client::systems::build_main_menu_ui(ctx.menu_ui_registry, ctx.window.getSize());
                    break;
                }
            }
        }
    }

    // Handle state logic outside the event loop (simplified: keep original behaviour)
    if (ctx.game_state == GameState::MainMenu) {
        ctx.ui_system.update(ctx.menu_ui_registry, ctx.window);
        const auto hovered_index = hovered_main_menu_index(ctx.menu_ui_registry);
        if (hovered_index) ctx.menu_selected_index = *hovered_index;
        set_main_menu_selection(ctx.menu_ui_registry, ctx.menu_selected_index, hovered_index.has_value());

        // Minimal: keep enter navigation
        for (const auto& id : ctx.ui_system.consume_pressed()) {
            if (id == kMainMenuPlayId) menu_action = MainMenuAction::Play;
            else if (id == kMainMenuScoresId) {
                ctx.audio_manager->play_button_click();
                ctx.game_state = GameState::Scores;
                ctx.scores_ui.build_ui(ctx.scores_ui_registry, ctx.leaderboard);
            }
            else if (id == kMainMenuHelpId) {
                ctx.audio_manager->play_button_click();
                ctx.game_state = GameState::Help;
                client::systems::build_help_ui(ctx.help_ui_registry, ctx.window.getSize());
            }
            else if (id == kMainMenuSettingsId) menu_action = MainMenuAction::Settings;
            else if (id == kMainMenuQuitId) menu_action = MainMenuAction::Quit;
            else log_unhandled_button("MainMenu", id);
        }

        if (menu_action == MainMenuAction::Play) {
        ctx.audio_manager->play_button_click();
            ctx.game_state = GameState::Connect;
            build_connect_ui(ctx.connect_ui_registry, ctx.window.getSize(), ctx.lobby_data);
        } else if (menu_action == MainMenuAction::Settings) {
            ctx.audio_manager->play_button_click();
            ctx.game_state = GameState::Settings;
            ctx.settings_ui_dirty = true;
        } else if (menu_action == MainMenuAction::Quit) {
                ctx.audio_manager->play_button_click();
            ctx.window.close();
            return false;
        }
    } else if (ctx.game_state == GameState::Scores) {
        ctx.ui_system.update(ctx.scores_ui_registry, ctx.window);
        for (const auto& id : ctx.ui_system.consume_pressed()) {
            if (id == "back_to_menu") {
                ctx.audio_manager->play_button_click();
                ctx.game_state = GameState::MainMenu;
                client::systems::build_main_menu_ui(ctx.menu_ui_registry, ctx.window.getSize());
                // Clear ALL pending button presses to prevent accidental QUIT
                ctx.ui_system.consume_pressed();
                ctx.ui_system.consume_pressed();
                ctx.ui_system.consume_pressed();
                return true; // Skip processing other states this frame
            } else {
                log_unhandled_button("Scores", id);
            }
        }
    } else if (ctx.game_state == GameState::Help) {
        ctx.ui_system.update(ctx.help_ui_registry, ctx.window);
        for (const auto& id : ctx.ui_system.consume_pressed()) {
            if (id == "back_to_menu") {
                ctx.audio_manager->play_button_click();
                ctx.game_state = GameState::MainMenu;
                client::systems::build_main_menu_ui(ctx.menu_ui_registry, ctx.window.getSize());
                // Clear ALL pending button presses to prevent accidental QUIT
                ctx.ui_system.consume_pressed();
                ctx.ui_system.consume_pressed();
                ctx.ui_system.consume_pressed();
                return true; // Skip processing other states this frame
            } else {
                log_unhandled_button("Help", id);
            }
        }
    } else if (ctx.game_state == GameState::Connect) {
        ctx.ui_system.update(ctx.connect_ui_registry, ctx.window);
        bool rebuild_connect_ui = false;
        bool return_to_menu = false;
        for (const auto& id : ctx.ui_system.consume_pressed()) {
            if (id == kConnectId) {
                ctx.net_client = std::make_unique<NetworkClient>(ctx.lobby_data.host, ctx.lobby_data.port);
                if (ctx.net_client->connect(ctx.lobby_data.player_name, ctx.lobby_data.selected_level, static_cast<std::uint8_t>(ctx.settings.difficulty))) {
                    ctx.game_state = GameState::InGame;
                    ctx.had_player_before = false;
                    ctx.death_menu_shown = false;
                } else {
                    ctx.net_client.reset();
                }
            } else if (id == kBackId) {
                ctx.audio_manager->play_button_click();
                ctx.game_state = GameState::MainMenu;
                client::systems::build_main_menu_ui(ctx.menu_ui_registry, ctx.window.getSize());
                // Clear ALL pending button presses to prevent accidental QUIT
                ctx.ui_system.consume_pressed();
                ctx.ui_system.consume_pressed();
                ctx.ui_system.consume_pressed();
                return_to_menu = true;
            } else if (id == kLevel1Id) { ctx.lobby_data.selected_level = 1; rebuild_connect_ui = true; }
            else if (id == kLevel2Id) { ctx.lobby_data.selected_level = 2; rebuild_connect_ui = true; }
            else if (id == kLevel3Id) { ctx.lobby_data.selected_level = 3; rebuild_connect_ui = true; }
            else if (id == kLevel4Id) { ctx.lobby_data.selected_level = 4; rebuild_connect_ui = true; }
            else if (id == kLevel5Id) { ctx.lobby_data.selected_level = 5; rebuild_connect_ui = true; }
            else { log_unhandled_button("Connect", id); }
        }
        if (return_to_menu) return true; // Skip processing other states this frame
        if (rebuild_connect_ui) build_connect_ui(ctx.connect_ui_registry, ctx.window.getSize(), ctx.lobby_data);
        if (ctx.connect_on_start) {
            ctx.connect_on_start = false;
            ctx.net_client = std::make_unique<NetworkClient>(ctx.lobby_data.host, ctx.lobby_data.port);
            if (ctx.net_client->connect(ctx.lobby_data.player_name, ctx.lobby_data.selected_level, static_cast<std::uint8_t>(ctx.settings.difficulty))) {
                ctx.game_state = GameState::InGame;
                ctx.had_player_before = false;
                ctx.death_menu_shown = false;
            } else {
                ctx.net_client.reset();
            }
        }
    } else if (ctx.game_state == GameState::Settings) {
        if (ctx.settings_ui_dirty) {
            ctx.settings_ui.build_ui(ctx.settings_ui_registry, ctx.window.getSize(), ctx.settings);
            auto config_entity = ctx.settings_ui_registry.spawn_entity();
            engine::game::components::AccessibilityConfig access_config{};
            access_config.high_contrast = ctx.settings.high_contrast;
            access_config.font_scale_index = static_cast<std::size_t>(std::max(0, std::min(2, ctx.settings.font_scale_index)));
            ctx.settings_ui_registry.emplace_component<engine::game::components::AccessibilityConfig>(
                config_entity, access_config);
            ctx.settings_ui_dirty = false;
        }

        const auto settings_action = ctx.settings_input.update(ctx.settings_ui_registry, ctx.settings, settings_input_state);
        ctx.lobby_data.host = ctx.settings.default_server_ip;
        ctx.lobby_data.port = static_cast<std::uint16_t>(ctx.settings.default_port);
        ctx.lobby_data.player_name = ctx.settings.player_name;

        if (settings_action != client::systems::SettingsMenuAction::None) {
            apply_accessibility_to_all(ctx);
            if (settings_action == client::systems::SettingsMenuAction::Apply) {
                ctx.settings_ui_dirty = true;
                const float new_music_vol = ctx.settings.master_volume * ctx.settings.music_volume * 100.f;
                const float new_sfx_vol   = ctx.settings.master_volume * ctx.settings.sfx_volume   * 100.f;
                ctx.audio_manager->set_music_volume(new_music_vol);
                ctx.audio_manager->set_sfx_volume(new_sfx_vol);
                ctx.snapshot_system.set_sfx_volume(ctx.settings.master_volume * ctx.settings.sfx_volume * 100.f);
                ctx.window.setFramerateLimit(static_cast<unsigned>(ctx.settings.target_fps));
                ctx.window.setVerticalSyncEnabled(ctx.settings.vsync);
            }
        }

        if (settings_action == client::systems::SettingsMenuAction::Cancel) {
            ctx.audio_manager->play_button_click();
            ctx.game_state = GameState::MainMenu;
            ctx.settings_ui_dirty = true;
            // Rebuild main menu UI to reflect language changes
            client::systems::build_main_menu_ui(ctx.menu_ui_registry, ctx.window.getSize());
            build_connect_ui(ctx.connect_ui_registry, ctx.window.getSize(), ctx.lobby_data);
            // Clear ALL pending button presses to prevent accidental QUIT
            ctx.ui_system.consume_pressed();
            ctx.ui_system.consume_pressed();
            ctx.ui_system.consume_pressed();
            return true; // Skip processing other states this frame
        }
    } else if (ctx.game_state == GameState::DeathMenu) {
        ctx.ui_system.update(ctx.death_menu_ui_registry, ctx.window);
        bool clicked_spectate = false;
        bool clicked_go_back = false;
        for (const auto& id : ctx.ui_system.consume_pressed()) {
            if (id == "spectate") clicked_spectate = true;
            else if (id == "go_back_menu") clicked_go_back = true;
            else log_unhandled_button("DeathMenu", id);
        }
        if (clicked_spectate) {
            ctx.audio_manager->play_button_click();
            // Continue as spectator - return to game state
            // The server has already marked the player as spectator, just return to viewing the game
            ctx.game_state = GameState::InGame;
            // Don't reset death_menu_shown - if all players die while spectating, we'll show menu again
        } else if (clicked_go_back) {
            ctx.audio_manager->play_button_click();
            cleanup_and_return_to_menu(ctx, "death menu - go back button");
        }
    } else if (ctx.game_state == GameState::GameOver) {
        ctx.ui_system.update(ctx.game_over_ui_registry, ctx.window);
        bool clicked_play_again = false;
        bool clicked_go_back = false;
        for (const auto& id : ctx.ui_system.consume_pressed()) {
            if (id == "play_again") clicked_play_again = true;
            else if (id == "go_back_menu") clicked_go_back = true;
            else log_unhandled_button("GameOver", id);
        }
        if (clicked_play_again) {
            ctx.audio_manager->play_button_click();

            // Reset all game state (same as cleanup_and_return_to_menu, minus network shutdown)
            ctx.local_player_entity.reset();
            ctx.had_player_before = false;
            ctx.death_menu_shown = false;
            ctx.score_saved = false;
            ctx.input_mask = 0;

            ctx.registry = rtype::ecs::registry{};
            ctx.star_registry.clear();
            ctx.starfield.reinitialize(ctx.star_registry);
            ctx.snapshot_system.reset();      // CRITICAL: clears cached HP state
            ctx.hud->reset_transition();
            ctx.ui_system.consume_pressed();

            ctx.game_state = GameState::InGame;
            ctx.audio_manager->play_game_music();

            if (ctx.net_client) ctx.net_client->send_input(0);
        } else if (clicked_go_back) {
            ctx.audio_manager->play_button_click();
            cleanup_and_return_to_menu(ctx, "game over - go back button");
        }
    }

    return true;
}
