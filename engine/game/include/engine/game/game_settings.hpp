#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

namespace engine::game {

struct GameSettings {
    float master_volume = 1.0f;
    float music_volume = 0.8f;
    float sfx_volume = 0.9f;
    bool music_enabled = true;

    int difficulty = 1;

    int enemies_per_wave = 15;
    int kills_per_wave = 30;
    int player_lives = 3;
    bool infinite_lives = false;
    float enemy_spawn_rate = 1.0f;
    
    // Level-based multipliers (set by server based on current level)
    float level_enemy_speed_mult = 1.0f;
    float level_enemy_hp_mult = 1.0f;
    float level_spawn_rate_mult = 1.0f;

    bool fullscreen = false;
    bool vsync = true;
    bool show_fps = false;
    bool screen_shake = true;
    int target_fps = 60;  // Target FPS limit (60-300)

    std::string default_server_ip = "127.0.0.1";
    int default_port = 4242;
    std::string player_name = "Player";
    bool auto_connect = false;

    // Accessibility settings
    bool high_contrast = false;
    int font_scale_index = 0;  // 0=1.0x, 1=1.25x, 2=1.5x
    int language = 0;  // 0=English, 1=Spanish, 2=French

    float get_font_scale() const {
        static constexpr float scales[] = {1.0f, 1.25f, 1.5f};
        if (font_scale_index < 0 || font_scale_index >= 3) {
            return 1.0f;
        }
        return scales[font_scale_index];
    }

    int get_enemies_per_wave() const {
        switch (difficulty) {
            case 0: return 10;   // Easy
            case 1: return 15;   // Normal
            case 2: return 20;   // Hard
            case 3: return 30;   // Hardcore
            default: return 15;  // Default to Normal
        }
    }

    bool load_from_file(const std::string& path = "config/game_settings.cfg") {
        const std::filesystem::path file_path(path);
        if (!std::filesystem::exists(file_path)) {
            *this = GameSettings{};
            return save_to_file(path);
        }

        std::ifstream file(file_path);
        if (!file.is_open()) {
            return false;
        }

        *this = GameSettings{};

        std::string line;
        while (std::getline(file, line)) {
            std::string trimmed = trim(line);
            if (trimmed.empty() || trimmed[0] == '#') {
                continue;
            }

            auto pos = trimmed.find('=');
            if (pos == std::string::npos) {
                continue;
            }

            std::string key = trim(trimmed.substr(0, pos));
            std::string value = trim(trimmed.substr(pos + 1));

            if (key == "master_volume") {
                master_volume = std::stof(value);
            } else if (key == "music_volume") {
                music_volume = std::stof(value);
            } else if (key == "sfx_volume") {
                sfx_volume = std::stof(value);
            } else if (key == "music_enabled") {
                music_enabled = parse_bool(value);
            } else if (key == "difficulty") {
                difficulty = std::stoi(value);
            } else if (key == "enemies_per_wave") {
                enemies_per_wave = std::stoi(value);
            } else if (key == "kills_per_wave") {
                kills_per_wave = std::stoi(value);
            } else if (key == "player_lives") {
                player_lives = std::stoi(value);
            } else if (key == "infinite_lives") {
                infinite_lives = parse_bool(value);
            } else if (key == "enemy_spawn_rate") {
                enemy_spawn_rate = std::stof(value);
            } else if (key == "fullscreen") {
                fullscreen = parse_bool(value);
            } else if (key == "vsync") {
                vsync = parse_bool(value);
            } else if (key == "show_fps") {
                show_fps = parse_bool(value);
            } else if (key == "screen_shake") {
                screen_shake = parse_bool(value);
            } else if (key == "target_fps") {
                target_fps = std::stoi(value);
            } else if (key == "default_server_ip") {
                default_server_ip = value;
            } else if (key == "default_port") {
                default_port = std::stoi(value);
            } else if (key == "player_name") {
                player_name = value;
            } else if (key == "auto_connect") {
                auto_connect = parse_bool(value);
            } else if (key == "high_contrast") {
                high_contrast = parse_bool(value);
            } else if (key == "font_scale_index") {
                font_scale_index = std::stoi(value);
            } else if (key == "language") {
                language = std::stoi(value);
            }
        }

        return true;
    }

