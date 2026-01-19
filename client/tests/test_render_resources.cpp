#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <filesystem>
#include "render_system.hpp"

TEST_CASE("RenderSystem initializes fallback texture") {
    // Crear un directorio temporal para los tests
    std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "rtype_test_sprites";
    std::filesystem::create_directories(test_dir);
    
    client::systems::RenderSystem rs{test_dir};

    auto tex = rs.texture_for("id_que_no_existe");
    CHECK(tex != nullptr);
    CHECK(tex->getSize().x == 1);
    CHECK(tex->getSize().y == 1);
    
    // Limpiar
    std::filesystem::remove_all(test_dir);
}

TEST_CASE("RenderSystem preload_all does not throw") {
    // Crear un directorio temporal para los tests
    std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "rtype_test_sprites2";
    std::filesystem::create_directories(test_dir);
    
    client::systems::RenderSystem rs{test_dir};
    CHECK_NOTHROW(rs.preload_all());
    
    // Limpiar
    std::filesystem::remove_all(test_dir);
}

TEST_CASE("RenderSystem loads existing texture file") {
    std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "rtype_test_sprites3";
    std::filesystem::create_directories(test_dir);
    
    // Create a simple 2x2 PNG file (minimal valid PNG)
    std::filesystem::path test_png = test_dir / "test_sprite.png";
    std::ofstream file(test_png, std::ios::binary);
    // Minimal PNG header and IEND chunk
    unsigned char png_data[] = {
        0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
        0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
        0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02,
        0x08, 0x02, 0x00, 0x00, 0x00, 0xFD, 0xD4, 0x9A,
        0x73, 0x00, 0x00, 0x00, 0x0C, 0x49, 0x44, 0x41,
        0x54, 0x08, 0xD7, 0x63, 0xF8, 0xFF, 0xFF, 0x3F,
        0x00, 0x05, 0xFE, 0x02, 0xFE, 0xDC, 0xCC, 0x59,
        0xE7, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E,
        0x44, 0xAE, 0x42, 0x60, 0x82
    };
    file.write(reinterpret_cast<char*>(png_data), sizeof(png_data));
    file.close();
    
    client::systems::RenderSystem rs{test_dir};
    auto tex = rs.texture_for("test_sprite");
    CHECK(tex != nullptr);
    
    std::filesystem::remove_all(test_dir);
}

TEST_CASE("RenderSystem caches textures") {
    std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "rtype_test_sprites4";
    std::filesystem::create_directories(test_dir);
    
    client::systems::RenderSystem rs{test_dir};
    auto tex1 = rs.texture_for("cached_sprite");
    auto tex2 = rs.texture_for("cached_sprite");
    
    CHECK(tex1 == tex2);  // Should return the same pointer
    
    std::filesystem::remove_all(test_dir);
}

TEST_CASE("RenderSystem handles multiple different textures") {
    std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "rtype_test_sprites5";
    std::filesystem::create_directories(test_dir);
    
    client::systems::RenderSystem rs{test_dir};
    auto tex1 = rs.texture_for("sprite1");
    auto tex2 = rs.texture_for("sprite2");
    auto tex3 = rs.texture_for("sprite3");
    
    CHECK(tex1 != nullptr);
    CHECK(tex2 != nullptr);
    CHECK(tex3 != nullptr);
    CHECK(tex1 != tex2);
    CHECK(tex2 != tex3);
    
    std::filesystem::remove_all(test_dir);
}
