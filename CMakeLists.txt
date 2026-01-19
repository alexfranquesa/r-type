cmake_minimum_required(VERSION 3.20)

project(rtype VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =============================================================================
# CONFIGURACIÓN DE RUNTIME LIBRARY (Windows)
# =============================================================================
# Estrategia: Usar /MD (MultiThreaded DLL) en Release para que coincida
# con las librerías de vcpkg (x64-windows usa /MD por defecto)
# =============================================================================
if(MSVC)
    add_compile_options(/EHsc /FS)

    # Forzar uso de /MD (dinámico) en Release para evitar mismatch con SFML
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

    # Alternativa: Si usas x64-windows-static, descomentar siguiente línea:
    # set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)

option(RTYPE_BUILD_TESTS "Build unit tests" ON)  #tests desactivated

# =============================================================================
# FIND PACKAGES (vcpkg resolverá automáticamente)
# =============================================================================
find_package(SFML COMPONENTS System Window Graphics Audio Network REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(doctest CONFIG REQUIRED)

# =============================================================================
# ENGINE (librería estática que contiene toda la lógica core)
# =============================================================================
add_subdirectory(engine)

# =============================================================================
# SERVER EXECUTABLE
# =============================================================================
add_executable(rtype_server
    server/app/main.cpp
    server/app/network_server.cpp
    server/systems/apply_input_system.cpp
)

target_link_libraries(rtype_server
    PRIVATE
        rtype_engine
)

rtype_enable_warnings(rtype_server)

target_include_directories(rtype_server
    PRIVATE
        server/app
        server/systems
)

# IMPORTANTE: NO definir SFML_STATIC en targets que usan x64-windows (dinámico)
# Solo necesario si usas x64-windows-static triplet
# if(WIN32 AND VCPKG_TARGET_TRIPLET MATCHES "static")
#     target_compile_definitions(rtype_server PRIVATE SFML_STATIC)
# endif()

# =============================================================================
# CLIENT EXECUTABLE
# =============================================================================
add_executable(rtype_client
    client/app/main.cpp
    client/app/app_runner.cpp
    client/app/context/client_context.hpp
    client/app/context/client_context_init.cpp
    client/app/context/client_context_events.cpp
    client/app/context/client_context_render.cpp
    client/app/network_client.cpp
    client/app/ui/ui_helpers.cpp
    client/systems/src/render_system.cpp
    client/systems/src/accessibility_settings.cpp
    client/systems/src/accessibility_ui_system.cpp
    client/systems/src/game_settings.cpp
    client/systems/src/language_system.cpp
    client/systems/src/settings_ui_system.cpp
    client/systems/src/settings_input_system.cpp
    client/systems/src/animation_system.cpp
    client/systems/src/particle_system.cpp
    client/systems/src/snapshot_apply_system.cpp
    client/systems/src/snapshot_receive_system.cpp
    client/systems/src/lobby_ui_system.cpp
    client/systems/src/main_menu_ui_system.cpp
    client/systems/src/death_menu_ui_system.cpp
    client/systems/src/game_over_ui_system.cpp
    client/systems/src/scores_ui_system.cpp
    client/systems/src/help_ui_system.cpp
    client/systems/src/ui_system.cpp
    client/systems/src/ui_render_system.cpp
    client/systems/src/hud_system.cpp
    client/systems/src/heart_display_system.cpp
    client/systems/src/ultimate_display_system.cpp
    client/systems/src/starfield_system.cpp
    client/systems/src/audio_manager.cpp
    client/systems/src/background_system.cpp
    client/systems/src/client_prediction_system.cpp
    client/app/sfml_renderer.hpp
)

target_link_libraries(rtype_client
    PRIVATE
        rtype_engine
)

rtype_enable_warnings(rtype_client)

target_include_directories(rtype_client
    PRIVATE
        client/app
        client/systems
        client/systems/include
)

# IMPORTANTE: Ver comentario anterior sobre SFML_STATIC
# if(WIN32 AND VCPKG_TARGET_TRIPLET MATCHES "static")
#     target_compile_definitions(rtype_client PRIVATE SFML_STATIC)
# endif()

# =============================================================================
# TESTS EXECUTABLE
# =============================================================================
if(RTYPE_BUILD_TESTS)
    enable_testing()

    add_executable(rtype_tests
        client/tests/test_render_resources.cpp
        client/systems/src/render_system.cpp
        client/systems/src/snapshot_apply_system.cpp
        client/systems/src/hud_system.cpp
        client/systems/src/heart_display_system.cpp
        testing/ecs_registry_tests.cpp
        testing/movement_system_tests.cpp
        testing/snapshot_apply_tests.cpp
        testing/shoot_cooldown_tests.cpp
    )

    target_link_libraries(rtype_tests
        PRIVATE
            rtype_engine
            doctest::doctest
    )

    target_include_directories(rtype_tests
        PRIVATE
            client/app
            client/systems/include
            engine/core/include
            engine/game/include
            engine/net/include
            testing
    )

    rtype_enable_warnings(rtype_tests)

    # IMPORTANTE: Ver comentario anterior sobre SFML_STATIC
    # if(WIN32 AND VCPKG_TARGET_TRIPLET MATCHES "static")
    #     target_compile_definitions(rtype_tests PRIVATE SFML_STATIC)
    # endif()

    add_test(NAME rtype_tests COMMAND rtype_tests)
endif()

# =============================================================================
# INFORMACIÓN DE BUILD (para debugging)
# =============================================================================
message(STATUS "========================================")
message(STATUS "R-Type Build Configuration")
message(STATUS "========================================")
message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
if(MSVC)
    message(STATUS "MSVC Runtime: ${CMAKE_MSVC_RUNTIME_LIBRARY}")
    message(STATUS "VCPKG Triplet: ${VCPKG_TARGET_TRIPLET}")
endif()
message(STATUS "SFML version: ${SFML_VERSION}")
message(STATUS "Build tests: ${RTYPE_BUILD_TESTS}")
message(STATUS "========================================")
