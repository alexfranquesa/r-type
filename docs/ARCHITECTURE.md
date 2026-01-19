# Architecture Overview

Project goal: an authoritative R-Type server, a thin graphical client, and a reusable C++20 engine with ECS, rendering, audio, networking, and gameplay subsystems.

```
┌────────┐      UDP       ┌────────┐
│Client  │ ─────────────► │ Server │
│engine::│ ◄───────────── │engine::│
│render  │  Snapshots     │core    │
└────────┘ Input masks    └────────┘
```

## Modules

- `engine::core`: ECS registry, sparse arrays, basic helpers (no SFML or gameplay).
- `engine::net`: UDP sockets, encode/decode, thread-safe queue.
- `engine::render`: thin SFML layer (window, textures, sprite bank) + `IRenderer` interface.
- `engine::audio`: `IAudio` interface (music/SFX), implemented in the client AudioManager.
- `engine::game`: shared components/systems (movement, shooting, collision, spawn, health, score/wave).
- `server`: authoritative at 60 Hz, applies inputs, runs gameplay, serializes snapshots with score/wave.
- `client`: render/input loop, applies snapshots to ECS, HUD (players/enemies/wave/score, HP bars), accessibility UI and lobby ECS. Uses `IRenderer`/`IAudio` adapters.

## Build Targets

- `rtype_engine` – library with engine/core/net/render/audio/game.
- `rtype_server` – authoritative binary (logic + networking).
- `rtype_client` – graphical binary (render + input + snapshot receive).

## Server Runtime Flow

1. Networking thread receives Hello/Input/Ping over UDP and pushes them into a thread-safe queue.
2. Game loop (60 Hz): ApplyInput → Movement → Shooting → Projectile → Collision (contact damage) → Health → EnemySpawn → SnapshotSend.
3. SnapshotSend builds a full snapshot (pos/vel/health/sprite_id/owner_id + score/wave) and broadcasts to all clients.

## Client Runtime Flow

1. Input: capture keyboard, update `input_mask`, send Input to server.
2. Networking: receive snapshots, apply to ECS (create/update/destroy entities; store score/wave/level).
3. Render: draw entities (Position + Sprite via `IRenderer`) + HUD (players/enemies/wave/score, HP bars) + accessibility UI. Lobby/settings use ECS UI helpers.
4. Audio: play music/SFX through `IAudio` (menu/game tracks, shoot/explosion).
5. Ping every ~2s to keep the connection alive and measure RTT.

## Data Contracts

- Shared: Position, Velocity, Sprite, Health, Faction, Projectile, Owner, GameStats (score/wave/level).
- Server-only: spawn timers, AI/boss state.
- Client-only: UI/accessibility state.
- Network: see `docs/protocol.md` (Hello/Welcome/Input/Snapshot/Ping).

## Current Next Steps

- Modular enforcement: prefer `IRenderer`/`IAudio` in client glue; keep gameplay SFML-free.
- Robust cleanup on disconnects and level resets.
- Boss AI and level progression polish; settings menu visuals and bindings.
