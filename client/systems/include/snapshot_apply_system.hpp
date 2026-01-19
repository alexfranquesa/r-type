#pragma once

#include <cstdint>
#include <vector>

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <filesystem>
#include <SFML/Audio.hpp>
#include "engine/core/registry.hpp"
#include "engine/net/packet.hpp"
#include "engine/game/components/gameplay/game_stats.hpp"
#include "engine/game/components/core/sprite.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/visual/sprite_id.hpp"

namespace client::systems {

class SnapshotApplySystem {
public:
    SnapshotApplySystem() = default;

    // Reset any internal state cached from previous snapshots
    void reset();

    // Apply a snapshot message to the registry.
    // Format of blob:
    //   u16 entity_count
    //   repeat entity_count times:
    //     u16 entity_id
    //     f32 x, f32 y
    //     f32 vx, f32 vy
    //     i16 hp_cur, i16 hp_max   (=-1 if no health component)
    //     u16 sprite_id            (0=default/player, 1=enemy, 2=projectile)
    //     u8  is_spectating
    //     u8  ultimate_frame
    //     u8  ultimate_ready
    // If snapshot.flags != 0, entities missing from the blob are removed.
    void apply(rtype::ecs::registry& registry, const engine::net::SnapshotMessage& snapshot);
    
    // Set volume for sound effects (0-100)
    void set_sfx_volume(float volume);

private:
    float read_float(const std::uint8_t* data);
    std::uint16_t read_uint16(const std::uint8_t* data);
    std::int16_t read_int16(const std::uint8_t* data);
    std::uint32_t last_snapshot_tick_ = 0;

    // Only ships should trigger explosions (not projectiles)
    bool is_ship_sprite(std::uint16_t sprite_id) const {
        using engine::game::components::SpriteId;
        return (sprite_id == static_cast<std::uint16_t>(SpriteId::Player) ||
                sprite_id == static_cast<std::uint16_t>(SpriteId::EnemyBasic));
    }

    // Sound functions
    void play_sound(const sf::SoundBuffer& buffer);
    bool load_buffer(sf::SoundBuffer& buf, std::initializer_list<std::filesystem::path> paths);
    void play_player_shoot();
    void play_enemy_shoot();
    void play_enemy_destroy();
    void play_player_death();

    std::unordered_map<std::uint16_t, engine::game::components::Position> last_positions_;
    std::unordered_map<std::uint16_t, std::uint16_t> last_sprite_ids_;

    // Track last known HP to detect death transitions and avoid exploding projectiles.
    std::unordered_map<std::uint16_t, std::int16_t> last_hp_;

    std::unordered_set<std::uint16_t> logged_missing_sprite_ids_;

    // Current level for applying level-specific sprites
    std::uint16_t current_level_{1};

    // Sound assets
    sf::SoundBuffer buf_player_shoot_;
    sf::SoundBuffer buf_enemy_shoot_;
    sf::SoundBuffer buf_enemy_destroy_;
    sf::SoundBuffer buf_player_death_;
    std::vector<sf::Sound> active_sounds_;
    float sfx_volume_ = 50.f;  // Default volume
    bool fail_player_shoot_ = false;
    bool fail_enemy_shoot_ = false;
    bool fail_enemy_destroy_ = false;
    bool fail_player_death_ = false;
};

}  // namespace client::systems
