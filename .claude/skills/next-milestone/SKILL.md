---
name: next-milestone
description: Decide what the next Pyxift milestone should target. Surveys upstream Pyxel's API surface (`../pyxel/python/pyxel/__init__.pyi`) against Pyxift's current Swift public API, merges in the existing `docs/status.md` ledger (Open tasks / Roadmap / Future considerations), and proposes a ranked Top-3 of candidate areas with reasoning so the user can pick what to work on next. Invoke when the user runs `/next-milestone`, just after marking a milestone complete with an `M<n>:` empty commit, or whenever they ask things like "what should we do next", "pick the next milestone", "what's the next thing to tackle", "where is Pyxift behind upstream", or otherwise want a structured proposal for the next chunk of work. Use this skill proactively right after an `M<n>:` commit lands, since that is the natural decision point. The skill is read-only against the codebase — it does not write to `docs/status.md` itself; that handoff goes to `/milestone-update` once the user picks a target.
---

# next-milestone

Pyxift's roadmap is not pinned by an external product manager — it is decided turn by turn from "what does upstream Pyxel offer that Pyxift hasn't implemented yet?" plus whatever follow-ups the team has already written down. This skill captures that decision moment as a repeatable workflow so the next target is chosen against a complete, current picture instead of from memory.

The hard part is **not** writing the decision down (that's `milestone-update`'s job) and **not** investigating the chosen area (that's `pyxel-research`'s job). The hard part is **enumerating the candidate space honestly** — including upstream APIs nobody has thought about lately — and ranking it. That enumeration is what this skill owns.

## When to invoke

- The user runs `/next-milestone` (with or without an argument hinting at a focus area, e.g. `/next-milestone audio`).
- An `M<n>: ...` empty commit was just made and the user is looking for what to tackle next.
- The user asks open-endedly "what should we do next" / "where are we behind upstream" / "pick the next milestone for me" — even without naming this skill.

If the conversation already picks a target before this skill runs (e.g. the user has been deep in audio work and clearly wants to continue there), it is fine to short-circuit: surface that as the obvious Top-1 candidate and proceed to handoff. Do not force the full survey when the answer is already in hand.

## Workflow

The skill runs as a **sync → enumerate → rank → confirm → handoff** loop. Steps 1–2 do read-only work; step 3 is where Claude takes a position; step 4 is the human decision point; step 5 hands off to other skills. Do not skip the human decision in step 4 — the point of this skill is that the human signs off on the target before any further work begins.

### Step 1 — Sync the upstream checkout

Invoke `pyxel-ref-update` (no arguments) via the Skill tool. This pins `../pyxel/` to the latest stable kitao/pyxel release tag, which is the basis for the API-diff in step 2. Capture the resolved tag and date from its output — surface them later so the user knows which upstream version the proposal was built against.

If `pyxel-ref-update` reports `already at <tag>`, that's fine; continue with the existing checkout.

### Step 2 — Enumerate candidates

This step builds a **candidate pool** by combining three sources. The result is a flat list of "things Pyxift could plausibly do next." Delegate the heavy reading to an `Explore` subagent so the main conversation context only takes the digested list, not the raw `.pyi` and Swift source.

**Subagent prompt template** (adapt as needed):

> Compare upstream Pyxel's API surface against Pyxift's current Swift public API.
>
> 1. Read `../pyxel/python/pyxel/__init__.pyi` and list every public symbol (functions, classes, methods, module-level constants).
> 2. Read Pyxift's public Swift surface under `Sources/Pyxift/` — look for `public func`, `public var`, `public struct`, `public class`, `public enum`. Match by name (Pyxel's `pyxel.btn(key)` ↔ Pyxift's `Pyx.btn(...)` etc.), accounting for the namespace shift from `pyxel.*` to `Pyx.*` / `Pyxift.*`.
> 3. Return a Markdown table with three columns: `Upstream symbol`, `Pyxift status` (`implemented` / `partial` / `missing`), `Notes` (brief — signature mismatch, deferred per status.md, etc.). Group rows by upstream module/class.
> 4. Also list any Pyxift public symbols that have **no** upstream counterpart, separately (these are extensions, not gaps).
>
> Read whatever supporting files you need to make accurate calls (e.g. `crates/pyxel-wrapper/src/` for signatures, Pyxift's `Sources/Pyxift/` for the Swift side). Do not edit anything. Return only the table and the extension list.

After the subagent returns, also pull in:

- **Open tasks** and **Roadmap** and **Future considerations** from `docs/status.md` — read directly in the main conversation; the file is short.
- The full **`docs/decisions.md`** — read directly in the main conversation. This is the SSOT for settled "we will *not* do X" calls (e.g. the "Math and RNG APIs" section explicitly lists upstream APIs Pyxift declines to provide). Without this read, the skill will repeatedly propose candidates that conflict with already-settled decisions.
- Any `[todo]` markers introduced into `docs/pyxel-reference.md` by recent `/pyxel-research` runs (grep for `\[todo\]`).

The raw candidate pool is: `(missing upstream APIs) ∪ (partial upstream APIs with notable gaps) ∪ (status.md open tasks) ∪ (status.md future considerations) ∪ (pyxel-reference [todo] items)`.

Then **filter the pool against `decisions.md`**: drop any candidate whose adoption would directly contradict a settled "do not provide" / "out of scope" / "rejected alternative" decision. Track the dropped items separately — the count and a one-line pointer to the relevant decisions.md section get surfaced in step 4 so the user can override the filter if they want to revisit a past decision.

A candidate is *not* dropped merely because it is deferred or unscheduled; only explicit non-adoption decisions filter it out. When unsure whether a decision is binding, keep the candidate in the pool and flag the tension inline rather than silently dropping it.

### Step 3 — Rank into a Top-3

Score each candidate along three loose axes and pick the three that look strongest. The scoring is **not** numeric and **not** a published rubric — it's a way to force Claude to take a position so the user has something to push back on.

- **Pyxel-relevance** — How visible is this in upstream's public API and docs? An API every Pyxel demo uses ranks higher than an obscure helper. (Approximation: how often is the symbol referenced from `python/pyxel/__init__.pyi` and the upstream examples directory? Don't grep exhaustively — eyeball it.)
- **Implementation cost** — Rough size class: small (one file, no new core subsystem), medium (touches both C++ core and Swift wrapper), large (new core subsystem, e.g. an audio feature requiring new mixer state). Smaller is preferred when other axes tie.
- **Dependency posture** — Is this blocking other deferred items? Items that unblock multiple downstream candidates rank higher. Items blocked by an unfinished prerequisite rank lower (and the prerequisite itself becomes the candidate instead).