    bool save_to_file(const std::string& path = "config/game_settings.cfg") const {
        const std::filesystem::path file_path(path);
        if (file_path.has_parent_path()) {
            std::filesystem::create_directories(file_path.parent_path());
        }

        std::ofstream file(file_path);
        if (!file.is_open()) {
            return false;
        }

        file << "# R-Type Game Settings Configuration File\n";
        file << "# Format: key=value (no spaces around =)\n\n";

        file << "# ============ AUDIO SETTINGS ============\n";
        write_float(file, "master_volume", master_volume);
        write_float(file, "music_volume", music_volume);
        write_float(file, "sfx_volume", sfx_volume);
        file << "music_enabled=" << (music_enabled ? "true" : "false") << "\n\n";

        file << "# ============ DIFFICULTY SETTINGS ============\n";
        file << "# 0=Easy, 1=Normal, 2=Hard, 3=Hardcore\n";
        file << "difficulty=" << difficulty << "\n\n";

        file << "# ============ GAMEPLAY SETTINGS ============\n";
        file << "enemies_per_wave=" << enemies_per_wave << "\n";
        file << "kills_per_wave=" << kills_per_wave << "\n";
        file << "player_lives=" << player_lives << "\n";
        file << "infinite_lives=" << (infinite_lives ? "true" : "false") << "\n";
        write_float(file, "enemy_spawn_rate", enemy_spawn_rate);
        file << "\n";

        file << "# ============ GRAPHICS SETTINGS ============\n";
        file << "fullscreen=" << (fullscreen ? "true" : "false") << "\n";
        file << "vsync=" << (vsync ? "true" : "false") << "\n";
        file << "show_fps=" << (show_fps ? "true" : "false") << "\n";
        file << "screen_shake=" << (screen_shake ? "true" : "false") << "\n";
        file << "target_fps=" << target_fps << "\n\n";

        file << "# ============ NETWORK SETTINGS ============\n";
        file << "default_server_ip=" << default_server_ip << "\n";
        file << "default_port=" << default_port << "\n";
        file << "player_name=" << player_name << "\n";
        file << "auto_connect=" << (auto_connect ? "true" : "false") << "\n\n";

        file << "# ============ ACCESSIBILITY SETTINGS ============\n";
        file << "high_contrast=" << (high_contrast ? "true" : "false") << "\n";
        file << "font_scale_index=" << font_scale_index << "\n";
        file << "language=" << language << "\n";

        return true;
    }

    float enemy_hp_multiplier() const {
        float difficulty_mult = 1.0f;
        switch (difficulty) {
            case 0: difficulty_mult = 0.75f; break;
            case 2: difficulty_mult = 1.25f; break;
            case 3: difficulty_mult = 1.50f; break;
            default: difficulty_mult = 1.0f; break;
        }
        return difficulty_mult * level_enemy_hp_mult;
    }

    float enemy_speed_multiplier() const {
        float difficulty_mult = 1.0f;
        switch (difficulty) {
            case 0: difficulty_mult = 0.90f; break;
            case 2: difficulty_mult = 1.10f; break;
            case 3: difficulty_mult = 1.20f; break;
            default: difficulty_mult = 1.0f; break;
        }
        return difficulty_mult * level_enemy_speed_mult;
    }

    float enemy_spawn_rate_multiplier() const {
        float difficulty_mult = 1.0f;
        switch (difficulty) {
            case 0: difficulty_mult = 0.85f; break;
            case 2: difficulty_mult = 1.15f; break;
            case 3: difficulty_mult = 1.30f; break;
            default: difficulty_mult = 1.0f; break;
        }
        return difficulty_mult * level_spawn_rate_mult;
    }

    float player_shoot_cooldown_multiplier() const {
        switch (difficulty) {
            case 0: return 0.90f;
            case 2: return 1.10f;
            case 3: return 1.20f;
            default: return 1.0f;
        }
    }

    float enemy_shoot_cooldown_multiplier() const {
        switch (difficulty) {
            case 0: return 1.20f;
            case 2: return 0.90f;
            case 3: return 0.80f;
            default: return 1.0f;
        }
    }

private:
    static std::string trim(const std::string& input) {
        const auto not_space = [](unsigned char ch) { return !std::isspace(ch); };
        auto begin = std::find_if(input.begin(), input.end(), not_space);
        auto end = std::find_if(input.rbegin(), input.rend(), not_space).base();
        if (begin >= end) {
            return {};
        }
        return std::string(begin, end);
    }

    static bool parse_bool(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (value == "true" || value == "1") {
            return true;
        }
        if (value == "false" || value == "0") {
            return false;
        }
        return false;
    }

    static void write_float(std::ostream& out, const char* key, float value) {
        std::ostringstream buffer;
        buffer << std::fixed << std::setprecision(1) << value;
        out << key << "=" << buffer.str() << "\n";
    }
};

}  // namespace engine::game
