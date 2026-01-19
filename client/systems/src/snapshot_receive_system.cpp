#include "snapshot_receive_system.hpp"
#include "../../app/network_client.hpp"

namespace client::systems {

void SnapshotReceiveSystem::run(NetworkClient& client,
                                rtype::ecs::registry& reg,
                                SnapshotApplySystem& apply) {
    while (auto snapshot = client.poll_snapshot()) {

        // Store server pause state
        paused_ = snapshot->paused;

        // Only apply snapshot if game is NOT paused
        if (!snapshot->paused) {
            apply.apply(reg, *snapshot);
        }
    }
}


}  // namespace client::systems
