# docs – Documentation Workflow

Purpose
-------
Core documentation lives in `docs/` and must be written in English. Per-module guides live next to the code (see `docs/INDEX.md`).

What to keep updated
--------------------
- `README.md`: quick start and build/run instructions.
- `ARCHITECTURE.md`: module boundaries and runtime flows.
- `protocol.md`: network protocol format.
- `comparative_study.md`: technology choices and trade-offs.
- `STRUCTURE.md`: folder layout and responsibilities.

Optional automation (Doxygen)
-----------------------------
If you want to generate API docs automatically:
1. Add a `Doxyfile` at the repo root.
2. Configure input to `engine/`, `server/`, `client/`.
3. Output to `docs/generated/` and publish it as static pages.

Doxygen is optional; the core documentation must still explain the architecture and workflows.
