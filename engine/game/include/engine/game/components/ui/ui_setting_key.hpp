// Shared identifiers for settings-bound UI controls.
#pragma once

namespace engine::game::components {

enum class SettingKey {
    None,
    MasterVolume,
    MusicVolume,
    SfxVolume,
    MusicEnabled,
    Difficulty,
    PlayerLives,
    EnemiesPerWave,
    KillsPerWave,
    InfiniteLives,
    EnemySpawnRate,
    Fullscreen,
    Vsync,
    ShowFps,
    ScreenShake,
    TargetFPS,
    DefaultServerIp,
    DefaultPort,
    PlayerName,
    AutoConnect,
    HighContrast,
    FontScale,
    Language
};

}  // namespace engine::game::components
