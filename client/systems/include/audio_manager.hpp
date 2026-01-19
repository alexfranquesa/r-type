#pragma once

#include <SFML/Audio.hpp>
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

#include "engine/audio/audio_interface.hpp"

namespace client::systems {

// SFML-based implementation of the generic audio interface.
class AudioManager : public engine::audio::IAudio {
public:
    AudioManager() = default;
    
    // Cargar y reproducir música
    bool load_menu_music();
    bool load_game_music();
    void play_menu_music();
    void play_game_music();
    void stop_music();
    void set_music_volume(float volume) override; // 0-100
    
    // Sonidos de efectos
    bool load_button_click_sound();
    void play_button_click();
    void play_music(engine::audio::MusicId id) override;
    void play_sfx(engine::audio::SfxId id) override;
    void set_sfx_volume(float volume) override; // 0-100
    
    // Update para limpiar sonidos terminados
    void update();
    
private:
    // Música
    sf::Music menu_music_;
    sf::Music game_music_;
    sf::Music* current_music_ = nullptr;
    
    // Efectos de sonido
    sf::SoundBuffer button_click_buffer_;
    std::vector<sf::Sound> active_sounds_;
    
    float music_volume_ = 50.f;
    float sfx_volume_ = 50.f;
    
    bool try_load_music(sf::Music& music, std::initializer_list<std::filesystem::path> paths);
    bool try_load_buffer(sf::SoundBuffer& buffer, std::initializer_list<std::filesystem::path> paths);
};

} // namespace client::systems
