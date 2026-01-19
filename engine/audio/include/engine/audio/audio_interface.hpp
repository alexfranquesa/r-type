#pragma once

#include <cstdint>

namespace engine::audio {

enum class MusicId { Menu, Game };
enum class SfxId { Button, Shoot, Explosion };

class IAudio {
public:
    virtual ~IAudio() = default;
    virtual void play_music(MusicId id) = 0;
    virtual void stop_music() = 0;
    virtual void play_sfx(SfxId id) = 0;
    virtual void set_music_volume(float volume) = 0;  // 0..100
    virtual void set_sfx_volume(float volume) = 0;    // 0..100
};

}  // namespace engine::audio

