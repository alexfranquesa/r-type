# Protocol (current implementation)

Binary protocol over UDP, little-endian. One message per datagram.

## Common Header
| Field    | Type      | Description                                      |
|----------|-----------|--------------------------------------------------|
| `magic`  | `uint16_t`| Constant `0xCAFE` to filter garbage traffic.     |
| `version`| `uint8_t` | Protocol version (current = `1`).                |
| `type`   | `uint8_t` | Message type (`MessageType` enum below).         |
| `seq`    | `uint32_t`| Per-sender sequence number, increments per msg.  |

Total header size: 8 bytes.

## Message Types
- `0` **Hello** (client → server)
- `1` **Welcome** (server → client)
- `2` **Input** (client → server)
- `3` **Snapshot** (server → client)
- `4` **Ping** (bidirectional, heartbeat/RTT)
- `5` **Disconnect** (optional client notice)

### Hello (type 0, client → server)
| Field         | Type        | Notes                      |
|---------------|-------------|----------------------------|
| `player_name` | `char[16]`  | UTF-8, zero-padded.        |

### Welcome (type 1, server → client)
| Field         | Type        | Notes                       |
|---------------|-------------|-----------------------------|
| `player_id`   | `uint16_t`  | Assigned ID for this client |
| `tick_rate`   | `uint16_t`  | Server ticks per second     |

### Input (type 2, client → server)
| Field            | Type        | Notes                                       |
|------------------|-------------|---------------------------------------------|
| `player_id`      | `uint16_t`  | Mirrors `Welcome.player_id`                 |
| `input_mask`     | `uint16_t`  | Bit 0=Up, 1=Down, 2=Left, 3=Right, 4=Shoot  |
| `client_time_ms` | `uint32_t`  | Client timestamp (ms) for reconciliation    |

### Snapshot (type 3, server → client)
| Field     | Type        | Notes                                    |
|-----------|-------------|------------------------------------------|
| `tick`    | `uint32_t`  | Server tick of this snapshot             |
| `flags`   | `uint8_t`   | Bit 0: full snapshot (1=full, 0=delta)   |
| `paused`  | `uint8_t`   | 1 if the game is paused, else 0          |
| `blob`    | `bytes`     | Packed entity records + stats (see below)|

**Entity record (packed inside `blob`, repeated):**
| Field             | Type        | Notes                                                |
|-------------------|-------------|------------------------------------------------------|
| `entity_id`       | `uint16_t`  | ECS index used as network ID                         |
| `x`, `y`          | `float`     | Position                                             |
| `vx`, `vy`        | `float`     | Velocity                                             |
| `hp_cur`, `hp_max`| `int16_t`   | Health; -1/-1 if not present                         |
| `sprite_id`       | `uint16_t`  | Encoded sprite (player/enemy/projectile/hazard/etc.) |
| `owner_id`        | `uint16_t`  | Player owner (0 = server-owned entities/enemies)     |
| `lives_cur`, `lives_max` | `int16_t` | -1/-1 if not present                         |
| `is_spectating`   | `uint8_t`   | 1 if this entity is a spectator                     |
| `ultimate_frame`  | `uint8_t`   | Animation frame for ultimate projectiles            |
| `ultimate_ready`  | `uint8_t`   | 1 if ultimate is ready, else 0                      |

**Game stats (appended once at the end of `blob`):**
| Field              | Type        | Notes                                 |
|--------------------|-------------|---------------------------------------|
| `score`            | `uint32_t`  | Global score                          |
| `wave`             | `uint16_t`  | Current wave                          |
| `current_level`    | `uint16_t`  | Current level                         |
| `kills_this_level` | `uint16_t`  | Kills accumulated in this level       |
| `kills_to_next`    | `uint16_t`  | Kills required to advance level       |
| `total_kills`      | `uint16_t`  | Total kills overall                   |

*Full snapshots (flags=1) let the client cull missing entities; deltas (flags=0) only send changed entities.*

### Ping (type 4, bidirectional)
| Field         | Type        | Notes                     |
|---------------|-------------|---------------------------|
| `timestamp_ms`| `uint32_t`  | Echoed unchanged for RTT  |

## Typical Flows
- **Connect**: Hello → Welcome (assigns `player_id`).
- **Input**: Client sends mask on change; server applies to player entity.
- **Snapshots**: Server at 60 Hz sends full snapshots; client applies and cleans missing entities.
- **Ping/Heartbeat**: Client sends ping every 2s; server updates `last_seen` on any packet; server times out idle clients.

## Reliability / Resync
- UDP best-effort; inputs are stateless masks (resend latest on change).
- Snapshots are full; missing entities are culled client-side.
- No ACK/resent implemented; future work: delta snapshots + resend on seq gaps.

## Alignment with Code
- Header/types: `engine/net/packet.*`, `MessageType` enum.
- Hello/Welcome/Input structs: `engine/net/packet.hpp`.
- Snapshot serialization: `engine/game/src/network/network_send_system.cpp`.
- Snapshot application: `client/systems/src/snapshot_apply_system.cpp`.
