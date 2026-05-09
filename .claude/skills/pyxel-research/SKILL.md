---
name: pyxel-research
description: Research a specific area of upstream Pyxel (kitao/pyxel) — for example the Music/MML spec, the input-event pipeline, the resource-bundle layout, or any other narrow topic — and append the findings to Pyxift's SSOT (`docs/pyxel-reference.md`). Invoked when the user runs `/pyxel-research <topic>` or `/pyxel-research`, and proactively whenever the user asks things like "look up how Pyxel does X", "investigate Pyxel's Y spec", "what does upstream Pyxel actually do for Z", "I need a reference for Pyxel's W", or otherwise wants a focused upstream spec dive captured into the project's reference document. This is Pyxel-specific — do not use it for generic third-party research. Always run this skill before re-implementing a Pyxel subsystem or before promoting an upstream behavior into a Pyxift milestone, so the SSOT picks up the spec.
allowed-tools: Bash(git -C ../pyxel:*) Bash(grep:*) Bash(rg:*) Bash(find:*) Bash(ls:*) Bash(wc:*)
---

# pyxel-research

Pyxift implements the Pyxel API independently, so any non-trivial subsystem we add (audio, input, resource format, drawing primitive, …) needs an explicit reference of what upstream actually does. This skill takes a single **topic** (e.g. "Music MML grammar", "Mouse wheel events", "Tilemap serialization") and produces a structured spec note that lands in `docs/pyxel-reference.md`, the SSOT for upstream-derived facts.

The skill is deliberately Pyxel-only. The upstream checkout is fixed at `../pyxel/`, the destination is fixed at `docs/pyxel-reference.md`, and the output style follows the existing sections of that file. Generic "go research something on the web" is out of scope.

## Invocation patterns

Two shapes are supported:

1. **With a topic argument** — `/pyxel-research music mml grammar`. Treat the argument as the research subject and start at Step 1 below.
2. **Without arguments** — `/pyxel-research`. Re-read the recent conversation; if a research subject is obvious, confirm it with the user, otherwise ask them to name the topic before proceeding.

If the conversation context is insufficient and the user does not provide a topic, stop and ask. Do not invent a topic.

## Workflow

The skill runs as a five-step **sync → locate → draft → confirm → record** loop. Do not skip steps; in particular, never write to `docs/pyxel-reference.md` before the user has approved the placement and wording.

### Step 1 — Sync the upstream checkout

Always start by invoking the `pyxel-ref-update` skill (no arguments) via the Skill tool. That skill is the single entry point for keeping `../pyxel/` pinned to the latest stable release tag, and it is cheap to call repeatedly. Capture the one-line `synced ../pyxel to <tag> (<date>)` result it prints — the resolved tag and date are recorded alongside the research note in Step 5 so the reader knows which upstream version the spec was extracted from.

If `pyxel-ref-update` reports `already at <tag>`, that is fine: continue with the existing checkout.

This mirrors the pattern used in `.claude/skills/pyxel-sync/SKILL.md` ("Auto-sync copyright-year notation" delegates to `copyright-sync` the same way).

### Step 2 — Locate the relevant upstream files

Pyxel is a Rust core (`crates/pyxel-core/`) wrapped in Python bindings (`crates/pyxel-wrapper/`, `python/pyxel/`). Most spec-bearing code lives in `crates/pyxel-core/src/`. The full tree is checked out (sparse-checkout was retired in favor of full clones), so feel free to read any path under `../pyxel/`.

Useful starting points by topic family:

- API surface / Python signatures → `python/pyxel/__init__.pyi`, `crates/pyxel-wrapper/src/`
- Numeric constants → `crates/pyxel-core/src/settings.rs`
- Audio (channels, tones, sounds, music, MML) → `crates/pyxel-core/src/{channel,oscillator,sound,music,audio}.rs`
- Drawing & canvas → `crates/pyxel-core/src/{canvas,graphics,image,tilemap}.rs`
- Input → `crates/pyxel-core/src/{input,key,keys}.rs`
- Resource format → `crates/pyxel-core/src/{resource,resource_data}.rs`
- Compression helpers → `crates/pyxel-core/src/utils.rs`

Use `rg` / `grep` from the repo root against `../pyxel/` to confirm the actual filenames for the current tag (the layout shifts release-to-release). Read each candidate file in full where practical — Pyxel's modules are small, and partial reads tend to miss subtle defaults.

Record the list of upstream paths consulted; they are needed for Step 5.

