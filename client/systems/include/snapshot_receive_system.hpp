#pragma once

#include "snapshot_apply_system.hpp"

class NetworkClient;

namespace client::systems {

class SnapshotReceiveSystem {
public:
    // Poll snapshots from the client and apply them to the registry.
    void run(NetworkClient& client, rtype::ecs::registry& reg, SnapshotApplySystem& apply);

    bool is_paused() const { return paused_; }

private:
    bool paused_ = false; 
};


}  // namespace client::systems
