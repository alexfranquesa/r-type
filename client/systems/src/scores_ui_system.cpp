#include "scores_ui_system.hpp"
#include "engine/game/components/ui/ui_transform.hpp"
#include "engine/game/components/ui/ui_text.hpp"
#include "engine/game/components/ui/ui_button.hpp"
#include <sstream>
#include <iomanip>
#include <ctime>

namespace client::systems {

void ScoresUISystem::build_ui(rtype::ecs::registry& reg, const engine::game::Leaderboard& leaderboard) {
    reg.clear();

    constexpr float kVirtualWidth = 1280.0f;
    constexpr float kVirtualHeight = 720.0f;

    // Colors
    constexpr std::uint32_t kGold = 0xFFD700FF;
    constexpr std::uint32_t kSilver = 0xC0C0C0FF;
    constexpr std::uint32_t kBronze = 0xCD7F32FF;
    constexpr std::uint32_t kWhite = 0xFFFFFFFF;

    // Title
    {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, kVirtualWidth * 0.5f - 200.0f, 50.0f, 0.0f, 0.0f);
        engine::game::components::UIText txt{};
        txt.text = "HIGH SCORES";
        txt.font_size = 50;
        txt.color = kGold;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    // Column headers
    const float kHeaderY = 130.0f;
    const std::vector<std::pair<std::string, float>> headers = {
        {"#", 150.0f},
        {"NAME", 250.0f},
        {"SCORE", 500.0f},
        {"WAVE", 700.0f},
        {"LEVEL", 850.0f},
        {"DATE", 1000.0f}
    };

    for (const auto& [text, x] : headers) {
        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, x, kHeaderY, 0.0f, 0.0f);
        engine::game::components::UIText txt{};
        txt.text = text;
        txt.font_size = 24;
        txt.color = kGold;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }

    // Entries
    const auto& entries = leaderboard.entries();
    const float kEntryStartY = 180.0f;
    const float kEntrySpacing = 40.0f;

    for (std::size_t i = 0; i < engine::game::Leaderboard::kMaxEntries; ++i) {
        const float y = kEntryStartY + i * kEntrySpacing;
        std::uint32_t color = kWhite;

        // Top 3 colors
        if (i == 0) color = kGold;
        else if (i == 1) color = kSilver;
        else if (i == 2) color = kBronze;

        std::string rank_text = std::to_string(i + 1);
        std::string name_text = "---";
        std::string score_text = "---";
        std::string wave_text = "---";
        std::string level_text = "---";
        std::string date_text = "---";

        if (i < entries.size()) {
            const auto& entry = entries[i];
            name_text = entry.player_name;
            score_text = std::to_string(entry.score);
            wave_text = std::to_string(entry.wave_reached);
            level_text = std::to_string(entry.level_reached);

            // Format date
            std::time_t time = static_cast<std::time_t>(entry.timestamp);
            std::tm tm_buf;
            #ifdef _WIN32
            localtime_s(&tm_buf, &time);
            #else
            localtime_r(&time, &tm_buf);
            #endif
            std::ostringstream oss;
            oss << std::put_time(&tm_buf, "%Y-%m-%d");
            date_text = oss.str();
        }

        // Rank
        {
            auto entity = reg.spawn_entity();
            reg.emplace_component<engine::game::components::UITransform>(entity, 150.0f, y, 0.0f, 0.0f);
            engine::game::components::UIText txt{};
            txt.text = rank_text;
            txt.font_size = 20;
            txt.color = color;
            reg.emplace_component<engine::game::components::UIText>(entity, txt);
        }

        // Name
        {
            auto entity = reg.spawn_entity();
            reg.emplace_component<engine::game::components::UITransform>(entity, 250.0f, y, 0.0f, 0.0f);
            engine::game::components::UIText txt{};
            txt.text = name_text;
            txt.font_size = 20;
            txt.color = color;
            reg.emplace_component<engine::game::components::UIText>(entity, txt);
        }

        // Score
        {
            auto entity = reg.spawn_entity();
            reg.emplace_component<engine::game::components::UITransform>(entity, 500.0f, y, 0.0f, 0.0f);
            engine::game::components::UIText txt{};
            txt.text = score_text;
            txt.font_size = 20;
            txt.color = color;
            reg.emplace_component<engine::game::components::UIText>(entity, txt);
        }

        // Wave
        {
            auto entity = reg.spawn_entity();
            reg.emplace_component<engine::game::components::UITransform>(entity, 700.0f, y, 0.0f, 0.0f);
            engine::game::components::UIText txt{};
            txt.text = wave_text;
            txt.font_size = 20;
            txt.color = color;
            reg.emplace_component<engine::game::components::UIText>(entity, txt);
        }

        // Level
        {
            auto entity = reg.spawn_entity();
            reg.emplace_component<engine::game::components::UITransform>(entity, 850.0f, y, 0.0f, 0.0f);
            engine::game::components::UIText txt{};
            txt.text = level_text;
            txt.font_size = 20;
            txt.color = color;
            reg.emplace_component<engine::game::components::UIText>(entity, txt);
        }

        // Date
        {
            auto entity = reg.spawn_entity();
            reg.emplace_component<engine::game::components::UITransform>(entity, 1000.0f, y, 0.0f, 0.0f);
            engine::game::components::UIText txt{};
            txt.text = date_text;
            txt.font_size = 20;
            txt.color = color;
            reg.emplace_component<engine::game::components::UIText>(entity, txt);
        }
    }

    // Back button
    {
        const float button_width = 320.0f;
        const float button_height = 56.0f;
        const float button_x = (kVirtualWidth - button_width) * 0.5f;
        const float button_y = kVirtualHeight - 80.0f;

        auto entity = reg.spawn_entity();
        reg.emplace_component<engine::game::components::UITransform>(entity, button_x, button_y, button_width, button_height);
        reg.emplace_component<engine::game::components::UIButton>(entity, engine::game::components::UIButton{"back_to_menu", false, false});
        engine::game::components::UIText txt{};
        txt.text = ">> BACK TO MENU <<";
        txt.font_size = 28;
        txt.color = kWhite;
        reg.emplace_component<engine::game::components::UIText>(entity, txt);
    }
}

}  // namespace client::systems
