#include <doctest/doctest.h>

#include "engine/core/registry.hpp"
#include "engine/core/entity.hpp"

struct Dummy {
    int value{};
};

TEST_CASE("registry basic lifecycle") {
    rtype::ecs::registry reg;

    // Register and emplace a component.
    auto entity = reg.spawn_entity();
    reg.emplace_component<Dummy>(entity, 42);

    auto* dummy = reg.try_get<Dummy>(entity);
    REQUIRE(dummy != nullptr);
    CHECK(dummy->value == 42);

    // View should visit the entity.
    int visited = 0;
    reg.view<Dummy>([&](std::size_t idx, Dummy& d) {
        ++visited;
        CHECK(static_cast<rtype::ecs::entity_id_t>(idx) == static_cast<rtype::ecs::entity_id_t>(entity));
        CHECK(d.value == 42);
    });
    CHECK(visited == 1);

    // Kill entity and verify component gone.
    reg.kill_entity(entity);
    auto* dummy_after = reg.try_get<Dummy>(entity);
    CHECK(dummy_after == nullptr);
}
