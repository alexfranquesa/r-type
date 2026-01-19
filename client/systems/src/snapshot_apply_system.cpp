#include "snapshot_apply_system.hpp"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <string_view>
#include <filesystem>

#include "engine/net/packet.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/sprite.hpp"
#include "engine/game/components/core/velocity.hpp"
#include "engine/game/components/gameplay/health.hpp"
#include "engine/game/components/gameplay/lives.hpp"
#include "engine/game/components/gameplay/spectator.hpp"
#include "engine/game/components/gameplay/ultimate_charge.hpp"
#include "engine/game/components/network/owner.hpp"
#include "engine/game/components/visual/animation.hpp"
#include "engine/game/components/visual/particle_effect.hpp"

namespace {

using engine::game::components::Animation;
using engine::game::components::ParticleEffect;

constexpr float kPlayerFrameDuration = 0.1f;

std::vector<Animation::IntRect> make_grid_frames(int start_x,
                                                 int start_y,
                                                 int frame_w,
                                                 int frame_h,
                                                 std::size_t frame_count,
                                                 std::size_t frames_per_row = 0) {
    std::vector<Animation::IntRect> frames;
    if (frame_w <= 0 || frame_h <= 0 || frame_count == 0) {
        return frames;
    }
    frames.reserve(frame_count);
    const std::size_t per_row = (frames_per_row == 0) ? frame_count : frames_per_row;
    for (std::size_t i = 0; i < frame_count; ++i) {
        const std::size_t row = i / per_row;
        const std::size_t col = i % per_row;
        const int offset_x = static_cast<int>(col) * frame_w;
        const int offset_y = static_cast<int>(row) * frame_h;
        frames.push_back(Animation::IntRect{
            start_x + offset_x,
            start_y + offset_y,
            frame_w,
            frame_h
        });
    }
    return frames;
}

// Player ship animation frames from r-typesheet42.png (5 frames of ship with different thruster states)
// Each frame is 33x17 pixels, arranged horizontally
const std::vector<Animation::IntRect> kPlayerFrames =
    make_grid_frames(0, 0, 33, 17, 5, 5);

// Player projectile animation frames from r-typesheet1.png
// Blue energy projectile, 2 frames animation
const std::vector<Animation::IntRect> kProjectileFrames =
    make_grid_frames(232, 103, 16, 12, 2, 2);

// Ultimate projectile animation frames from bulletulti.png (3 frames, 34x34 each)
const std::vector<Animation::IntRect> kUltimateProjectileFrames =
    make_grid_frames(0, 0, 34, 34, 3, 3);

struct AnimationConfig {
    const Animation::IntRect* frames = nullptr;
    std::size_t frame_count = 0;
    float frame_duration_seconds = 0.0f;
    bool loop = true;
};

struct SpriteConfig {
    std::uint16_t sprite_id = 0;
    std::string_view texture_id;
    engine::game::components::Sprite::Rect rect{};
    float scale = 1.0f;
    float origin_x = 0.0f;
    float origin_y = 0.0f;
    float z_index = 0.0f;
    bool flip_x = false;
    bool flip_y = false;
    const AnimationConfig* animation = nullptr;
    // Color tint (for level variations)
    std::uint8_t color_r = 255;
    std::uint8_t color_g = 255;
    std::uint8_t color_b = 255;
};

const AnimationConfig kPlayerAnimation{
    kPlayerFrames.data(),
    kPlayerFrames.size(),
    kPlayerFrameDuration,
    true
};

const AnimationConfig kProjectileAnimation{
    kProjectileFrames.data(),
    kProjectileFrames.size(),
    0.05f,  // Fast animation for projectiles
    true
};

const AnimationConfig kUltimateProjectileAnimation{
    kUltimateProjectileFrames.data(),
    kUltimateProjectileFrames.size(),
    0.06f,
    true
};

// Ice crab enemy animation frames from r-typesheet7.gif
// Use only first 4 frames (left-facing towards player)
const std::vector<Animation::IntRect> kIceCrabFrames =
    make_grid_frames(0, 0, 34, 34, 4, 4);

const AnimationConfig kIceCrabAnimation{
    kIceCrabFrames.data(),
    kIceCrabFrames.size(),
    0.12f,  // Smooth crab animation
    true
};

// Boss sprite from BOSS.png (2048x140, 8 horizontal frames of 256x140 each)
const std::vector<Animation::IntRect> kRealBossFrames = {
    {0, 0, 256, 140},
    {256, 0, 256, 140},
    {512, 0, 256, 140},
    {768, 0, 256, 140},
    {1024, 0, 256, 140},
    {1280, 0, 256, 140},
    {1536, 0, 256, 140},
    {1792, 0, 256, 140},
};

const AnimationConfig kRealBossAnimation{
    kRealBossFrames.data(),
    kRealBossFrames.size(),
    0.12f,  // Smooth animation
    true    // Loop
};

// Boss animation frames from r-typesheet32.png (520x573 total)
// 8 frames total: 2 rows x 4 columns, each frame 130x286 pixels
const std::vector<Animation::IntRect> kBossFrames = {
    {0, 0, 130, 286},
    {130, 0, 130, 286},
    {260, 0, 130, 286},
    {390, 0, 130, 286},
    {0, 286, 130, 286},
    {130, 286, 130, 286},
    {260, 286, 130, 286},
    {390, 286, 130, 286},
};

const AnimationConfig kBossAnimation{
    kBossFrames.data(),
    kBossFrames.size(),
    0.12f,  // Smooth boss animation
    true
};

// Enemy bullet animation frames from enemiebullet.png (681x158 sprite sheet)
// Sprite sheet with projectile frames - use simple single frame for now
const std::vector<Animation::IntRect> kEnemyBulletFrames = {
    {0, 0, 34, 32},
    {34, 0, 34, 32},
    {68, 0, 34, 32},
    {102, 0, 34, 32},
};

const AnimationConfig kEnemyBulletAnimation{
    kEnemyBulletFrames.data(),
    kEnemyBulletFrames.size(),
    0.1f,   // Animation speed
    true    // Loop
};

// Boss projectile animation - FinalProjectile.png 4 frames in 2x2 grid
// 1024x1024 total, each frame is 512x512 pixels
const std::vector<Animation::IntRect> kBossProjectileFrames = {
    {0, 0, 512, 512},       // Frame 1 (top-left)
    {512, 0, 512, 512},     // Frame 2 (top-right)
    {0, 512, 512, 512},     // Frame 3 (bottom-left)
    {512, 512, 512, 512},   // Frame 4 (bottom-right)
};

const AnimationConfig kBossProjectileAnimation{
    kBossProjectileFrames.data(),
    kBossProjectileFrames.size(),
    0.1f,   // Fast animation
    true    // Loop
};

// Enemy configurations per level (Level 1-6)
// Each level has different sprites and colors
struct LevelEnemyConfig {
    std::string_view texture_id;
    engine::game::components::Sprite::Rect rect;
    float scale;
    std::uint8_t color_r;
    std::uint8_t color_g;
    std::uint8_t color_b;
    const AnimationConfig* animation;  // Optional animation
};

const LevelEnemyConfig kEnemyConfigPerLevel[] = {
    // Level 1: Green aliens in space (classic r-typesheet5) - no animation
    {"r-typesheet5", {5, 6, 21, 24}, 2.2f, 100, 255, 100, nullptr},
    // Level 2: VOLCANIC - Intense YELLOW enemies
    {"r-typesheet5", {5, 6, 21, 24}, 2.4f, 255, 255, 0, nullptr},
    // Level 3: Orange crab enemies (asteroid theme) - animated
    {"r-typesheet7", {0, 0, 34, 34}, 2.0f, 255, 255, 255, &kIceCrabAnimation},
    // Level 4: Ice crab enemies (ice theme) - animated cyan crabs
    {"r-typesheet7", {0, 0, 34, 34}, 2.0f, 150, 220, 255, &kIceCrabAnimation},
    // Level 5: INFERNO - Fire creatures from r-typesheet18
    {"r-typesheet18", {52, 2, 50, 33}, 1.8f, 255, 150, 100, nullptr},
    // Level 6: Dark red boss minions
    {"r-typesheet30a", {1, 3, 39, 28}, 2.2f, 255, 100, 100, nullptr},
};

// Enemy projectile configuration per level (sprite + color)
struct LevelProjectileConfig {
    std::string_view texture_id;
    engine::game::components::Sprite::Rect rect;
    float scale;
    std::uint8_t color_r;
    std::uint8_t color_g;
    std::uint8_t color_b;
};

const LevelProjectileConfig kEnemyProjectilePerLevel[] = {
    // Level 1: Green energy blast (space theme)
    {"r-typesheet5", {236, 10, 21, 14}, 1.8f, 200, 255, 100},
    // Level 2: LAVA FIREBALL - orange/red projectile
    {"r-typesheet5", {236, 10, 21, 14}, 2.0f, 255, 100, 30},
    // Level 3: Orange energy (asteroid theme)
    {"r-typesheet5", {236, 10, 21, 14}, 1.8f, 255, 180, 80},
    // Level 4: ICE BLUE - regular projectiles with ice color (NOT the r-typesheet30a)
    {"r-typesheet5", {236, 10, 21, 14}, 1.8f, 100, 200, 255},
    // Level 5: Red fireball (inferno theme)
    {"r-typesheet3", {1, 2, 29, 12}, 1.6f, 255, 80, 50},
    // Level 6: Dark red plasma (boss theme)
    {"r-typesheet5", {236, 10, 21, 14}, 2.0f, 255, 50, 50},
};

const SpriteConfig kSpriteConfigs[] = {
    // Player ship from r-typesheet42.png - classic R-Type ship
    // Frame size 33x17, scaled up to ~50px width for visibility
    {static_cast<std::uint16_t>(engine::game::components::SpriteId::Player),
     "r-typesheet42",
     {0, 0, 33, 17},
     1.8f,
     16.5f,
     8.5f,
     0.5f,
     false,
     false,
     &kPlayerAnimation,
     255, 255, 255},
    {static_cast<std::uint16_t>(engine::game::components::SpriteId::EnemyBasic),
     "r-typesheet5",
     {5, 6, 21, 24},
     2.2f,
     0.0f,
     0.0f,
     0.4f,
     false,
     false,
     nullptr,
     255, 255, 255},
    // Player projectile - blue energy blast from r-typesheet1.png
    {static_cast<std::uint16_t>(engine::game::components::SpriteId::PlayerProjectile),
     "r-typesheet1",
     {232, 103, 16, 12},
     1.5f,
     8.0f,
     6.0f,
     0.6f,
     false,
     false,
     &kProjectileAnimation,
     255, 255, 255},
    {static_cast<std::uint16_t>(engine::game::components::SpriteId::UltimateProjectile),
     "bulletulti",
     {0, 0, 34, 34},
     1.5f,
     17.0f,
     17.0f,
     0.6f,
     false,
     false,
     &kUltimateProjectileAnimation,
     255, 255, 255},
    {static_cast<std::uint16_t>(engine::game::components::SpriteId::EnemyProjectile),
     "r-typesheet5",
     {236, 10, 21, 14},
     1.8f,
     10.5f,
     7.0f,
     0.6f,
     true,
     false,
     nullptr,
     255, 255, 255},
    // Lava drop hazard - using r-typesheet3 meteor sprite (working)
    {static_cast<std::uint16_t>(engine::game::components::SpriteId::LavaDrop),
     "r-typesheet3",
     {1, 2, 29, 12},
     2.5f,
     14.5f,
     6.0f,
     0.65f,
     false,
     false,
     nullptr,
     255, 150, 50},  // Orange/red fire tint
    // Asteroid obstacle - using r-typesheet8 asteroid sprites
    {static_cast<std::uint16_t>(engine::game::components::SpriteId::Asteroid),
     "r-typesheet8",
     {1, 2, 28, 26},
     2.5f,
     14.0f,
     13.0f,
     0.45f,
     false,
     false,
     nullptr,
     180, 160, 140},  // Gray/brown rock tint
    // Ice crab enemy - Level 4 animated ice enemy from r-typesheet7
    {static_cast<std::uint16_t>(engine::game::components::SpriteId::IceEnemy),
     "r-typesheet7",
     {0, 0, 34, 34},
     2.0f,
     17.0f,
     17.0f,
     0.45f,
     false,
     false,
     &kIceCrabAnimation,
     150, 220, 255},  // Ice blue tint
    // Ice projectile - Level 4 frozen projectile from r-typesheet30a
    // The sprite sheet has 3 ice projectiles horizontally, each ~34x14 pixels
    {static_cast<std::uint16_t>(engine::game::components::SpriteId::IceProjectile),
     "r-typesheet30a",
     {0, 0, 34, 14},
     2.5f,
     17.0f,
     7.0f,
     0.6f,
     false,
     false,
     nullptr,
     150, 255, 255},  // Bright cyan ice tint
    // Boss - Level 5 final boss from BOSS.png (2048x140, 8 frames of 256x140)
    // Hitbox is only on the blue sphere at top-left, not the shell body
    {static_cast<std::uint16_t>(engine::game::components::SpriteId::Boss),
     "BOSS",
     {0, 0, 256, 140},  // First frame (256x140)
     1.5f,              // Scale (384x210 display)
     128.0f,            // Origin X (center)
     70.0f,             // Origin Y (center)
     0.0f,
     false,
     false,
     &kRealBossAnimation, // Use 8-frame animation
     255, 255, 255},    // No tint
    // Boss projectile - Level 5 using FinalProjectile.png (homing magnetic projectile)
    {static_cast<std::uint16_t>(engine::game::components::SpriteId::BossProjectile),
     "FinalProjectile",
     {0, 0, 512, 512},  // First frame of 2x2 animation grid
     0.08f,             // Scale down (512px -> ~40px)
     256.0f,            // Origin X (center of 512)
     256.0f,            // Origin Y (center of 512)
     0.0f,
     false,
     false,
     &kBossProjectileAnimation, // 4-frame animation
     255, 255, 255},    // No tint - use original colors
};

const SpriteConfig kFallbackSprite{
    0,
    "missing",
    {0, 0, 0, 0},
    1.0f,
    0.0f,
    0.0f,
    0.0f,
    false,
    false,
    nullptr,
    255, 255, 255
};

const SpriteConfig* find_sprite_config(std::uint16_t sprite_id) {
    for (const auto& config : kSpriteConfigs) {
        if (config.sprite_id == sprite_id) {
            return &config;
        }
    }
    return nullptr;
}

// Helper to get level-specific enemy config (0-indexed level)
const LevelEnemyConfig& getEnemyConfigForLevel(std::uint16_t level) {
    constexpr std::size_t num_configs = sizeof(kEnemyConfigPerLevel) / sizeof(kEnemyConfigPerLevel[0]);
    std::size_t idx = (level > 0 && level <= num_configs) ? (level - 1) : 0;
    return kEnemyConfigPerLevel[idx];
}

// Helper to get level-specific enemy projectile config
const LevelProjectileConfig& getEnemyProjectileConfigForLevel(std::uint16_t level) {
    constexpr std::size_t num_configs = sizeof(kEnemyProjectilePerLevel) / sizeof(kEnemyProjectilePerLevel[0]);
    std::size_t idx = (level > 0 && level <= num_configs) ? (level - 1) : 0;
    return kEnemyProjectilePerLevel[idx];
}

constexpr int kExplosionFrameSize = 32;
constexpr std::size_t kExplosionFrameCount = 7;
constexpr float kExplosionScale = 2.2f;
constexpr float kExplosionFrameDuration = 0.06f;
const std::vector<Animation::IntRect> kExplosionFrames =
    make_grid_frames(0, 0, kExplosionFrameSize, kExplosionFrameSize, kExplosionFrameCount, kExplosionFrameCount);
const float kExplosionLifetime = kExplosionFrameDuration * static_cast<float>(kExplosionFrameCount) + 0.02f;

}  // namespace

