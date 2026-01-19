# Roadmap (current focus)

This is the living plan after the refactors (render/audio interfaces, client context split). Update it as we close items.

## Stability & Architecture
- Enforce interface boundaries (`IRenderer`, `IAudio`, network client facade) and keep gameplay SFML-free.
- Harden connect/disconnect: no ghost players, clean registry reset, reliable level select → play flow.
- Keep Linux/Windows presets green; fix any build warnings before commits.

## Gameplay & Levels
- Fix level selection flow (no auto-jump, proper reset when returning to menu).
- Level/asteroid/lava/boss tuning: ensure sane progression and aesthetics; fix Level 4 overflow bug.
- Enemy/boss AI polish (targeting player when shooting), respawn pacing, collision/hitbox tuning.
- Game over/respawn correctness (no random deaths; clear feedback on death).

## Networking & Protocol
- Validate snapshot contents vs HUD (players/enemies/score/wave/level).
- Improve timeout/rejoin handling; ensure entities are removed promptly on leave.

## UI/UX & Audio
- Settings menu polish (visuals + sliders working: music/SFX volume, difficulty/level).
- Keep accessibility overlay functional (contrast/font scaling).
- Verify SFX/music hooks for menu/gameplay events.

## Documentation
- Keep `ARCHITECTURE.md`, `protocol.md`, module READMEs, and this roadmap in sync with code changes.
- Add short developer notes near new modules (audio, renderer interface, client context).
