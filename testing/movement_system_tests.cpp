#include <doctest/doctest.h>

#include "engine/core/registry.hpp"
#include "engine/game/components/core/position.hpp"
#include "engine/game/components/core/velocity.hpp"
#include "engine/game/components/gameplay/input_state.hpp"
#include "engine/game/components/gameplay/faction.hpp"
#include "engine/game/systems/world/movement_system.hpp"

TEST_CASE("movement system applies input to position") {
    rtype::ecs::registry reg;
    const float dt = 1.0f;

    auto entity = reg.spawn_entity();
    reg.emplace_component<engine::game::components::Position>(entity, 0.f, 0.f);
    reg.emplace_component<engine::game::components::Velocity>(entity, 0.f, 0.f);
    auto& inputOpt = reg.emplace_component<engine::game::components::InputState>(entity);
    reg.emplace_component<engine::game::components::FactionComponent>(entity, engine::game::components::Faction::PLAYER);

    REQUIRE(inputOpt.has_value());
    inputOpt->up = true;
    inputOpt->left = true;

    rtype::game::MovementSystem system;
    system.run(reg, dt);

    auto* pos = reg.try_get<engine::game::components::Position>(entity);
    REQUIRE(pos != nullptr);
    // MovementSystem uses 200 units/s and normalizes diagonals by ~0.707
    CHECK(pos->x == doctest::Approx(-141.4f).epsilon(0.2f));
    CHECK(pos->y == doctest::Approx(-141.4f).epsilon(0.2f));
}
