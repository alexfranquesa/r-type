# Project Structure Overview

This document summarizes what lives in each top-level module and how they are meant to interact. Keep this in sync as the codebase evolves. For per-folder guides, see `docs/INDEX.md`.

## engine/core
- ECS primitives: `entity`, `types` (entity_id), `sparse_array`, `registry` (add/get/remove/view), `SystemScheduler` + `ISystem`.
- No dependencies on SFML, networking, or gameplay. Standard library only.
- Shared by both server and client to manage entities/components/systems.

## engine/game
- Gameplay-facing components and systems, organized by domain:
  - `components/core`: `Position`, `Velocity`, `Sprite` (render-agnostic data).
  - `components/gameplay`: `Health`, `Collider`, `Projectile`, `InputState`, `FactionComponent`, `KillOnExitScreen`, `ScoreValue`, `AIBehavior`, etc.
  - `components/ui`: `UITransform`, `UIText`, `Scene`.
  - `components/network`: `NetworkOwned` (network identity).
  - `systems/world`: movement/cleanup.
  - `systems/gameplay`: shooting, collisions, health, enemy spawn, scoring, etc.
  - `systems/network`: snapshot build/send, receive/apply (to be filled).
- Knows about ECS but not SFML directly; render/network are kept outside.

## engine/net
- Networking primitives: UDP socket wrapper (Asio), packet header/types, serializer helpers, thread-safe queue.
- No gameplay or render knowledge. Used by server and client networking layers.

## engine/render
- Rendering helpers built on SFML: texture loader, sprite bank, render utilities, `IRenderer` interface.
- Knows SFML, but not gameplay or networking. Gameplay code should depend on the interface, not SFML types.

## engine/audio
- Audio abstraction: `IAudio` interface for music/SFX.
- Implemented in the client via an AudioManager that wraps SFML audio.
- No gameplay or networking logic here; expose high-level play/stop/volume only.

## server
- Application for the authoritative simulation.
- Networking loop (UDP), input queue, game tick at fixed rate, uses `engine::core` + `engine::game` + `engine::net`.
- Systems to apply inputs, run gameplay, build snapshots, and broadcast to clients.

## client
- Graphical application: SFML window, input capture, UI, render systems.
- Networking client for Hello/Input/Snapshot.
- Uses `engine::core`, `engine::game`, `engine::render`, `engine::audio`, `engine::net`.
- Organized under `client/app/`:
  - `context/` (init, events, render, runner),
  - `ui/` (helpers for ECS-based lobby/settings),
  - systems under `client/systems/` for render/UI/input/snapshot.

## docs
- `ARCHITECTURE.md`: high-level module/runtime flows.
- `protocol.md`: network protocol details.
- `roadmap.md`, `milestone_mvp.md`: planning.
- `STRUCTURE.md` (this file): module contents and responsibilities.
