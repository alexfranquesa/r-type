# MVP Milestone – 15/12

This milestone summarizes every objective required for the first defense (3 credits). Use it as a checklist in Notion.

## Scope Overview

- **Authoritative server** running full gameplay loop (movement, shooting, enemy spawn, collision, death) at fixed tick with UDP networking.
- **Thin graphical client** handling rendering, input, interpolation, HUD, accessibility hooks.
- **Engine foundation** (ECS, render wrapper, networking module) reusable by both binaries.
- **Product quality** (Part 0): build/install docs, package manager, CI (Linux + Windows preset), accessibility basics.

## Deliverables

1. **Server**
   - Core loop @ 60 Hz (configurable).
   - Networking thread (UDP) with Hello/Input/Snapshot/Spawn/Destroy/Ping.
   - Gameplay systems: Movement, Shoot, EnemySpawn, Collision, Death, Cleanup.
   - Enemy waves + player projectiles + basic starfield logic.
2. **Client**
   - Connection screen + Hello handshake.
   - Snapshot receiver + ECS apply + interpolation/smoothing.
   - RenderSystem (sprites, starfield, projectiles, explosions), HUD (score, lives), basic audio.
   - InputSystem (keyboard remapping, key repeat handling) sending Input packets.
3. **Engine**
   - ECS (entity, sparse_array, registry, system runner, component masks).
   - engine::net socket wrapper + serializer helpers + thread-safe queues.
   - engine::render wrappers over SFML/SDL (window, textures, sprite manager, audio hooks).
   - Shared components definitions (Transform, Velocity, Sprite, PlayerTag, EnemyTag, Projectile, Health, InputState, etc.).
4. **Product / Part 0**
   - Conan/vcpkg lockfile installing deps automatically.
   - CMake presets for Linux + Windows (Debug/Release).
   - README (install, build instructions per OS, run instructions).
   - CONTRIBUTING + Git workflow description.
- CI job building server/client on Linux; plan for Windows (manual verification OK for MVP).
   - Accessibility doc + implemented basics: remappable input, readable UI (contrast/size), sound volume control.
   - Protocol documentation (`docs/protocol.md`) aligned with implementation.

## Checklist (Grouped)

### Tooling & Docs
- [ ] Package manager integrated (deps resolve via single command).
- [ ] cmake-presets + configure/build scripts.
- [ ] README/CONTRIBUTING complete for MVP.
- [ ] CI (Linux) + Windows preset validated manually.
- [ ] Accessibility guidelines documented + minimal features in client.

### Engine Core
- [ ] ECS registry + unit tests.
- [ ] Component library shared across modules.
- [ ] System scheduler (ordered execution, delta time).

### Networking & Server
- [ ] UDP socket abstraction, serializer, thread-safe queues.
- [ ] Hello/Input/Snapshot pipeline functional.
- [ ] Game loop deterministic @ 60 Hz with systems implemented.
- [ ] Enemy spawn logic + difficulty parameters tunable.
- [ ] Snapshot delta encoding + resend strategy.

### Client & Render
- [ ] SFML/SDL integration (window, textures, audio).
- [ ] Snapshot application + interpolation (Alex).
- [ ] RenderSystem draws players, enemies, projectiles, starfield.
- [ ] HUD + UI (score, lives, connection status, accessibility options).
- [ ] Input remap UI + network send.

### Testing & Demo Prep
- [ ] LAN session with ≥4 players stable.
- [ ] Resilience tests (disconnect, packet loss).
- [ ] Demo script + talking points (architecture, protocol, tooling, accessibility).

## Dependencies / Ownership

- **Mau (PM + Architecture + Tooling + Docs):** project structure (folders, CMake, vcpkg, presets), ECS core baseline, config/logging/Part 0 items, main documentation (ARCHITECTURE.md, PROTOCOL.md, README). Works alongside Enric on the server/network to ensure integration.
- **Enric (Networking + Authoritative Server):** UDP wrapper, serializer, thread-safe queues, protocol (Hello/Welcome/Input/Snapshot), fixed 60 Hz tick, server input application + snapshot generation, timeouts/disconnects/basic resend. Backed up by Mau.
- **Alex (Gameplay Core + Client Snapshots/Interpolation):** gameplay systems (movement, shooting, enemy spawn, projectiles, advanced collisions), health/death/score logic, receiving & applying snapshots on the client, interpolation/resync after lag.
- **Pol (Engine Support + Gameplay + Playtest QA):** support systems (HealthSystem, CleanupSystem, ScoreSystem, etc.), gameplay tuning (speed, timers, difficulty), assist with client-side feel, playtests (2–4 players) and bug reporting.
- **Lodo (Render + UI + Accessibility):** rendering pipeline (window, sprites, HUD), texture loader & sprite bank, lobby UI, network status UI (ping/connection/error), accessibility options (contrast, font size, remapping).

Use this list to seed the task board; each checkbox should map to tasks/subtasks with due dates.
