# Comparative Study (Graphics, Networking, Packaging)

## Graphics / Windowing

| Option | Pros | Cons | Why we chose SFML 3 |
| --- | --- | --- | --- |
| SFML 3 | Simple API, good 2D support, input/audio/window in one, vcpkg package, CMake config ready | No official 3D, fewer high-end effects | We need fast 2D, easy input/audio, minimal boilerplate; SFML 3 fits and is packaged |
| SDL2 | Mature, wide platform support, lower-level control | Boilerplate for textures/render, addons for image/audio, mix with SDL_image/SDL_mixer | More setup for similar 2D needs; less straightforward for our timeline |
| Raylib | Very simple, quick prototyping | Less control over window/input granularity, smaller ecosystem | SFML gives us finer control and better integration with CMake/vcpkg |

## Networking

| Option | Pros | Cons | Why we chose Asio (standalone) |
| --- | --- | --- | --- |
| Asio (standalone) | Header-only, async UDP/TCP, well-documented, vcpkg package | Verbose handlers, requires discipline with threads | We only need UDP sockets + async receive/send; Asio is lightweight and available |
| ENet | Reliability over UDP, channels, built-in sequencing | Extra abstraction we don't need yet, custom protocol on top | Our protocol is simple UDP; Asio keeps full control and less dependency risk |
| Boost.Asio | Same API + TLS, timers, strands | Pulls Boost; heavier dependency | Standalone Asio avoids Boost footprint |

## ECS / Engine

| Option | Pros | Cons | Why we rolled our own |
| --- | --- | --- | --- |
| Custom ECS | Tailored to subject, zero external dep, matches bootstrap | Requires maintenance/tests | Subject explicitly wants ECS architecture; bootstrap guided implementation |
| entt | Very fast, feature-rich | Learning curve, template-heavy, extra dep | Overkill for scope; subject encourages own ECS |
| flecs | Data-oriented, reflection | Additional dep, different API style | Same as above; simpler to stay custom |

## Packaging / Dependencies

| Option | Pros | Cons | Why we chose vcpkg manifest |
| --- | --- | --- | --- |
| vcpkg (manifest) | Lockstep versions via baseline, cross-platform, integrates with CMake presets | Needs bootstrap step, caches can be large | Simple for both Linux/Windows, already supported by school environments |
| Conan | Powerful, profiles, reproducible | More setup time, server/cache config | vcpkg covers SFML/Asio/spdlog easily |
| System packages | Quick on Linux | Not portable to Windows, version drift | We need self-contained, cross-platform builds |

## Summary

- **SFML 3 + Asio + vcpkg + Custom ECS**: minimal dependencies, fast to prototype, matches subject constraints (self-contained, cross-platform, ECS enforced, server-authoritative UDP). 
- Trade-offs accepted: no reliability layer over UDP (we handle simple pings/full snapshots), no high-end rendering. For Part 2, we can add interpolation/reliability or switch to ENet if needed.