namespace client::systems {

float SnapshotApplySystem::read_float(const std::uint8_t* data) {
    std::uint32_t bits = static_cast<std::uint32_t>(data[0]) |
                         (static_cast<std::uint32_t>(data[1]) << 8) |
                         (static_cast<std::uint32_t>(data[2]) << 16) |
                         (static_cast<std::uint32_t>(data[3]) << 24);
    float value;
    std::memcpy(&value, &bits, sizeof(float));
    return value;
}

std::uint16_t SnapshotApplySystem::read_uint16(const std::uint8_t* data) {
    return static_cast<std::uint16_t>(data[0]) |
           (static_cast<std::uint16_t>(data[1]) << 8);
}

std::int16_t SnapshotApplySystem::read_int16(const std::uint8_t* data) {
    return static_cast<std::int16_t>(data[0]) |
           static_cast<std::int16_t>(static_cast<std::uint16_t>(data[1]) << 8);
}

void SnapshotApplySystem::reset() {
    last_snapshot_tick_ = 0;
    last_positions_.clear();
    last_sprite_ids_.clear();
    last_hp_.clear();
    logged_missing_sprite_ids_.clear();
    current_level_ = 1;
}

void SnapshotApplySystem::play_sound(const sf::SoundBuffer& buffer) {
    // Cleanup finished sounds first
    active_sounds_.erase(
        std::remove_if(active_sounds_.begin(), active_sounds_.end(),
                       [](const sf::Sound& s) { return s.getStatus() == sf::SoundSource::Status::Stopped; }),
        active_sounds_.end());

    // Create and add sound to vector first, then play
    active_sounds_.emplace_back(buffer);
    sf::Sound& sound = active_sounds_.back();
    sound.setRelativeToListener(true);
    sound.setAttenuation(0.f);
    sound.setMinDistance(1.f);
    sound.setVolume(sfx_volume_);  // Use the configured volume
    sound.play();
}

void SnapshotApplySystem::set_sfx_volume(float volume) {
    sfx_volume_ = std::clamp(volume, 0.f, 100.f);
}

bool SnapshotApplySystem::load_buffer(sf::SoundBuffer& buf, std::initializer_list<std::filesystem::path> paths) {
    if (buf.getSampleCount() > 0) {
        return true;
    }
    for (const auto& p : paths) {
        if (std::filesystem::exists(p)) {
            if (buf.loadFromFile(p.string())) {
                std::cout << "[sound] loaded " << p.string() << std::endl;
                return true;
            } else {
                std::cerr << "[sound] failed to load " << p.string() << std::endl;
            }
        }
    }
    return false;
}

void SnapshotApplySystem::play_player_shoot() {
    if (!fail_player_shoot_ && load_buffer(buf_player_shoot_, {
            "sound_effects/sounds_blaster.ogg",
            "../sound_effects/sounds_blaster.ogg",
            "../../sound_effects/sounds_blaster.ogg",
            "client/sound_effects/sounds_blaster.ogg",
            "assets/sound_effects/sounds_blaster.ogg",
            "client/assets/sound_effects/sounds_blaster.ogg"
        })) {
        play_sound(buf_player_shoot_);
    } else if (buf_player_shoot_.getSampleCount() == 0) {
        fail_player_shoot_ = true;
    }
}

void SnapshotApplySystem::play_enemy_shoot() {
    if (!fail_enemy_shoot_ && load_buffer(buf_enemy_shoot_, {
            "sound_effects/sounds_enemy_attack.ogg",
            "../sound_effects/sounds_enemy_attack.ogg",
            "../../sound_effects/sounds_enemy_attack.ogg",
            "client/sound_effects/sounds_enemy_attack.ogg",
            "assets/sound_effects/sounds_enemy_attack.ogg",
            "client/assets/sound_effects/sounds_enemy_attack.ogg"
        })) {
        play_sound(buf_enemy_shoot_);
    } else if (buf_enemy_shoot_.getSampleCount() == 0) {
        fail_enemy_shoot_ = true;
    }
}

void SnapshotApplySystem::play_enemy_destroy() {
    if (!fail_enemy_destroy_ && load_buffer(buf_enemy_destroy_, {
            "sound_effects/sounds_enemy_destroy.ogg",
            "../sound_effects/sounds_enemy_destroy.ogg",
            "../../sound_effects/sounds_enemy_destroy.ogg",
            "client/sound_effects/sounds_enemy_destroy.ogg",
            "assets/sound_effects/sounds_enemy_destroy.ogg",
            "client/assets/sound_effects/sounds_enemy_destroy.ogg"
        })) {
        play_sound(buf_enemy_destroy_);
    } else if (buf_enemy_destroy_.getSampleCount() == 0) {
        fail_enemy_destroy_ = true;
    }
}

void SnapshotApplySystem::play_player_death() {
    if (!fail_player_death_ && load_buffer(buf_player_death_, {
            "sound_effects/super-mario-death-sound-sound-effect.ogg",
            "../sound_effects/super-mario-death-sound-sound-effect.ogg",
            "../../sound_effects/super-mario-death-sound-sound-effect.ogg",
            "client/sound_effects/super-mario-death-sound-sound-effect.ogg",
            "assets/sound_effects/super-mario-death-sound-sound-effect.ogg",
            "client/assets/sound_effects/super-mario-death-sound-sound-effect.ogg"
        })) {
        play_sound(buf_player_death_);
    } else if (buf_player_death_.getSampleCount() == 0) {
        fail_player_death_ = true;
    }
}

void SnapshotApplySystem::apply(rtype::ecs::registry& registry, const engine::net::SnapshotMessage& snapshot) {
    // SNAPSHOT STABILITY GUARD
    if (snapshot.paused) {
        return; // Freeze world state
    }
    if (snapshot.tick <= last_snapshot_tick_) {
        return;
    }
    last_snapshot_tick_ = snapshot.tick;


    auto spawn_explosion = [&](float x, float y) {
        auto entity = registry.spawn_entity();
        registry.emplace_component<engine::game::components::Position>(entity, x, y);

        engine::game::components::Sprite sprite{};
        sprite.texture_id = "explosion-sheet";
        sprite.texture_rect = {0, 0, kExplosionFrameSize, kExplosionFrameSize};
        sprite.scale_x = kExplosionScale;
        sprite.scale_y = kExplosionScale;
        sprite.origin_x = static_cast<float>(kExplosionFrameSize) * 0.5f;
        sprite.origin_y = static_cast<float>(kExplosionFrameSize) * 0.5f;
        sprite.z_index = 0.9f;
        sprite.visible = true;
        registry.add_component(entity, std::move(sprite));

        Animation anim;
        anim.frames.assign(kExplosionFrames.begin(), kExplosionFrames.end());
        anim.frame_duration_seconds = kExplosionFrameDuration;
        anim.loop = false;
        anim.playing = true;
        registry.add_component(entity, std::move(anim));

        registry.add_component(entity, ParticleEffect{kExplosionLifetime, 0.0f, true});
    };

    const auto& blob = snapshot.blob;
    if (blob.size() < 2) {
        std::cerr << "[SnapshotApplySystem] Blob too small: " << blob.size() << " bytes" << std::endl;
        return;
    }

    std::uint16_t entity_count = read_uint16(blob.data());
    constexpr std::size_t per_entity = 2 + 4 + 4 + 4 + 4 + 2 + 2 + 2 + 2 + 2 + 2 + 1 + 1 + 1;  // id + x + y + vx + vy + hp_cur + hp_max + sprite_id + owner_id + lives_remaining + lives_max + is_spectating + ulti_frame + ulti_ready
    // Stats: score(4) + wave(2) + current_level(2) + kills_this_level(2) + kills_to_next(2) + total_kills(2)
    const std::size_t stats_size = 4 + 2 + 2 + 2 + 2 + 2;

    std::size_t actual_data_size = blob.size() >= (2 + stats_size) ? blob.size() - 2 - stats_size : 0;
    std::uint16_t actual_entity_count = static_cast<std::uint16_t>(actual_data_size / per_entity);

    if (actual_entity_count != entity_count) {
        entity_count = actual_entity_count;
    }

    if (blob.size() < 2 + static_cast<std::size_t>(entity_count) * per_entity + stats_size) {
        std::cerr << "[SnapshotApplySystem] Invalid blob size: " << blob.size()
                  << " for " << entity_count << " entities" << std::endl;
        return;
    }

    // Pre-read current_level from stats footer BEFORE processing entities
    // This ensures enemy coloring uses the correct level
    {
        std::size_t stats_offset = 2 + static_cast<std::size_t>(entity_count) * per_entity;
        stats_offset += 4;  // Skip score (4 bytes)
        stats_offset += 2;  // Skip wave (2 bytes)
        if (stats_offset + 2 <= blob.size()) {
            current_level_ = read_uint16(&blob[stats_offset]);
        }
    }

    std::size_t offset = 2;
    std::vector<std::uint16_t> seen;
    seen.reserve(entity_count);

    for (std::uint16_t i = 0; i < entity_count; ++i) {
        std::uint16_t entity_id = read_uint16(&blob[offset]);
        offset += 2;
        seen.push_back(entity_id);

        const bool existed_before = (last_sprite_ids_.find(entity_id) != last_sprite_ids_.end());

        float x = read_float(&blob[offset]); offset += 4;
        float y = read_float(&blob[offset]); offset += 4;

        last_positions_[entity_id] = engine::game::components::Position{x, y};

        float vx = read_float(&blob[offset]); offset += 4;
        float vy = read_float(&blob[offset]); offset += 4;

        std::int16_t hp_cur = read_int16(&blob[offset]); offset += 2;
        std::int16_t hp_max = read_int16(&blob[offset]); offset += 2;

        std::uint16_t sprite_id = read_uint16(&blob[offset]); offset += 2;
        last_sprite_ids_[entity_id] = sprite_id;

        std::uint16_t owner_id = read_uint16(&blob[offset]); offset += 2;

        std::int16_t lives_remaining = read_int16(&blob[offset]); offset += 2;
        std::int16_t lives_max = read_int16(&blob[offset]); offset += 2;
        
        // Read spectator flag
        std::uint8_t is_spectating = blob[offset]; offset += 1;
        std::uint8_t ultimate_frame = blob[offset]; offset += 1;
        std::uint8_t ultimate_ready = blob[offset]; offset += 1;

        const SpriteConfig* config = find_sprite_config(sprite_id);
        if (!config) {
            if (logged_missing_sprite_ids_.insert(sprite_id).second) {
                std::cerr << "[SnapshotApplySystem] Unknown sprite_id " << sprite_id
                          << ", using fallback sprite config." << std::endl;
            }
            config = &kFallbackSprite;
        }

        // New entity event sounds
        if (!existed_before) {
            using engine::game::components::SpriteId;
            if (sprite_id == static_cast<std::uint16_t>(SpriteId::PlayerProjectile)) {
                play_player_shoot();
            } else if (sprite_id == static_cast<std::uint16_t>(SpriteId::EnemyProjectile) ||
                       sprite_id == static_cast<std::uint16_t>(SpriteId::IceProjectile)) {
                play_enemy_shoot();
            }
        }

        rtype::ecs::entity_t entity = registry.entity_from_index(entity_id);

        // Position
        auto* pos = registry.try_get<engine::game::components::Position>(entity);
        if (!pos) {
            registry.emplace_component<engine::game::components::Position>(entity, x, y);
        } else {
            pos->x = x;
            pos->y = y;
        }

        // Sprite - check if we need to apply level-specific colors
        auto* sprite = registry.try_get<engine::game::components::Sprite>(entity);
        auto* animation = registry.try_get<Animation>(entity);

        // Determine color based on sprite type and current level
        std::uint8_t sprite_r = config->color_r;
        std::uint8_t sprite_g = config->color_g;
        std::uint8_t sprite_b = config->color_b;
        float sprite_scale = config->scale;
        std::string_view override_texture = config->texture_id;
        engine::game::components::Sprite::Rect override_rect = config->rect;
        const AnimationConfig* anim_config = config->animation;  // Animation to apply

        // Apply level-specific styling for enemies and enemy projectiles
        if (sprite_id == static_cast<std::uint16_t>(engine::game::components::SpriteId::EnemyBasic)) {
            const auto& enemy_config = getEnemyConfigForLevel(current_level_);
            sprite_r = enemy_config.color_r;
            sprite_g = enemy_config.color_g;
            sprite_b = enemy_config.color_b;
            sprite_scale = enemy_config.scale;
            override_texture = enemy_config.texture_id;
            override_rect = enemy_config.rect;
            anim_config = enemy_config.animation;  // Use level-specific animation
        } else if (sprite_id == static_cast<std::uint16_t>(engine::game::components::SpriteId::EnemyProjectile) ||
                   sprite_id == static_cast<std::uint16_t>(engine::game::components::SpriteId::IceProjectile)) {
            // All enemy projectiles (including ice projectiles) use level-based styling
            const auto& proj_config = getEnemyProjectileConfigForLevel(current_level_);
            sprite_r = proj_config.color_r;
            sprite_g = proj_config.color_g;
            sprite_b = proj_config.color_b;
            sprite_scale = proj_config.scale;
            override_texture = proj_config.texture_id;
            override_rect = proj_config.rect;
        }

        if (!sprite) {
            engine::game::components::Sprite spr;
            spr.texture_id = std::string{override_texture};
            spr.texture_rect = override_rect;
            spr.scale_x = sprite_scale;
            spr.scale_y = sprite_scale;
            spr.origin_x = config->origin_x;
            spr.origin_y = config->origin_y;
            spr.z_index = config->z_index;
            spr.flip_x = config->flip_x;
            spr.flip_y = config->flip_y;
            spr.visible = !is_spectating;  // Hide spectators
            spr.color_r = sprite_r;
            spr.color_g = sprite_g;
            spr.color_b = sprite_b;
            registry.add_component(entity, std::move(spr));
        } else {
            sprite->texture_id = std::string{override_texture};
            sprite->texture_rect = override_rect;
            sprite->scale_x = sprite_scale;
            sprite->scale_y = sprite_scale;
            sprite->origin_x = config->origin_x;
            sprite->origin_y = config->origin_y;
            sprite->z_index = config->z_index;
            sprite->flip_x = config->flip_x;
            sprite->flip_y = config->flip_y;
            sprite->visible = !is_spectating;  // Hide spectators
            sprite->color_r = sprite_r;
            sprite->color_g = sprite_g;
            sprite->color_b = sprite_b;
        }

        if (anim_config) {
            if (!animation) {
                Animation anim;
                anim.frames.assign(anim_config->frames,
                                   anim_config->frames + anim_config->frame_count);
                anim.frame_duration_seconds = anim_config->frame_duration_seconds;
                anim.loop = anim_config->loop;
                anim.playing = true;
                registry.add_component(entity, std::move(anim));
                animation = registry.try_get<Animation>(entity);
            } else if (animation->frames.empty()) {
                animation->frames.assign(anim_config->frames,
                                         anim_config->frames + anim_config->frame_count);
                animation->frame_duration_seconds = anim_config->frame_duration_seconds;
                animation->loop = anim_config->loop;
                animation->playing = true;
            }

            if (!animation || animation->frames.empty()) {
                // Keep base rect if animation isn't ready yet.
            } else if (!sprite) {
                // New sprite uses first frame as initial rect.
                auto* sprite_after = registry.try_get<engine::game::components::Sprite>(entity);
                if (sprite_after) {
                    const auto* frame = animation->current_frame();
                    if (frame) {
                        sprite_after->texture_rect = {frame->x, frame->y, frame->w, frame->h};
                    }
                }
            }
        } else {
            if (!sprite) {
                // New sprite already has rect set above.
            } else {
                sprite->texture_rect = override_rect;
            }
        }

        // Velocity
        auto* vel = registry.try_get<engine::game::components::Velocity>(entity);
        if (!vel) {
            registry.emplace_component<engine::game::components::Velocity>(entity, vx, vy);
        } else {
            vel->vx = vx;
            vel->vy = vy;
        }

        // HEALTH + death detection (only ships)
        if (hp_cur >= 0 && hp_max > 0) {
            // Detect transition: alive -> dead
            const auto itPrev = last_hp_.find(entity_id);
            const std::int16_t prev_hp = (itPrev != last_hp_.end()) ? itPrev->second : hp_cur;

            const bool was_alive = (prev_hp > 0);
            const bool is_dead   = (hp_cur <= 0);

            if (was_alive && is_dead && is_ship_sprite(sprite_id)) {
                spawn_explosion(x, y);
                using engine::game::components::SpriteId;
                if (sprite_id == static_cast<std::uint16_t>(SpriteId::EnemyBasic)) {
                    play_enemy_destroy();
                } else if (sprite_id == static_cast<std::uint16_t>(SpriteId::Player)) {
                    play_player_death();
                }
            }

            // Update stored hp
            last_hp_[entity_id] = hp_cur;

            // Sync Health component
            auto* health = registry.try_get<engine::game::components::Health>(entity);
            if (!health) {
                registry.emplace_component<engine::game::components::Health>(
                    entity,
                    static_cast<int>(hp_cur),
                    static_cast<int>(hp_max)
                );
            } else {
                health->current = hp_cur;
                health->max = hp_max;
            }
        } else {
            // No health in snapshot -> keep last_hp_ as-is (don't invent explosions)
        }

        // Owner
        auto* owner = registry.try_get<engine::game::components::Owner>(entity);
        if (!owner) {
            registry.emplace_component<engine::game::components::Owner>(entity, owner_id);
        } else {
            owner->player_id = owner_id;
        }

        const bool is_player_entity =
            sprite_id == static_cast<std::uint16_t>(engine::game::components::SpriteId::Player);
        if (owner_id > 0 && is_player_entity) {
            auto* ultimate = registry.try_get<engine::game::components::UltimateCharge>(entity);
            if (!ultimate) {
                engine::game::components::UltimateCharge charge{};
                charge.kills_since_last_ulti = (ultimate_frame > 0) ? static_cast<std::uint8_t>(ultimate_frame - 1) : 0;
                charge.ui_frame = ultimate_frame;
                charge.ready = (ultimate_ready != 0);
                registry.add_component(entity, std::move(charge));
            } else {
                ultimate->kills_since_last_ulti = (ultimate_frame > 0) ? static_cast<std::uint8_t>(ultimate_frame - 1) : 0;
                ultimate->ui_frame = ultimate_frame;
                ultimate->ready = (ultimate_ready != 0);
            }
        } else {
            if (registry.try_get<engine::game::components::UltimateCharge>(entity)) {
                registry.remove_component<engine::game::components::UltimateCharge>(entity);
            }
        }

        // Spectator - Add or remove based on flag
        auto* spectator = registry.try_get<engine::game::components::Spectator>(entity);
        if (is_spectating > 0) {
            if (!spectator) {
                registry.emplace_component<engine::game::components::Spectator>(entity);
            }
        } else {
            if (spectator) {
                registry.remove_component<engine::game::components::Spectator>(entity);
            }
        }

        // Lives - Update even when remaining is 0 (player dead)
        if (lives_max > 0) {
            auto* lives = registry.try_get<engine::game::components::Lives>(entity);
            if (!lives) {
                registry.emplace_component<engine::game::components::Lives>(
                    entity,
                    static_cast<int>(lives_remaining),
                    static_cast<int>(lives_max)
                );
                std::cout << "[Snapshot] Created Lives component for entity " << entity
                          << ": " << lives_remaining << "/" << lives_max << std::endl;
            } else {
                lives->remaining = lives_remaining;
                lives->max = lives_max;
                std::cout << "[Snapshot] Updated Lives for entity " << entity
                          << ": " << lives_remaining << "/" << lives_max << std::endl;
            }
        }
    }

    // Game stats at the end (score + wave + level info)
    if (offset + stats_size <= blob.size()) {
        std::uint32_t score =
            static_cast<std::uint32_t>(blob[offset]) |
            (static_cast<std::uint32_t>(blob[offset + 1]) << 8) |
            (static_cast<std::uint32_t>(blob[offset + 2]) << 16) |
            (static_cast<std::uint32_t>(blob[offset + 3]) << 24);
        offset += 4;

        std::uint16_t wave = read_uint16(&blob[offset]);
        offset += 2;

        std::uint16_t current_level = read_uint16(&blob[offset]);
        offset += 2;

        std::uint16_t kills_this_level = read_uint16(&blob[offset]);
        offset += 2;

        std::uint16_t kills_to_next_level = read_uint16(&blob[offset]);
        offset += 2;

        std::uint16_t total_kills = read_uint16(&blob[offset]);
        offset += 2;

        // Update cached level for sprite coloring in next frame
        current_level_ = current_level;

        rtype::ecs::entity_t stats_entity = registry.entity_from_index(0);
        if (!registry.try_get<engine::game::components::GameStats>(stats_entity)) {
            stats_entity = registry.spawn_entity();
        }
        auto* gs = registry.try_get<engine::game::components::GameStats>(stats_entity);
        if (!gs) {
            engine::game::components::GameStats new_stats{};
            new_stats.score = score;
            new_stats.wave = wave;
            new_stats.current_level = current_level;
            new_stats.kills_this_level = kills_this_level;
            new_stats.kills_to_next_level = kills_to_next_level;
            new_stats.total_kills = total_kills;
            registry.add_component(stats_entity, std::move(new_stats));
        } else {
            gs->score = score;
            gs->wave = wave;
            gs->current_level = current_level;
            gs->kills_this_level = kills_this_level;
            gs->kills_to_next_level = kills_to_next_level;
            gs->total_kills = total_kills;
        }
    }

    // FULL snapshot removal: explode ONLY if it was a ship AND last_hp <= 0
    if (snapshot.flags != 0) {
        auto& positions = registry.get_components<engine::game::components::Position>();
        for (std::size_t idx = 0; idx < positions.size(); ++idx) {
            if (!positions[idx].has_value())
                continue;

            const auto eid = static_cast<std::uint16_t>(idx);
            if (std::find(seen.begin(), seen.end(), eid) != seen.end())
                continue;

            // Entity disappeared from full snapshot
            const auto itSpr = last_sprite_ids_.find(eid);
            const std::uint16_t last_sprite = (itSpr != last_sprite_ids_.end()) ? itSpr->second : 9999;

            const bool should_explode = is_ship_sprite(last_sprite);

            if (should_explode) {
                auto itPos = last_positions_.find(eid);
                if (itPos != last_positions_.end()) {
                    spawn_explosion(itPos->second.x, itPos->second.y);
                }
                using engine::game::components::SpriteId;
                if (last_sprite == static_cast<std::uint16_t>(SpriteId::EnemyBasic)) {
                    play_enemy_destroy();
                } else if (last_sprite == static_cast<std::uint16_t>(SpriteId::Player)) {
                    play_player_death();
                }
            }

            last_positions_.erase(eid);
            last_sprite_ids_.erase(eid);
            last_hp_.erase(eid);

            registry.kill_entity(rtype::ecs::entity_t{eid});
        }
    }
}

}  // namespace client::systems
