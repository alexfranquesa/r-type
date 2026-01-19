#include "engine/game/systems/gameplay/game_over_system.hpp"
#include "engine/game/components/gameplay/lives.hpp"
#include "engine/game/components/network/owner.hpp"
#include <iostream>

namespace rtype::game {

void GameOverSystem::run(rtype::ecs::registry& reg, float dt) {
    (void)dt;  // Unused parameter
    
    // Don't check again if already game over
    if (game_over_) {
        return;
    }

    // Count players with lives > 0
    std::size_t alive_players = 0;
    std::size_t total_players = 0;

    reg.view<engine::game::components::Owner, engine::game::components::Lives>(
        [&](std::size_t /*eid*/, 
            engine::game::components::Owner& owner,
            engine::game::components::Lives& lives) {
            // Only count actual players (owner_id > 0)
            if (owner.player_id > 0) {
                total_players++;
                if (lives.remaining > 0) {
                    alive_players++;
                }
            }
        });

    // Game over if we have players but all are dead
    if (total_players > 0 && alive_players == 0) {
        game_over_ = true;
        std::cout << "[GameOverSystem] Game Over! All players eliminated." << std::endl;
    }
}

}  // namespace rtype::game
