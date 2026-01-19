# engine/core – ECS Core

Purpose
-------
`engine/core` holds the ECS primitives shared by server and client. It must not depend on SFML, networking, or gameplay logic. Standard library only.

Key files
---------
- `engine/core/include/engine/core/entity.hpp`: entity ID type and helpers.
- `engine/core/include/engine/core/types.hpp`: aliases for `entity_id_t`, etc.
- `engine/core/include/engine/core/sparse_array.hpp`: storage for components.
- `engine/core/include/engine/core/registry.hpp`: ECS registry API (`register_component`, `emplace`, `get`, `view`, `kill_entity`).
- `engine/core/include/engine/core/system.hpp`: optional system helpers.

How to add a component
----------------------
1. Create a struct in `engine/game/include/engine/game/components/...`.
2. Register it in the client or server registry (example in `client/app/main.cpp`).
3. Use `registry.emplace_component(entity, Component{...})` to attach it.

How to iterate entities
-----------------------
Use `reg.view<CompA, CompB>(lambda)` to iterate only entities with those components.

Example:
```cpp
reg.view<Position, Velocity>([&](std::size_t eid, Position& pos, Velocity& vel) {
    pos.x += vel.vx * dt;
    pos.y += vel.vy * dt;
});
```

Rules
-----
- No SFML types here.
- No direct references to game-specific logic.
- Keep APIs simple and readable (no heavy metaprogramming).
