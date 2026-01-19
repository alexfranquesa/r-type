# testing – Tests

Purpose
-------
This folder hosts tests or debug utilities for the project.

How to run
----------
```bash
cmake --build --preset linux-debug
./build/linux-debug/rtype_tests
```

Notes
-----
- Prefer small, deterministic tests for ECS and serialization.
- Update this file when new tests are added.
