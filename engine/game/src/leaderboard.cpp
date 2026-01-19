#include "engine/game/leaderboard.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>

namespace engine::game {

bool Leaderboard::load(const std::filesystem::path& path) {
    entries_.clear();

    if (!std::filesystem::exists(path)) {
        return true;  // Empty leaderboard is valid
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    std::vector<ScoreEntry> temp_entries;

    for (std::size_t i = 1; i <= kMaxEntries; ++i) {
        ScoreEntry entry;
        bool found_name = false, found_score = false, found_wave = false;
        bool found_level = false, found_timestamp = false;

        // Read all lines for this entry
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::string key_prefix = "entry_" + std::to_string(i) + "_";
            if (line.find(key_prefix) != 0) {
                // Not this entry's data, rewind for next entry
                break;
            }

            auto equals_pos = line.find('=');
            if (equals_pos == std::string::npos) continue;

            std::string key = line.substr(0, equals_pos);
            std::string value = line.substr(equals_pos + 1);

            if (key == key_prefix + "name") {
                entry.player_name = value;
                found_name = true;
            } else if (key == key_prefix + "score") {
                entry.score = static_cast<std::uint32_t>(std::stoul(value));
                found_score = true;
            } else if (key == key_prefix + "wave") {
                entry.wave_reached = static_cast<std::uint16_t>(std::stoul(value));
                found_wave = true;
            } else if (key == key_prefix + "level") {
                entry.level_reached = static_cast<std::uint16_t>(std::stoul(value));
                found_level = true;
            } else if (key == key_prefix + "timestamp") {
                entry.timestamp = std::stoll(value);
                found_timestamp = true;
            }
        }

        // Add entry if we found at least the essential fields
        if (found_name && found_score && found_wave && found_level && found_timestamp) {
            temp_entries.push_back(entry);
        }
    }

    // Sort and store
    std::sort(temp_entries.begin(), temp_entries.end(), std::greater<>());
    entries_ = std::move(temp_entries);

    return true;
}

bool Leaderboard::save(const std::filesystem::path& path) const {
    // Ensure parent directory exists
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }

    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }

    file << "# R-Type Leaderboard\n";
    file << "# Top " << kMaxEntries << " high scores\n\n";

    for (std::size_t i = 0; i < entries_.size() && i < kMaxEntries; ++i) {
        const auto& entry = entries_[i];
        std::size_t idx = i + 1;

        file << "entry_" << idx << "_name=" << entry.player_name << "\n";
        file << "entry_" << idx << "_score=" << entry.score << "\n";
        file << "entry_" << idx << "_wave=" << entry.wave_reached << "\n";
        file << "entry_" << idx << "_level=" << entry.level_reached << "\n";
        file << "entry_" << idx << "_timestamp=" << entry.timestamp << "\n";
        file << "\n";
    }

    return true;
}

bool Leaderboard::add_entry(const ScoreEntry& entry) {
    // Validate and truncate name
    ScoreEntry validated_entry = entry;
    if (validated_entry.player_name.empty()) {
        validated_entry.player_name = "Anonymous";
    }
    if (validated_entry.player_name.length() > 16) {
        validated_entry.player_name = validated_entry.player_name.substr(0, 16);
    }

    // Set timestamp if not set
    if (validated_entry.timestamp == 0) {
        validated_entry.timestamp = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now()
        );
    }

    // Add and sort
    entries_.push_back(validated_entry);
    std::sort(entries_.begin(), entries_.end(), std::greater<>());

    // Trim to max entries
    if (entries_.size() > kMaxEntries) {
        entries_.resize(kMaxEntries);
    }

    // Check if the entry made it
    auto it = std::find_if(entries_.begin(), entries_.end(),
                           [&](const ScoreEntry& e) {
                               return e.player_name == validated_entry.player_name &&
                                      e.score == validated_entry.score &&
                                      e.timestamp == validated_entry.timestamp;
                           });

    return it != entries_.end();
}

bool Leaderboard::would_make_leaderboard(std::uint32_t score) const {
    if (entries_.size() < kMaxEntries) {
        return true;
    }
    return score > entries_.back().score;
}

}  // namespace engine::game
