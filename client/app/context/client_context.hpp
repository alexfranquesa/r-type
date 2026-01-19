#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>

#include "app_runner.hpp"
#include "network_client.hpp"
#include "render_system.hpp"
#include "snapshot_apply_system.hpp"
#include "snapshot_receive_system.hpp"
#include "animation_system.hpp"
#include "particle_system.hpp"
#include "game_settings.hpp"
#include "ui_system.hpp"
#include "ui_render_system.hpp"
#include "main_menu_ui_system.hpp"
#include "settings_ui_system.hpp"
#include "settings_input_system.hpp"
#include "death_menu_ui_system.hpp"
#include "game_over_ui_system.hpp"
#include "scores_ui_system.hpp"
#include "help_ui_system.hpp"
#include "hud_system.hpp"
#include "heart_display_system.hpp"
#include "ultimate_display_system.hpp"
#include "starfield_system.hpp"
#include "audio_manager.hpp"
#include "background_system.hpp"
#include "client_prediction_system.hpp"
#include "../ui/ui_helpers.hpp"
#include "../sfml_renderer.hpp"
#include "engine/core/registry.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/sprite.hpp"
#include "engine/game/components/visual/animation.hpp"
#include "engine/game/components/visual/particle_effect.hpp"
#include "engine/game/components/ui/ui_transform.hpp"
#include "engine/game/components/ui/ui_text.hpp"
#include "engine/game/components/ui/ui_button.hpp"
#include "engine/game/components/ui/accessibility_config.hpp"
#include "engine/game/components/network/owner.hpp"
#include "engine/game/components/gameplay/lives.hpp"
#include "engine/game/components/gameplay/game_stats.hpp"
#include "engine/game/components/gameplay/spectator.hpp"
#include "engine/game/leaderboard.hpp"
#include "engine/game/components/gameplay/ultimate_charge.hpp"
#include "engine/render/rendering.hpp"

struct ClientContext {
    // Settings and state
    client::systems::GameSettings settings{};
    client::app::GameState game_state = client::app::GameState::MainMenu;
    client::app::LobbyData lobby_data{};
    bool connect_on_start = false;
    std::uint16_t input_mask = 0;
    int menu_selected_index = 0;
    bool settings_ui_dirty = true;
    std::optional<rtype::ecs::entity_t> local_player_entity;
    bool had_player_before = false;
    bool death_menu_shown = false;
    bool score_saved = false;

    // Window and view
    sf::RenderWindow window;
    sf::View virtual_view;
    sf::Font overlay_font;

    // Clocks / timers
    sf::Clock delta_clock;
    sf::Clock fps_clock;
    int fps_counter = 0;
    int current_fps = 0;
    bool last_fullscreen_state = false;
    bool last_vsync_state = false;
    int last_target_fps = 60;

    // Registries
    rtype::ecs::registry registry;
    rtype::ecs::registry menu_ui_registry;
    rtype::ecs::registry connect_ui_registry;
    rtype::ecs::registry settings_ui_registry;
    rtype::ecs::registry star_registry;
    rtype::ecs::registry death_menu_ui_registry;
    rtype::ecs::registry game_over_ui_registry;
    rtype::ecs::registry spectator_menu_ui_registry;
    rtype::ecs::registry scores_ui_registry;
    rtype::ecs::registry help_ui_registry;
    rtype::ecs::registry hud_registry;

    // Systems
    client::systems::RenderSystem render_system{"sprites"};
    std::unique_ptr<SfmlRenderer> renderer;  // IRenderer wrapper over RenderSystem
    client::systems::UISystem ui_system;
    std::unique_ptr<client::systems::UIRenderSystem> ui_render;
    std::unique_ptr<client::systems::HUDSystem> hud;
    client::systems::HeartDisplaySystem heart_display;
    client::systems::UltimateDisplaySystem ultimate_display;
    client::systems::SettingsUISystem settings_ui;
    client::systems::SettingsInputSystem settings_input;
    client::systems::SnapshotApplySystem snapshot_system;
    client::systems::AnimationSystem animation_system;
    client::systems::ParticleSystem particle_system;
    client::systems::ClientPredictionSystem prediction_system;
    client::systems::StarfieldSystem starfield{star_registry, 1280, 720};
    client::systems::BackgroundSystem background;
    std::unique_ptr<client::systems::AudioManager> audio_manager;
    engine::audio::IAudio* audio = nullptr;  // non-owning pointer to audio_manager

    std::unique_ptr<engine::net::INetworkClient> net_client;

    // Leaderboard
    client::systems::ScoresUISystem scores_ui;
    engine::game::Leaderboard leaderboard;
};

// Initialization
void initialize_context(ClientContext& ctx, int argc, char** argv);

// Per-frame handling
bool handle_events(ClientContext& ctx);
bool process_frame(ClientContext& ctx);
