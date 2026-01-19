#include "client_context.hpp"

#include <algorithm>

#include "../ui/ui_helpers.hpp"
#include "engine/core/engine_core.hpp"
#include "engine/game/game_api.hpp"
#include "localization.hpp"

using namespace client::app;

namespace {

void add_accessibility_config(rtype::ecs::registry& reg, const client::systems::GameSettings& settings) {
    auto entity = reg.spawn_entity();
    engine::game::components::AccessibilityConfig cfg{};
    cfg.high_contrast = settings.high_contrast;
    cfg.font_scale_index = static_cast<std::size_t>(std::max(0, std::min(2, settings.font_scale_index)));
    reg.emplace_component<engine::game::components::AccessibilityConfig>(entity, cfg);
}

void register_components(ClientContext& ctx) {
    ctx.registry.register_component<engine::game::components::Position>();
    ctx.registry.register_component<engine::game::components::Sprite>();
    ctx.registry.register_component<engine::game::components::Animation>();
    ctx.registry.register_component<engine::game::components::ParticleEffect>();
    ctx.registry.register_component<engine::game::components::AccessibilityConfig>();
    ctx.registry.register_component<engine::game::components::Spectator>();
    ctx.registry.register_component<engine::game::components::Owner>();
    ctx.registry.register_component<engine::game::components::GameStats>();
    ctx.registry.register_component<engine::game::components::UltimateCharge>();
    add_accessibility_config(ctx.registry, ctx.settings);

    ctx.hud_registry.register_component<engine::game::components::Position>();
    ctx.hud_registry.register_component<engine::game::components::Sprite>();
}

void load_audio(ClientContext& ctx) {
    ctx.audio_manager->load_menu_music();
    ctx.audio_manager->load_game_music();
    ctx.audio_manager->load_button_click_sound();
    const float music_vol = ctx.settings.master_volume * ctx.settings.music_volume * 100.f;
    const float sfx_vol   = ctx.settings.master_volume * ctx.settings.sfx_volume   * 100.f;
    ctx.audio_manager->set_music_volume(music_vol);
    ctx.audio_manager->set_sfx_volume(sfx_vol);
    ctx.snapshot_system.set_sfx_volume(ctx.settings.master_volume * ctx.settings.sfx_volume * 100.f);
}

}  // namespace

void initialize_context(ClientContext& ctx, int argc, char** argv) {
    engine::core::initialize();
    engine::render::initialize();
    engine::game::initialize();

    ctx.render_system.preload_all();
    ctx.renderer = std::make_unique<SfmlRenderer>(ctx.render_system, ctx.registry, ctx.window);

    ctx.settings.load_from_file();
    
    // Initialize localization system with user's language setting
    client::systems::Localization::setLanguage(static_cast<client::systems::Language>(ctx.settings.language));
    
    ctx.lobby_data.host = ctx.settings.default_server_ip;
    ctx.lobby_data.port = static_cast<std::uint16_t>(ctx.settings.default_port);
    ctx.lobby_data.player_name = ctx.settings.player_name;
    ctx.connect_on_start = ctx.settings.auto_connect;

    if (argc >= 2) {
        ctx.lobby_data.host = argv[1];
        ctx.connect_on_start = true;
    }
    if (argc >= 3) {
        try {
            int potential_port = std::stoi(argv[2]);
            if (potential_port > 0 && potential_port <= 65535) {
                ctx.lobby_data.port = static_cast<std::uint16_t>(potential_port);
                if (argc >= 4) ctx.lobby_data.player_name = argv[3];
            } else {
                ctx.lobby_data.player_name = argv[2];
            }
        } catch (const std::invalid_argument&) {
            ctx.lobby_data.player_name = argv[2];
        }
    }

    const auto window_state = ctx.settings.fullscreen ? sf::State::Fullscreen : sf::State::Windowed;
    ctx.window.create(sf::VideoMode({1280U, 720U}), "R-TYPE Client", sf::Style::Close, window_state);
    ctx.window.setFramerateLimit(static_cast<unsigned>(ctx.settings.target_fps));
    ctx.window.setVerticalSyncEnabled(ctx.settings.vsync);
    ctx.last_target_fps = ctx.settings.target_fps;
    ctx.virtual_view = sf::View(sf::FloatRect({0.f, 0.f}, {1280.f, 720.f}));
    ctx.window.setView(ctx.virtual_view);

    const auto font_path = find_font();
    if (!font_path.empty()) {
        (void)ctx.overlay_font.openFromFile(font_path);
    }

    ctx.ui_render = std::make_unique<client::systems::UIRenderSystem>(ctx.overlay_font);
    ctx.hud = std::make_unique<client::systems::HUDSystem>(ctx.overlay_font);

    ctx.starfield.reinitialize(ctx.star_registry);
    ctx.background.load_texture("sprites/background.png");

    register_components(ctx);
    ctx.menu_ui_registry = rtype::ecs::registry{};
    client::systems::build_main_menu_ui(ctx.menu_ui_registry, ctx.window.getSize());
    ctx.connect_ui_registry = rtype::ecs::registry{};
    build_connect_ui(ctx.connect_ui_registry, ctx.window.getSize(), ctx.lobby_data);
    ctx.settings_ui.build_ui(ctx.settings_ui_registry, ctx.window.getSize(), ctx.settings);
    client::systems::build_death_menu_ui(ctx.death_menu_ui_registry, ctx.window.getSize());
    client::systems::build_game_over_ui(ctx.game_over_ui_registry, ctx.window.getSize());

    ctx.last_fullscreen_state = ctx.settings.fullscreen;
    ctx.last_vsync_state = ctx.settings.vsync;

    ctx.audio_manager = std::make_unique<client::systems::AudioManager>();
    ctx.audio = ctx.audio_manager.get();
    load_audio(ctx);

    // Load leaderboard
    ctx.leaderboard.load("config/leaderboard.cfg");

    if (ctx.connect_on_start) {
        ctx.net_client = std::make_unique<NetworkClient>(ctx.lobby_data.host, ctx.lobby_data.port);
        if (!ctx.net_client->connect(ctx.lobby_data.player_name, ctx.lobby_data.selected_level, static_cast<std::uint8_t>(ctx.settings.difficulty))) {
            ctx.net_client.reset();
        } else {
            ctx.game_state = GameState::InGame;
            ctx.audio_manager->play_game_music();
        }
    } else {
        ctx.audio_manager->play_menu_music();
    }
}
