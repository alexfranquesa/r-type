# cmake – Build Configuration

Purpose
-------
This folder hosts helper CMake modules and build presets used by the project.

What to look at
--------------
- `CMakeLists.txt` (root): defines targets `rtype_engine`, `rtype_server`, `rtype_client`.
- `CMakePresets.json`: cross-platform presets (linux/windows, debug/release).
- `cmake/`: helper modules (warnings, toolchain helpers).

Rules
-----
- Keep the build self-contained; dependencies are handled via vcpkg.
- Avoid hard-coding absolute paths.
