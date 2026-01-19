#include <doctest/doctest.h>
#include <vector>
#include <cstdint>
#include <cstring>

#include "engine/core/registry.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/velocity.hpp"
#include "engine/game/components/core/sprite.hpp"
#include "engine/game/components/gameplay/health.hpp"
#include "snapshot_apply_system.hpp"

namespace {
// Helpers to serialize little-endian
void write_u16(std::vector<std::uint8_t>& buf, std::uint16_t v) {
    buf.push_back(static_cast<std::uint8_t>(v & 0xFF));
    buf.push_back(static_cast<std::uint8_t>((v >> 8) & 0xFF));
}
void write_f32(std::vector<std::uint8_t>& buf, float f) {
    std::uint32_t bits;
    std::memcpy(&bits, &f, sizeof(f));
    buf.push_back(static_cast<std::uint8_t>(bits & 0xFF));
    buf.push_back(static_cast<std::uint8_t>((bits >> 8) & 0xFF));
    buf.push_back(static_cast<std::uint8_t>((bits >> 16) & 0xFF));
    buf.push_back(static_cast<std::uint8_t>((bits >> 24) & 0xFF));
}
void write_i16(std::vector<std::uint8_t>& buf, std::int16_t v) {
    buf.push_back(static_cast<std::uint8_t>(v & 0xFF));
    buf.push_back(static_cast<std::uint8_t>((v >> 8) & 0xFF));
}
}  // namespace

TEST_CASE("snapshot apply creates and updates components") {
    rtype::ecs::registry reg;
    client::systems::SnapshotApplySystem apply;

    // Build a blob for 1 entity with pos (10,20), vel (1,2), hp 5/10, sprite_id 0
    std::vector<std::uint8_t> blob;
    write_u16(blob, 1);          // count
    write_u16(blob, 3);          // entity id
    write_f32(blob, 10.f);       // x
    write_f32(blob, 20.f);       // y
    write_f32(blob, 1.f);        // vx
    write_f32(blob, 2.f);        // vy
    write_i16(blob, 5);          // hp_cur
    write_i16(blob, 10);         // hp_max
    write_u16(blob, 0);          // sprite_id (player/default)

    engine::net::SnapshotMessage snap{};
    snap.blob = blob;
    snap.tick = 0;
    snap.flags = 1; // full snapshot
    apply.apply(reg, snap);

    auto* pos = reg.try_get<engine::game::components::Position>(rtype::ecs::entity_t{3});
    REQUIRE(pos != nullptr);
    CHECK(pos->x == doctest::Approx(10.f));
    CHECK(pos->y == doctest::Approx(20.f));

    auto* vel = reg.try_get<engine::game::components::Velocity>(rtype::ecs::entity_t{3});
    REQUIRE(vel != nullptr);
    CHECK(vel->vx == doctest::Approx(1.f));
    CHECK(vel->vy == doctest::Approx(2.f));

    auto* hp = reg.try_get<engine::game::components::Health>(rtype::ecs::entity_t{3});
    REQUIRE(hp != nullptr);
    CHECK(hp->current == 5);
    CHECK(hp->max == 10);

    auto* sprite = reg.try_get<engine::game::components::Sprite>(rtype::ecs::entity_t{3});
    REQUIRE(sprite != nullptr);
    CHECK(sprite->visible == true);
}
