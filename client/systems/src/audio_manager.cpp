#include "audio_manager.hpp"

#include <algorithm>

namespace client::systems {

bool AudioManager::try_load_music(sf::Music& music, std::initializer_list<std::filesystem::path> paths) {
    for (const auto& path : paths) {
        if (std::filesystem::exists(path)) {
            if (music.openFromFile(path.string())) {
                std::cout << "[AudioManager] Loaded music: " << path.string() << std::endl;
                return true;
            }
        }
    }
    std::cerr << "[AudioManager] Failed to load music from provided paths" << std::endl;
    return false;
}

bool AudioManager::try_load_buffer(sf::SoundBuffer& buffer, std::initializer_list<std::filesystem::path> paths) {
    for (const auto& path : paths) {
        if (std::filesystem::exists(path)) {
            if (buffer.loadFromFile(path.string())) {
                std::cout << "[AudioManager] Loaded sound: " << path.string() << std::endl;
                return true;
            }
        }
    }
    std::cerr << "[AudioManager] Failed to load sound from provided paths" << std::endl;
    return false;
}

bool AudioManager::load_menu_music() {
    return try_load_music(menu_music_, {
        "sound_effects/menu_music.ogg",
        "../sound_effects/menu_music.ogg",
        "../../sound_effects/menu_music.ogg",
        "client/sound_effects/menu_music.ogg"
    });
}

bool AudioManager::load_game_music() {
    return try_load_music(game_music_, {
        "sound_effects/game_music.ogg",
        "../sound_effects/game_music.ogg",
        "../../sound_effects/game_music.ogg",
        "client/sound_effects/game_music.ogg"
    });
}

void AudioManager::play_menu_music() {
    stop_music();
    menu_music_.setLooping(true);
    menu_music_.setVolume(music_volume_);
    menu_music_.play();
    current_music_ = &menu_music_;
    std::cout << "[AudioManager] Playing menu music" << std::endl;
}

void AudioManager::play_game_music() {
    stop_music();
    game_music_.setLooping(true);
    game_music_.setVolume(music_volume_);
    game_music_.play();
    current_music_ = &game_music_;
    std::cout << "[AudioManager] Playing game music" << std::endl;
}

void AudioManager::stop_music() {
    if (current_music_) {
        current_music_->stop();
        current_music_ = nullptr;
    }
}

void AudioManager::set_music_volume(float volume) {
    music_volume_ = std::clamp(volume, 0.f, 100.f);
    if (current_music_) {
        current_music_->setVolume(music_volume_);
    }
}

bool AudioManager::load_button_click_sound() {
    return try_load_buffer(button_click_buffer_, {
        "sound_effects/button_click.ogg",
        "../sound_effects/button_click.ogg",
        "../../sound_effects/button_click.ogg",
        "client/sound_effects/button_click.ogg"
    });
}

void AudioManager::play_button_click() {
    if (button_click_buffer_.getSampleCount() == 0) {
        return;
    }
    
    // Limpiar sonidos terminados primero
    active_sounds_.erase(
        std::remove_if(active_sounds_.begin(), active_sounds_.end(),
                      [](const sf::Sound& s) { return s.getStatus() == sf::SoundSource::Status::Stopped; }),
        active_sounds_.end());
    
    // Crear nuevo sonido
    active_sounds_.emplace_back(button_click_buffer_);
    sf::Sound& sound = active_sounds_.back();
    sound.setVolume(sfx_volume_);
    sound.play();
}

void AudioManager::set_sfx_volume(float volume) {
    sfx_volume_ = std::clamp(volume, 0.f, 100.f);
}

void AudioManager::play_music(engine::audio::MusicId id) {
    if (id == engine::audio::MusicId::Menu) play_menu_music();
    else play_game_music();
}

void AudioManager::play_sfx(engine::audio::SfxId id) {
    if (id == engine::audio::SfxId::Button) {
        play_button_click();
    }
    // Otros SFX pueden añadirse aquí (shoot, explosion, etc.).
}

void AudioManager::update() {
    // Limpiar sonidos terminados
    active_sounds_.erase(
        std::remove_if(active_sounds_.begin(), active_sounds_.end(),
                      [](const sf::Sound& s) { return s.getStatus() == sf::SoundSource::Status::Stopped; }),
        active_sounds_.end());
}

} // namespace client::systems
