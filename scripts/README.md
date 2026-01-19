# scripts – Helper Scripts

Purpose
-------
This folder contains helper scripts used to bootstrap dependencies and validate builds.

Common scripts
--------------
- `scripts/setup_vcpkg.sh`: clones and bootstraps vcpkg locally.
- `scripts/verify-ci-local.*`: optional local build checks (if present).

Rules
-----
- Keep scripts simple and documented.
- Avoid modifying system-wide packages; keep the project self-contained.
