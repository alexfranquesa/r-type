#pragma once

#include <memory>
#include <vector>

#include <SFML/Graphics.hpp>

#include "engine/render/renderer_interface.hpp"
#include "render_system.hpp"

// Thin wrapper to adapt existing RenderSystem to the generic IRenderer interface.
class SfmlRenderer : public engine::render::IRenderer {
public:
    explicit SfmlRenderer(client::systems::RenderSystem& render_system, rtype::ecs::registry& registry, sf::RenderWindow& window)
        : render_system_(render_system), registry_(registry), window_(window) {}

    void begin() override {}

    void draw(const engine::render::SpriteView& s) override {
        // Convert SpriteView to ECS Sprite + Position temporarily
        // Note: For simplicity, we draw ad-hoc here without mutating ECS.
        auto tex = render_system_.sprite_bank().get(s.texture_id);
        if (!tex) {
            render_system_.sprite_bank().preload(s.texture_id);
            tex = render_system_.sprite_bank().get(s.texture_id);
        }
        if (!tex) return;
        sf::Sprite drawable(*tex);
        if (s.rect_width > 0 && s.rect_height > 0) {
            drawable.setTextureRect(sf::IntRect({s.rect_left, s.rect_top}, {s.rect_width, s.rect_height}));
        }
        drawable.setPosition({s.x, s.y});
        drawable.setScale({s.scale_x * (s.flip_x ? -1.f : 1.f), s.scale_y * (s.flip_y ? -1.f : 1.f)});
        drawable.setOrigin({s.origin_x, s.origin_y});
        drawable.setColor(sf::Color{s.color_r, s.color_g, s.color_b, s.color_a});
        window_.draw(drawable);
    }

    void draw_batch(const std::vector<engine::render::SpriteView>& sprites) override {
        for (const auto& s : sprites) draw(s);
    }

    void end() override {}

private:
    client::systems::RenderSystem& render_system_;
    rtype::ecs::registry& registry_;
    sf::RenderWindow& window_;
};
