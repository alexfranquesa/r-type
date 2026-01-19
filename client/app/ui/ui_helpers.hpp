#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

#include <SFML/Graphics.hpp>

#include "engine/core/registry.hpp"

namespace client::app {

// High-level client states.
enum class GameState {
    MainMenu,
    Settings,
    Scores,
    Help,
    Connect,
    InGame,
    DeathMenu,
    GameOver
};

enum class MainMenuAction {
    None,
    Play,
    Settings,
    Quit
};

struct MenuInput {
    bool up = false;
    bool down = false;
    bool enter = false;
};

struct LobbyData {
    std::string host = "127.0.0.1";
    std::uint16_t port = 4242;
    std::string player_name = "Player";
    std::uint16_t selected_level = 1;
};

constexpr int kMainMenuButtonCount = 5;
constexpr const char* kMainMenuPlayId = "play";
constexpr const char* kMainMenuScoresId = "scores";
constexpr const char* kMainMenuHelpId = "help";
constexpr const char* kMainMenuSettingsId = "settings";
constexpr const char* kMainMenuQuitId = "quit";
constexpr const char* kConnectId = "connect";
constexpr const char* kBackId = "back";
constexpr const char* kLevel1Id = "level1";
constexpr const char* kLevel2Id = "level2";
constexpr const char* kLevel3Id = "level3";
constexpr const char* kLevel4Id = "level4";
constexpr const char* kLevel5Id = "level5";

// Utilities
std::filesystem::path find_font();
std::uint16_t key_to_mask_bit(sf::Keyboard::Key key);

// UI helpers
void clear_ui_registry(rtype::ecs::registry& reg);
void build_connect_ui(rtype::ecs::registry& reg, sf::Vector2u window_size, const LobbyData& data);
void set_main_menu_selection(rtype::ecs::registry& reg, int selected_index, bool has_mouse_hover);
std::optional<int> hovered_main_menu_index(rtype::ecs::registry& reg);

}  // namespace client::app

