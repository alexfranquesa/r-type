add_library(rtype_engine STATIC
    core/src/core_init.cpp
    net/src/net_init.cpp
    render/src/render_init.cpp
    render/src/texture_loader.cpp
    render/src/sprite_bank.cpp
    game/src/game_init.cpp
    game/src/leaderboard.cpp
    game/src/world/movement_system.cpp
    game/src/world/cleanup_system.cpp
    game/src/gameplay/shooting_system.cpp
    game/src/gameplay/projectile_system.cpp
    game/src/gameplay/collision_system.cpp
    game/src/gameplay/health_system.cpp
    game/src/gameplay/game_over_system.cpp
    game/src/gameplay/enemy_spawn_system.cpp
    game/src/gameplay/enemy_shooting_system.cpp
    game/src/gameplay/movement_pattern_system.cpp
    game/src/gameplay/score_system.cpp
    game/src/gameplay/level_manager.cpp
    game/src/gameplay/lava_drop_spawn_system.cpp
    game/src/gameplay/asteroid_spawn_system.cpp
    game/src/gameplay/ice_enemy_spawn_system.cpp
    game/src/gameplay/boss_spawn_system.cpp
    game/src/gameplay/boss_behavior_system.cpp
    game/src/gameplay/ultimate_charge_system.cpp
    game/src/gameplay/ultimate_activation_system.cpp
    game/src/network/network_send_system.cpp
)

target_include_directories(rtype_engine
    PUBLIC
        core/include
        net/include
        render/include
        game/include
        audio/include
)

target_link_libraries(rtype_engine
    PUBLIC
        SFML::System
        SFML::Window
        SFML::Graphics
        SFML::Audio
        SFML::Network
        asio::asio
        fmt::fmt
        spdlog::spdlog
)

target_compile_features(rtype_engine PUBLIC cxx_std_20)
