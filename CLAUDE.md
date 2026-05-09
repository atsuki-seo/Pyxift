# Pyxift — Operating Rules for Claude

Project-specific guidance for Claude Code working in this repository.
Always start by reviewing `docs/decisions.md` (design decisions) and `docs/status.md` (open tasks and unresolved issues) before doing any work.

## Project Overview

A Pyxel-style retro 2D game engine (written in Swift, with a C++ core and an SDL3 backend). For detailed design decisions (target OS, language, distribution format, license), see `docs/decisions.md`.

## Language Policy

All artifacts committed to this repository must be written in English. See `.claude/rules/language-policy.md` for the full policy and the list of exceptions.

## Milestone Conventions

Steps to take when a milestone is complete:

1. Mark completion with an empty commit. The commit message must start with `M<number>[<suffix>]: ` (for example `M1:`, `M2a:`, `M13:`). Two forms exist:
   - **Release-tagging milestone** (commit will trigger an automatic version tag): include the version after `M<n>: `, e.g.
     ```
     git commit --allow-empty -m "M6: v0.3.0 asset bundle complete"
     ```
     Pattern: `^M[0-9]+[a-z]?: v[0-9]+\.[0-9]+\.[0-9]+ `.
   - **Intermediate milestone** (no tag): omit the version, e.g.
     ```
     git commit --allow-empty -m "M2a: input plumbing complete"
     ```
2. Do **not** run `git tag` manually. Pushing a release-tagging M-commit to `main` triggers `.github/workflows/release-tag.yml`, which runs the build with warnings-as-errors on macOS+Linux and creates the `vX.Y.Z` tag on success. `git tag` invocations from a developer machine are out of band.

Use `git log --grep '^M[0-9]'` to review completed milestones.

## Branching

- Commit directly to **main** (single-developer project, optimized for velocity).
- No PRs.
- Commit incrementally in the smallest logical units (per the existing `~/.claude/rules/git-commit.md` convention).

## Referencing the Upstream Pyxel Repository

The upstream code is expected to live in `../pyxel/` — the **parent directory** of the Pyxift repo (Pyxel itself is **written in Rust**).
The `/pyxel-ref-update` skill is the single primitive that prepares this checkout: it clones `../pyxel/` if missing, otherwise force-syncs the existing checkout to the latest stable release tag of kitao/pyxel (full checkout, `--depth=1`, no sparse-checkout). `/pyxel-sync` calls `/pyxel-ref-update` internally before doing any drift detection.

The "tracked-files table" in `docs/pyxel-reference.md` is the SSOT for which upstream paths are referenced by which Pyxift files. Whenever you add an implementation that draws on upstream code, you must register an entry in that table (`.claude/hooks/check-source-comment.sh` mechanically verifies that any file carrying an attribution comment is listed in the table). The table now functions purely as a **drift-detection filter** for `/pyxel-sync` — the diffs it surfaces are restricted to the paths it lists.
The tracked release tag and last-sync date live in the "upstream sync status" block of the same file, which is the SSOT for those values.

API compatibility and numeric-spec upstream sync are consolidated in the `/pyxel-sync` skill (`.claude/skills/pyxel-sync/SKILL.md`).

## Comment Style

Source-code comments follow `.claude/rules/comment-style.md`.

## Upstream-Derived Code — Attribution Comment Required

Any file containing `FONT_DATA`, `DEFAULT_COLORS`, or any `DEFAULT_TONE_*` symbol **must carry an attribution comment at the top of the file**:

```cpp
// Source: kitao/pyxel crates/pyxel-core/src/settings.rs
// License: MIT (Copyright (c) 2018-2026 Takashi Kitao)
```

`.claude/hooks/check-source-comment.sh` checks this before edits and warns if it is missing.

## C++ Core Layer: No SDL3 Imports

The C++ implementation is split into two layers:

- **Core layer** (`Sources/CPyxiftCore/src/core/`): pure C++, prioritizing testability.
  - Includes `InputState`, `AudioMixer`, `Canvas` (drawing logic), and so on.
  - `#include <SDL3/...>` is **forbidden**.
- **Adapter layer** (`Sources/CPyxiftCore/src/platform/`): SDL3-dependent.
  - Handles `SDL_Event` → `VirtualEvent` conversion, `SDL_AudioStream` wrappers, window management, etc.
  - `#include <SDL3/...>` is allowed.

`.claude/hooks/check-cpp-layer.sh` detects SDL3 imports in the core layer and warns.

## Swift 6 Strict Concurrency

- All Swift code is written under strict concurrency (`swift-language-mode: 6`).
- The game loop runs on a single `@MainActor`.
- C-function bridges are explicitly marked `nonisolated(unsafe)`.
- Running `swift build` on every file edit is too slow, so run `swift build -Xswiftc -warnings-as-errors` **manually at each milestone completion** to confirm zero warnings.

## API and Numeric Sync with Upstream Pyxel

Pyxift is an independent implementation aiming for Pyxel API compatibility. The triggers, detection scope, and diff-handling policy for upstream sync are consolidated in the `/pyxel-sync` skill (`.claude/skills/pyxel-sync/SKILL.md`). Copyright-year synchronization alone is split into a dedicated `/copyright-sync` skill (`.claude/skills/copyright-sync/SKILL.md`), which `/pyxel-sync` delegates to internally.

- Acknowledgments: `ACKNOWLEDGMENTS.md`. Full license text: `THIRD_PARTY_LICENSES/pyxel-MIT.txt`.

## Files to Read Before Starting Work

When Claude enters this repository in a new session, read the following in order:

1. `docs/decisions.md` — design decisions
2. `docs/status.md` — open tasks, unresolved issues, roadmap, pre-release-tag checklist
3. `docs/pyxel-reference.md` — numeric/data references reused from upstream and upstream-sync status
