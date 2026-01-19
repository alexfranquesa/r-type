#pragma once

#include <cstdint>

namespace rtype::ecs {
class registry;
}

namespace rtype::game {

class UltimateChargeSystem {
public:
    void run(rtype::ecs::registry& reg);

private:
    static constexpr std::uint8_t kMaxKills = 3;
};

}  // namespace rtype::game