Top-3 is a default, not a rule — surface 2 if there are clearly only two reasonable options, or 4–5 if the field is genuinely flat. Each entry should fit in 3–5 lines: one-line summary, why it ranked, the rough cost class, and any notable risk or dependency.

### Step 4 — Present and let the user pick

Present to the user, in this order:

1. The synced upstream tag and date (one line).
2. A one-paragraph framing of what the candidate pool looked like (e.g. "12 missing upstream APIs in the audio module, 3 status.md open tasks, 2 [todo] markers in pyxel-reference") so the user understands the shape of what was considered.
3. A one-line "decisions.md filter" report: how many candidates were dropped and a brief pointer (e.g. "filtered out 13 items per `decisions.md` → Math and RNG APIs, full coverage of key-code constants"). Omit the line only when zero items were dropped.
4. The Top-3 (or 2/4/5) with the reasoning sketch above.
5. An explicit "which do you want to take, or none of the above?" prompt — and remind the user they can re-include a filtered item by overturning the underlying decision (which would route through `/milestone-update` against `decisions.md`).

Wait for the user's pick. They may:

- Pick one of the Top entries → proceed to step 5 with that target.
- Pick something off the list (e.g. "actually I want to do X, which you didn't include") → proceed to step 5 with X. Do not relitigate the ranking.
- Ask to see more / different framing → iterate on the proposal.
- Decline to pick → end the skill cleanly. The proposal does not get persisted (the candidate pool is cheap to regenerate next time).

### Step 5 — Hand off

Once the target is picked, the rest of the work belongs to other skills. Do **not** edit `docs/status.md` or `docs/decisions.md` from this skill — that is `milestone-update`'s territory and gets its own human-confirmed propose/apply loop.

The handoff has two flavors depending on whether the target is well-understood:

- **Target needs upstream investigation first** (e.g. an API whose spec the user doesn't already know cold) — recommend the user run `/pyxel-research <topic>` to capture the spec into `docs/pyxel-reference.md`. State this as a recommendation; do not auto-invoke. Once research is done, the user will then invoke `/milestone-update` to add the new target to the roadmap.
- **Target is already well-understood** (e.g. an item already in `status.md` Open tasks, or a small cosmetic API) — recommend the user run `/milestone-update` directly to lift it onto the roadmap as the next milestone.

Summarize the suggested follow-up commands in 1–2 lines so the user can copy them. Do not invoke them automatically — `pyxel-research` and `milestone-update` both have their own confirmation steps that benefit from a fresh user-driven invocation.

## What this skill does NOT do

- It does **not** edit `docs/status.md`, `docs/decisions.md`, or `docs/pyxel-reference.md`. Each ledger has its own dedicated skill with its own confirmation loop, and routing through them keeps placement consistent across sessions.
- It does **not** start implementation. The chosen target is a **next milestone**, not an immediate code change. Implementation begins after the user moves on from this skill.
- It does **not** create the `M<n>:` empty commit. That is a manual step the user makes after implementation lands, per `CLAUDE.md`'s Milestone Conventions.
- It does **not** auto-invoke `pyxel-research` or `milestone-update`. Those are recommendations, not actions, so the user retains the propose/confirm step on each ledger touch.
- It does **not** look at issues or PRs on GitHub. Pyxift is single-developer with no PRs; the relevant signals are entirely in-repo and in `../pyxel/`.

## Related Files

- Roadmap / decision ledgers: `docs/status.md`, `docs/decisions.md`
- Upstream reference SSOT: `docs/pyxel-reference.md`
- Upstream sync primitive: `.claude/skills/pyxel-ref-update/SKILL.md`
- Upstream area research: `.claude/skills/pyxel-research/SKILL.md`
- Ledger updater: `.claude/skills/milestone-update/SKILL.md`
- Milestone conventions: `CLAUDE.md` ("Milestone Conventions")
