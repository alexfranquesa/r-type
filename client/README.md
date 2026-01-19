# client – Graphical Client App

# client – Graphical Client App

Purpose
-------
The client displays the game and sends input. It does not own gameplay decisions; the server is authoritative. Rendering and audio go through interfaces (`IRenderer`, `IAudio`) to keep gameplay SFML-free.

Main entry points
-----------------
- `client/app/app_runner.cpp`: thin main runner (argc/argv parsing + call into context).
- `client/app/context/`: init, event handling, rendering, shutdown.
- `client/app/ui/`: helpers to build ECS-based lobby/settings UI.

Key responsibilities
--------------------
- Connect to server (Hello/Welcome) and keep heartbeat.
- Send Input packets on key changes.
- Apply snapshots to local ECS (`SnapshotApplySystem`) and store stats (score/wave/level).
- Render all entities (Position + Sprite) via `IRenderer`.
- Draw HUD (players/enemies/wave/score, HP bars) and accessibility overlay.
- Lobby/settings UI in ECS (connect/quit, host/port, level select, audio sliders).
- Play music/SFX via `IAudio` (menu/game tracks, shoot/explosion).

Systems (selection)
-------------------
- `SnapshotApplySystem`: decodes blob and updates ECS; stores `GameStats`.
- `RenderSystem`: SFML renderer implementation, used through `IRenderer`.
- `HUDSystem`: shows stats and HP bars.
- `UISystem` / `UIRenderSystem`: ECS-based lobby/settings UI.
- `InputSystem`: captures keyboard → input mask → network client.

Rules
-----
- Client may smooth visuals, but does not decide gameplay outcomes.
- Avoid putting gameplay logic in the client.
- Do not call SFML directly from gameplay systems; route rendering/audio through the adapters.