### Step 3 — Draft the spec note

Translate what you read into a concise reference note. The reader is a future Pyxift implementer who wants the upstream spec without having to reread Rust code. Aim for:

- **Verifiable facts only**: numeric constants, grammar rules, struct field layouts, default values, error conditions. Quote upstream symbol names verbatim (`DEFAULT_TONE_*`, `IMAGE_SIZE`, `compress_vec1`, etc.) so the next reader can grep for them.
- **Tables for enumerable data**, prose for procedural behavior. Match the visual style of the existing "Constraint values" and "Built-in font" sections.
- **Explicit confidence markers**. The user has asked that the note distinguish *settled* facts from *in-progress* understanding. Use one of these prefixes per bullet / row:
  - `[confirmed]` — directly read out of upstream source at the synced tag.
  - `[inferred]` — deduced from upstream behavior but not stated in code; include the reasoning.
  - `[todo]` — known unknown that still needs investigation. These are the candidates that get handed to `milestone-update` in Step 5.
- **Source links** at the end of the note: `Source: crates/pyxel-core/src/<file>.rs @ <tag>` for each file consulted.

### Step 4 — Decide placement and confirm with the user

`docs/pyxel-reference.md` currently has these top-level sections: "Upstream sync status", "Tracked-files table", "Default 16-color palette", "Constraint values", "Built-in font", "Background color", "Items not adopted". A new research note may fit one of three ways:

1. **Extend an existing section** when the topic is a natural addition (e.g. a new row in "Constraint values").
2. **Add a new top-level section** when the topic is a coherent new area (e.g. `## Music MML grammar`). Place it after the most-related existing section, before "Items not adopted".
3. **Add a sub-heading inside an existing section** for a small clarification.

Per the user's standing instruction, *file restructuring is allowed*: if the existing layout is fighting the new content, propose the restructure (renamed section, split file, new file under `docs/`) as part of the confirmation step. Default to option 1 or 2 — only restructure when there is a real reason.

Present to the user, in this order:

1. The chosen placement (which section, before/after which heading).
2. The full proposed insertion as a Markdown block.
3. The list of `[todo]` items, if any, that should be lifted into `docs/status.md` via `milestone-update`.

Wait for explicit approval before editing the file. If the user pushes back on placement or wording, iterate.

### Step 5 — Record the note and hand off follow-ups

After approval:

1. Insert the proposed Markdown block into `docs/pyxel-reference.md` at the agreed location. Do **not** touch the `<!-- pyxel-upstream-sync -->` block or the "Tracked-files table" — those are owned by `pyxel-sync`. If the research surfaced a new upstream file that Pyxift should start tracking, mention it as a follow-up but let the user run `/pyxel-sync` to register it.
2. End the inserted note with a one-line provenance footer:
   ```
   _Researched against kitao/pyxel <tag> (<date>) via /pyxel-research._
   ```
   using the tag and date captured in Step 1.
3. Do **not** commit. Per `~/.claude/rules/git-commit.md` and `CLAUDE.md`, commits stay in the user's hands so they can split into smallest logical units.
4. If Step 3 produced any `[todo]` rows, or if the research suggested a new milestone / open question / decision, prompt the user to run `/milestone-update` to persist them into `docs/status.md` or `docs/decisions.md`. Summarize the candidate entries so they can be pasted in directly. Do not invoke `milestone-update` automatically — placement there also requires human review.

## What this skill does NOT do

- It does **not** modify Pyxift source code. Re-implementing what was researched is a separate task (typically a milestone of its own).
- It does **not** update the Tracked SHA, the tracked-files table, or copyright years. Those are `pyxel-sync` / `copyright-sync` territory.
- It does **not** persist roadmap / decision entries on its own — that crosses into `milestone-update`.
- It does **not** research anything outside `../pyxel/`. For unrelated upstream projects, write a topic-specific skill or do it inline.
- It does **not** commit changes to `docs/pyxel-reference.md`. The user reviews the diff and commits.

## Related Files

- Reference SSOT: `docs/pyxel-reference.md`
- Upstream sync primitive: `.claude/skills/pyxel-ref-update/SKILL.md`
- Upstream drift detection: `.claude/skills/pyxel-sync/SKILL.md`
- Roadmap / decision ledger updater: `.claude/skills/milestone-update/SKILL.md`
- Upstream policy: `CLAUDE.md` ("Referencing the Upstream Pyxel Repository")
