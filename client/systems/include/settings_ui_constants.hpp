// Shared ranges for settings UI controls (client-only).
#pragma once

namespace client::systems::settings_ui {

constexpr float kVolumeMin = 0.0f;
constexpr float kVolumeMax = 1.0f;

constexpr int kDifficultyMin = 0;
constexpr int kDifficultyMax = 3;

constexpr int kPlayerLivesMin = 1;
constexpr int kPlayerLivesMax = 9;
constexpr int kEnemiesPerWaveMin = 1;
constexpr int kEnemiesPerWaveMax = 50;
constexpr int kKillsPerWaveMin = 1;
constexpr int kKillsPerWaveMax = 100;

constexpr float kEnemySpawnRateMin = 0.1f;
constexpr float kEnemySpawnRateMax = 5.0f;

constexpr int kTargetFPSMin = 60;
constexpr int kTargetFPSMax = 300;

constexpr int kDefaultPortMin = 1;
constexpr int kDefaultPortMax = 65535;

}  // namespace client::systems::settings_ui
