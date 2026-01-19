# engine/net – Networking Helpers

Purpose
-------
`engine/net` contains the low-level networking helpers used by both server and client. It is intentionally gameplay-agnostic.

Key files
---------
- `engine/net/udp_socket.hpp`: thin wrapper around Asio UDP socket.
- `engine/net/packet.hpp`: message types, packet header, encode/decode helpers.
- `engine/net/serializer.hpp`: binary serialization helpers.
- `engine/net/thread_safe_queue.hpp`: queue used to pass inputs between threads.

Packet flow (high level)
------------------------
1. Client sends Hello → server replies Welcome (assigns player_id).
2. Client sends Input packets as input changes.
3. Server sends Snapshot packets at fixed tick rate.
4. Ping packets keep the connection alive and measure RTT.

Rules
-----
- No gameplay-specific data structures in this module.
- No SFML includes here.
- Keep protocol changes documented in `docs/protocol.md`.
