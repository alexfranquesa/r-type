# Contributing Guide

This project targets a professional-grade delivery for the R-Type subject. Follow the workflow and quality gates below to keep the team aligned and Part 0 compliant.

## Branching Strategy

- `main` – protected, tagged only for validated milestones/demos.
- `develop` – integration branch. All feature work merges here via PR.
- `feature/<topic>` – short-lived branches per issue (e.g. `feature/ecs-registry`, `feature/network-hello`).

Steps for new work:

1. Pick or create a GitHub issue (epic or task) and self-assign.
2. Branch from `develop`: `git checkout -b feature/<topic> develop`.
3. Commit progress referencing the issue number (e.g., `feat: add sparse_array (#7)`).
4. Open a PR into `develop` when ready. Request review from the relevant owner (see responsibilities table).
5. CI must pass before merging.

No force-pushes to `main`/`develop`. Rebase your feature branch as needed.

## Commit & PR Guidelines

- Keep commits scoped (one concern each) and use conventional-ish prefixes (`feat`, `fix`, `chore`, `docs`, `refactor`).
- Reference issues in either the commit body or PR description.
- PR description template:
  - Problem / feature intent.
  - Summary of changes.
  - Testing (commands executed, screenshots if UI).
  - Checklist (docs updated? new deps?).
- Require at least one reviewer approval (two for risky changes: networking, protocol, ECS core).

## Code Style & Tooling

- C++20, warnings-as-errors during CI (via `rtype_enable_warnings`).
- Run `cmake --build --preset <preset>` locally before requesting review.
- clang-format / clang-tidy config will be added soon; until then follow brace + naming conventions shown in current files.
- Tests will use doctest; add them under `tests/` once the framework is wired.
- For scripts, prefer POSIX shell or Python. Keep them idempotent.

## Dependencies & Environment

- Use vcpkg manifest mode only. Do not add hand-rolled third-party code without discussion.
- Always run `./scripts/setup_vcpkg.sh` after cloning to ensure the expected baseline.
- Keep `CMakePresets.json` as the authoritative source for configure/build commands.

## Documentation

- Update `README.md` if setup or run instructions change.
- Update `docs/ARCHITECTURE.md`, `docs/protocol.md`, or `docs/milestone_mvp.md` when architecture/protocol/milestone scope changes.
- Accessibility decisions (input remap, UI contrast) must be recorded in docs and PRs.

## Communication

- Daily stand-up (max 15 min): yesterday / today / blockers.
- Sync meeting agendas + notes tracked in Notion.
- Use GitHub Discussions or PR comments for technical debates to keep an audit trail.

By keeping this workflow, we satisfy Part 0 expectations (self-contained project, clear workflow, documentation, and cross-platform readiness).
