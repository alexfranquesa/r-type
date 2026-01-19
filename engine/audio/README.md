# engine/audio – Audio Abstraction

Purpose
-------
`engine/audio` defines the `IAudio` interface used by the client to play music and sound effects without leaking SFML into gameplay code.

Key files
---------
- `include/engine/audio/audio_interface.hpp`: interface types (`MusicId`, `SfxId`) and `IAudio` contract (`play_music`, `play_sfx`, `set_music_volume`, `set_sfx_volume`, `stop_music`).

Rules
-----
- Gameplay systems must not call SFML audio directly; use the interface from the client glue.
- Implementations live in the client (AudioManager wraps SFML audio).
- No networking or gameplay logic here.
