#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include "engine/core/registry.hpp"

namespace client::systems {

class BackgroundSystem {
public:
    BackgroundSystem();
    
    void update(float delta_time);
    void render(sf::RenderWindow& window);
    
    bool load_texture(const std::string& texture_path);
    
private:
    sf::Texture background_texture_;
    std::unique_ptr<sf::Sprite> background_sprite1_;
    std::unique_ptr<sf::Sprite> background_sprite2_;
    
    float scroll_speed_ = 20.0f;  // Velocidad de scroll en píxeles por segundo
    float offset_x_ = 0.0f;
    
    bool texture_loaded_ = false;
};

}  // namespace client::systems
