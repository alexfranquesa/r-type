#include <doctest/doctest.h>

#include "engine/game/components/gameplay/shoot_cooldown.hpp"

TEST_CASE("shoot cooldown helpers") {
    engine::game::components::ShootCooldown cd{};
    cd.cooldown_seconds = 1.5f;
    cd.remaining_seconds = 0.0f;
    cd.enabled = true;

    CHECK(cd.ready());

    cd.reset();
    CHECK(cd.remaining_seconds == doctest::Approx(1.5f));

    cd.remaining_seconds = 0.1f;
    CHECK(!cd.ready());

    cd.enabled = false;
    cd.remaining_seconds = 0.0f;
    CHECK(!cd.ready());
}
