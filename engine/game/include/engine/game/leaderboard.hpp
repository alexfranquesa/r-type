#pragma once

#include <filesystem>
#include <vector>
#include "score_entry.hpp"

namespace engine::game {

class Leaderboard {
public:
    static constexpr std::size_t kMaxEntries = 10;

    Leaderboard() = default;

    // Load leaderboard from INI-style config file
    bool load(const std::filesystem::path& path);

    // Save leaderboard to INI-style config file
    bool save(const std::filesystem::path& path) const;

    // Add entry, returns true if it made the top 10
    bool add_entry(const ScoreEntry& entry);

    // Check if score would make leaderboard
    bool would_make_leaderboard(std::uint32_t score) const;

    // Get all entries (sorted)
    const std::vector<ScoreEntry>& entries() const { return entries_; }

private:
    std::vector<ScoreEntry> entries_;
};

}  // namespace engine::game
