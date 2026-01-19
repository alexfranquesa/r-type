# engine/game – Gameplay Components & Systems

Purpose
-------
`engine/game` contains R-Type gameplay components and systems. It depends on `engine/core` but must not depend on SFML or low-level networking.

Component folders
-----------------
- `engine/game/components/core`: data used by render/movement (`Position`, `Velocity`, `Sprite`).
- `engine/game/components/gameplay`: gameplay data (`Health`, `Lives`, `Collider`, `Projectile`, `FactionComponent`, `InputState`, `EnemyType`, `UltimateCharge`, etc.).
- `engine/game/components/ui`: UI data (`UITransform`, `UIText`, `UIButton`).
- `engine/game/components/network`: network ownership (`Owner`).
- `engine/game/components/gameplay/game_stats.hpp`: global score/wave/level counters for HUD/snapshots.

System folders
--------------
- `engine/game/systems/world`: world-level systems (movement, cleanup).
- `engine/game/systems/gameplay`: gameplay systems (shooting, spawn, collision, health, level manager, boss/ice/lava/asteroid spawns).
- `engine/game/systems/network`: snapshot builder/send (server-side).

How to add a new gameplay system
--------------------------------
1. Declare in `engine/game/include/engine/game/systems/<domain>/your_system.hpp`.
2. Implement in `engine/game/src/<domain>/your_system.cpp`.
3. In the server loop (`server/app/main.cpp`), instantiate and call `run(registry, dt)`.
4. Keep systems focused: read a few components, write a few components.

Example: collision system logic
-------------------------------
`CollisionSystem` uses `Position` + `Collider` to detect AABB overlap, then reduces `Health` or destroys entities. It does not render or send network packets; those are separate systems.

Rules
-----
- No direct SFML usage in gameplay components/systems.
- No direct socket usage in gameplay systems.
- Gameplay systems should not modify rendering helpers directly.
