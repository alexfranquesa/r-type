# Gameplay TODO (current state)

Track these issues until they are verified in-game.

## Known issues to fix
- **Menu flow**
  - Play should stop on level select (do not auto-start).
  - Selecting a level, backing out, then playing must not keep the old level.
- **Level bugs**
  - Level 4 occasionally reports level “4023”; fix level manager/state.
  - Aesthetic pass on levels/backgrounds; ensure backgrounds and sprites load (no white squares).
- **Settings**
  - Verify all settings apply (music/SFX volumes, difficulty/level sliders). Improve settings UI visuals.
- **AI / Gameplay**
  - Boss shooting AI should track the player while firing.
  - Enemies respawn too slowly; retune timers.
  - Sometimes player dies randomly; investigate collision/health flow.
- **Networking / lifecycle**
  - When a player leaves and rejoins, no ghost entity should remain visible (cleanup timeout/remove).
- **HUD / Feedback**
  - Ensure explosions/death feedback looks correct; fix hitbox alignment for projectiles vs enemies.

## Stability checks to run
- Connect/disconnect/reconnect multiple times; server cleans entities promptly.
- Level select → play → back to menu → play again uses the selected level.
- Progression through levels (including Level 4) keeps correct wave/score/level counters.
- Settings changes persist and take effect immediately (audio/difficulty).
- Boss/regular enemies shoot and damage correctly; player deaths trigger game over flow.
