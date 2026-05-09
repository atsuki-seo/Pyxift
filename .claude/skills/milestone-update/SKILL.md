---
name: milestone-update
description: Update Pyxift's milestone ledgers — `docs/status.md` (Roadmap / Open tasks / Open questions / Future considerations) and `docs/decisions.md` (settled design decisions). Invoked when the user runs `/milestone-update`, or whenever they say things like "record this decision", "add a follow-up task", "update the roadmap", "this finding should go into status/decisions", "we agreed on X — write it down", or otherwise want their in-conversation conclusions persisted to the project ledgers. Use this skill proactively whenever a chat produces a durable design decision or a new open task that the codebase memory should retain across sessions, even if the user doesn't explicitly name a file. Marking a milestone complete (the `M<n>:` empty commit) is intentionally NOT part of this skill.
---

# milestone-update

Pyxift's two long-lived ledgers are:

- `docs/status.md` — what is **still pending**: Roadmap, Open tasks, Open questions, Future considerations, Pre-release-tag checklist.
- `docs/decisions.md` — what has been **settled**: design decisions that survive across sessions. New entries are typically appended under the appropriate existing section, or in a new dated subsection at the end.

This skill takes the user's in-conversation findings (a spec investigation result, a settled question, a newly discovered follow-up task, a roadmap shift, etc.) and writes them into the right ledger via a **propose → confirm → apply** loop. The placement decision is non-trivial — the same input can plausibly belong to either file, and getting it wrong fragments the project memory — so the skill always lets the human approve the placement before editing.

What this skill is NOT:

- It does not mark milestones complete. The `M<number>: ...` empty commit (per `CLAUDE.md`'s "Milestone Conventions") is a separate manual step the user makes after implementation lands.
- It does not run `/pyxel-sync` or `/copyright-sync`. If the conversation produces an upstream-sync decision, this skill records the decision; the actual sync work belongs to the dedicated skills.
- It does not commit. Producing a clean diff for the user to review and commit themselves keeps the existing `git-commit.md` "smallest logical units" workflow intact.

## Invocation patterns

Two shapes are supported:

1. **With a topic argument** — e.g. `/milestone-update audio buffer underrun investigation`. Treat the argument as a hint about what to record; pull the actual content from the recent conversation context.
2. **Without arguments** — e.g. `/milestone-update`. Re-read the recent conversation and ask the user which finding(s) to persist if it isn't obvious.

In either case, if the conversation context is insufficient (skill invoked in a fresh session with no preceding discussion), ask the user to paste or summarize the material to be recorded.

## Workflow

### Step 1 — Read the current ledger state

Always start by reading both files end-to-end:

- `docs/status.md`
- `docs/decisions.md`

Knowing the existing sections, heading levels, and prose style is what makes the proposal feel native rather than bolted-on. Pyxift's ledgers tend to be compact and prose-first (not bullet-checklist-heavy), so match that texture.

### Step 2 — Classify the input

For each piece of information the user wants recorded, decide which ledger it belongs to:

| Input shape | Likely ledger | Likely section |
|---|---|---|
| "We decided that X works like Y" | `decisions.md` | An existing topical section (Audio, Drawing, Input, etc.), or a new subsection if no fit |
| "Investigation found that upstream does Z" — and we agree to follow it | `decisions.md` | Cross-check with upstream Pyxel |
| "Investigation found that upstream does Z" — but we deferred deciding | `status.md` | Open questions |
| "We need to do W later" | `status.md` | Open tasks (with milestone tag if known) or Roadmap (if version-scoped) |
| "Tag v0.x.0 needs an extra check" | `status.md` | Pre-release-tag checklist (or a release-specific section) |
| "After v0.1.0 we should revisit ..." | `status.md` | Future considerations |
| "The next milestone M\<n\> targets vX.Y.Z" (confirming a tentative version from `/next-milestone`, or revising one already on the Roadmap) | `status.md` | Roadmap (and the matching Open tasks entry's `target vX.Y.Z` tag) |

When the same finding has both a settled-decision part and a leftover-task part, split it: one entry in `decisions.md` and a separate one in `status.md` referencing it. This keeps each ledger answering its own question ("what's pending?" vs. "what's settled?").

If classification is genuinely ambiguous, present both options to the user with the trade-off and let them pick.

When recording or revising a milestone's target version, apply the rules in `docs/decisions.md` → "Versioning policy" (patch / minor / major; `v0.x` does not absorb breaking changes into minor bumps). If the proposed version conflicts with that policy, flag the conflict in the proposal rather than silently writing it.

### Step 3 — Draft the proposal

Produce a single proposal block that shows, for each ledger touched:

- The target file
- The section heading the new content goes under (or a new heading if needed)
- The exact text to insert (in English, per `.claude/rules/language-policy.md`), formatted to match neighboring entries
- A one-line rationale for the placement

Example proposal shape:

```
Proposal:

1. docs/status.md — under "## Open tasks":
   - "(M5, target v0.3.0) Investigate SDL3 audio underrun on Linux when frame time spikes above 33ms."
   Rationale: it's a concrete follow-up tied to an unstarted milestone, not yet a settled approach.

2. docs/decisions.md — under "## Audio (v0.2 and later)":
   "Buffer size is 2048 frames at 22050 Hz (≈93 ms latency). Smaller sizes caused
   underruns on Linux during frame-time spikes; larger sizes pushed input-to-sound
   latency past the perceptual threshold."
   Rationale: a settled trade-off, useful to future readers wondering why the value isn't 1024.
```

Then ask the user something like: "Apply as-is, or want to tweak wording / placement first?"

### Step 4 — Apply on confirmation

After the user approves (with or without edits), make the edits using the `Edit` tool. Stay surgical — append within the matching section, keep blank-line conventions consistent with the surrounding text, and do not reflow unrelated paragraphs.

After applying, surface the resulting `git diff` so the user can read the final state and decide how to commit. Per the project's git rules, do not commit automatically.

### Step 5 — Cross-link if needed

If the new entry references another (e.g. an Open task points at a Decisions entry, or vice versa), include the cross-reference inline as a relative anchor: `see "Audio (v0.2 and later)" in decisions.md`. This keeps the two ledgers navigable without duplicating prose (an SSOT concern — the actual content lives in one place).

## Style guidance

- Match the existing voice: short prose paragraphs, parenthetical milestone tags like `(M7, target v0.4.0)`, no emoji, no first person.
- Prefer extending an existing section to creating a new heading. New top-level sections in either file are rare and should usually prompt a confirmation question on their own.
- Roadmap entries belong in the existing "Roadmap from vX.Y onward" list and follow its established shape: `vX.Y.Z: <one-line summary> (M<n>)` — version first, milestone tag in parentheses at the end. Do not invert the order.
- Open questions are framed as questions or as "revisit when ..." notes — they are explicitly things still un-decided.

## Out of scope

- Marking milestones complete (`M<n>:` empty commit) — done manually per `CLAUDE.md`.
- Editing `docs/pyxel-reference.md` — owned by `/pyxel-sync`.
- Editing copyright notices — owned by `/copyright-sync`.
- Editing `CLAUDE.md` itself, README, or any source file — out of scope; this skill is ledgers-only.
- Running tests / builds — unrelated.

## Related Files

- `docs/status.md` — pending work ledger
- `docs/decisions.md` — settled decisions ledger
- `CLAUDE.md` — "Files to Read Before Starting Work" lists both ledgers as session-startup reading
- `.claude/rules/language-policy.md` — entries must be written in English
- `.claude/skills/pyxel-sync/SKILL.md` — adjacent skill that may produce decisions this skill then records
