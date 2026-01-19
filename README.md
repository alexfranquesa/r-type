# R-Type Engine & Game

Authoritative multiplayer remake of R-Type built in C++20. The project is split into a reusable engine (ECS + networking + rendering), a headless server that runs the full simulation, and a thin client that renders and captures input. This repository hosts everything needed to build and demo the MVP for the first defense.

## Repository Layout

```
engine/            Shared engine modules (core, net, render, game)
server/            Authoritative server application
client/            Graphical client application
docs/              Architecture, roadmap, protocol, milestone
scripts/           Helper scripts (vcpkg setup, etc.)
cmake/             CMake helper modules (warnings, future toolchain files)
vcpkg*.json        Manifest describing third-party dependencies
```

Key documents:

- [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) – module breakdown + runtime flows.
- [`docs/protocol.md`](docs/protocol.md) – UDP protocol draft.
- [`docs/roadmap.md`](docs/roadmap.md) – week-by-week plan with parallel milestones.
- [`docs/milestone_mvp.md`](docs/milestone_mvp.md) – checklist for the 15/12 MVP.
- [`docs/INDEX.md`](docs/INDEX.md) – per-folder developer docs.

Module guides (code-level entry points):
- [`engine/core/README.md`](engine/core/README.md) – ECS core (entity, registry, sparse_array).
- [`engine/game/README.md`](engine/game/README.md) – gameplay components and systems.
- [`engine/net/README.md`](engine/net/README.md) – networking helpers and serialization.
- [`engine/render/README.md`](engine/render/README.md) – rendering helpers + `IRenderer` (SFML integration).
- [`engine/audio/README.md`](engine/audio/README.md) – audio abstraction (`IAudio`).
- [`server/README.md`](server/README.md) – authoritative server app and loop.
- [`client/README.md`](client/README.md) – client app, input, snapshot apply, HUD/UI.
- [`scripts/README.md`](scripts/README.md) – helper scripts.
- [`cmake/README.md`](cmake/README.md) – build/presets overview.
- [`config/README.md`](config/README.md) – config conventions.
- [`sprites/README.md`](sprites/README.md) – assets guidelines.
- [`testing/README.md`](testing/README.md) – tests and how to run them.

## Prerequisites

- CMake ≥ 3.20
- A C++20 compiler (GCC 13+, Clang 15+, or MSVC 19.3x)
- Git
- Python is **not** required; the helper scripts are shell-based.
- On Linux: install `build-essential`, `pkg-config`, `git`, `curl`.
- On Windows: Visual Studio 2022 with C++ workload and Ninja (or use VS generators).

## Dependencies via vcpkg

We use [vcpkg](https://github.com/microsoft/vcpkg) in manifest mode to fetch:

- SFML 3 (graphics, window, audio, network)
- Asio (networking)
- fmt, spdlog (logging)
- doctest (future unit tests)

### Initial setup

```bash
# Clone vcpkg (pinned baseline) and bootstrap the executable
./scripts/setup_vcpkg.sh
```

On Windows (PowerShell / Git Bash):

```powershell
scripts\setup_vcpkg.sh   # via Git Bash
# or manually:
git clone https://github.com/microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat
```

The manifest (`vcpkg.json`) and baseline (`vcpkg-configuration.json`) keep everyone on the same versions.

## Configure & Build

### Linux

```bash
# Configure (Debug)
cmake --preset linux-debug

# Build server + client
cmake --build --preset linux-debug
```

### Windows

Ensure Ninja is installed (or switch the preset generator to Visual Studio).

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
```

Release presets (`linux-release`, `windows-release`) are also available.

Build artifacts reside in `build/<preset>/`.

### Continuous Integration

GitHub Actions workflows are prepared but currently disabled (billing limits). Before pushing, run locally:
```bash
# Linux
cmake --preset linux-debug && cmake --build --preset linux-debug
# Windows
cmake --preset windows-debug && cmake --build --preset windows-debug
```
If you re-enable Actions, use `.github/workflows/ci.yml` for Ubuntu/Windows with vcpkg manifest mode.

## Run (LAN / remote)

Server (listens on UDP 4242):
```bash
./build/linux-debug/rtype_server
```

Client (optional args: host, port, player_name):
```bash
# Localhost
./build/linux-debug/rtype_client 127.0.0.1 4242 PlayerA

# Another machine on the LAN (e.g., 10.48.255.122)
./build/linux-debug/rtype_client 10.48.255.122 4242 PlayerB
```
Windows: use the `windows-debug` preset and run the .exe equivalents (copy SFML DLLs from `vcpkg_installed/x64-windows/bin` next to the .exe or add that path to `PATH`).

Firewalls: open UDP 4242 on the server machine (ufw/iptables on Linux, Windows Firewall on Win).

Lobby UI: if you omit args, edit Host/Port in the lobby and click Connect. The server should log “New client …” and the client “Connected! Entering game…” when the handshake succeeds.

## Documentation & Planning

- `docs/ARCHITECTURE.md`: modules and server/client flows; HUD/lobby ECS.
- `docs/protocol.md`: message formats (Hello/Welcome/Input/Snapshot with score/wave, ping/heartbeat).
- `docs/roadmap.md` and `docs/milestone_mvp.md`: planning and checklist.
- `docs/STRUCTURE.md`: folder layout and ownership.

## Contributing

See [`CONTRIBUTING.md`](CONTRIBUTING.md) for workflow (main → develop → feature), code-style expectations, and review rules.

## Next Steps (MVP)

- Enemy shooting and damage to players.
- Refine HUD (enemy count/box layout) and reconnection cleanup.
- Final doc polish and cross-platform build check (Linux/Windows).
