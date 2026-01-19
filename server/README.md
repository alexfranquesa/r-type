# server – Authoritative Server App

Purpose
-------
The server runs the authoritative simulation at a fixed tick (60 Hz). It receives inputs, updates the ECS, and sends snapshots to clients.

Main entry point
----------------
- `server/app/main.cpp`: initializes engine, starts UDP server, runs the fixed tick loop.

Key responsibilities
--------------------
- Accept clients (Hello/Welcome) and create player entities.
- Apply inputs via `ApplyInputSystem`.
- Run gameplay systems (movement, shooting, collision, health, spawn).
- Compute score/wave (stored in `GameStats`).
- Build and broadcast snapshots.

Networking
----------
- `server/app/network_server.*`: UDP server, client tracking, timeout cleanup.
- `server/systems/apply_input_system.*`: mapping from player_id to entity_id and input masks.

Rules
-----
- The server is authoritative: final decisions about damage, death, and spawns happen here.
- The game loop must not block on networking.
