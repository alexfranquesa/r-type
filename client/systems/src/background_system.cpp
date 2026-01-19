#include "background_system.hpp"
#include <iostream>
#include <algorithm>

namespace client::systems {

BackgroundSystem::BackgroundSystem() = default;

bool BackgroundSystem::load_texture(const std::string& texture_path) {
    if (!background_texture_.loadFromFile(texture_path)) {
        std::cerr << "[BackgroundSystem] Failed to load texture: " << texture_path << std::endl;
        return false;
    }
    
    std::cout << "[BackgroundSystem] ✓ Texture loaded from: " << texture_path << std::endl;
    std::cout << "[BackgroundSystem] ✓ Size: " << background_texture_.getSize().x 
              << "x" << background_texture_.getSize().y << std::endl;
    
    texture_loaded_ = true;
    
    auto tex_size = background_texture_.getSize();
    
    // Escalar para cubrir toda la ventana (asumiendo 1280x720)
    float scale_x = 1280.0f / static_cast<float>(tex_size.x);
    float scale_y = 720.0f / static_cast<float>(tex_size.y);
    float scale = std::max(scale_x, scale_y);
    
    std::cout << "[BackgroundSystem] ✓ Scale factor: " << scale << std::endl;
    
    // Crear los sprites ahora que tenemos la textura
    background_sprite1_ = std::make_unique<sf::Sprite>(background_texture_);
    background_sprite1_->setScale(sf::Vector2f(scale, scale));
    background_sprite1_->setPosition(sf::Vector2f(0.f, 0.f));
    
    background_sprite2_ = std::make_unique<sf::Sprite>(background_texture_);
    background_sprite2_->setScale(sf::Vector2f(scale, scale));
    background_sprite2_->setPosition(sf::Vector2f(1280.0f, 0.f));
    
    std::cout << "[BackgroundSystem] ✓ Background system initialized!" << std::endl;
    return true;
}

void BackgroundSystem::update(float delta_time) {
    if (!texture_loaded_ || !background_sprite1_ || !background_sprite2_) return;
    
    // Mover el offset
    offset_x_ -= scroll_speed_ * delta_time;
    
    // Cuando el primer sprite sale completamente de pantalla, reiniciar
    if (offset_x_ <= -1280.0f) {
        offset_x_ = 0.0f;
    }
    
    // Actualizar posiciones
    background_sprite1_->setPosition(sf::Vector2f(offset_x_, 0.f));
    background_sprite2_->setPosition(sf::Vector2f(offset_x_ + 1280.0f, 0.f));
}

void BackgroundSystem::render(sf::RenderWindow& window) {
    if (!texture_loaded_ || !background_sprite1_ || !background_sprite2_) return;
    
    window.draw(*background_sprite1_);
    window.draw(*background_sprite2_);
}

}  // namespace client::systems
