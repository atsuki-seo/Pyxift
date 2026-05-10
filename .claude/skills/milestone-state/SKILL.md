---
name: milestone-state
description: Inspect Pyxift's milestone-and-release state across local git history and the origin remote, so callers like `/next-milestone` and `/milestone-update` can ground their decisions in the actual repo state instead of conversation memory. Reports the most recent `M<n>:` commit, the latest local and origin `vX.Y.Z` tags, and whether they are in sync. Read-only — never writes, never commits, never pushes.
---

# milestone-state

Single-responsibility primitive that captures "where is Pyxift right now?" in four lines, intended to be called from other skills (`next-milestone`, `milestone-update`) the same way `pyxel-ref-update` is called from `pyxel-sync`.

The need this skill answers: an `M<n>: vX.Y.Z ...` commit only becomes a real release tag after `.github/workflows/release-tag.yml` runs on `main` and creates the tag on origin. So a developer's local view ("I just made the M-commit, we're at v0.7.0 now") can disagree with the canonical state on origin ("the tag does not exist yet — CI hasn't finished"). Skills that propose the next milestone or the next version number must see both views to avoid double-counting or misnumbering.

## Output format

The skill emits exactly four lines, in this fixed order, on stdout:

```
Last M-commit: <subject> (<commit-date>)
Last M-commit: (none)                                                      ← when no M<n>: commit exists yet
Local tags:    <space-separated, newest first, up to 5>
Local tags:    (none)
Origin tags:   <space-separated, newest first, up to 5>
Origin tags:   (unavailable — offline or remote unreachable)
Status:        <one of the values listed below>
```

`Status:` is one of:

- `in sync` — the most recent M-commit is either a non-release form (`M<n>: foo` without a `vX.Y.Z`) or a release form whose corresponding tag is present on origin
- `M<n> pushed but origin tag missing` — the most recent M-commit is a release form (`M<n>: vX.Y.Z ...`) but origin has no matching tag (CI may be in progress, push may be missing, or CI may have failed)
- `local-only check` — `git ls-remote` could not reach origin; origin state was not consulted

The fixed shape is what makes the output easy for callers to parse — they grep `^Status:` and branch on the value.

## Workflow

### Step 1 — Last M-commit

```bash
LAST_M_COMMIT=$(git log --grep '^M[0-9]' --format='%H%x09%s%x09%cs' -1)
```

If the result is empty, emit `Last M-commit: (none)` and skip the release-tag check (treat status as `in sync` once origin tags are evaluated). Otherwise extract the subject and the committer date (`%cs` = `YYYY-MM-DD`).

Detect the release form with the same regex the project uses elsewhere:

```bash
RELEASE_FORM_RE='^M[0-9]+[a-z]?: v[0-9]+\.[0-9]+\.[0-9]+ '
```

If the subject matches, parse out the version (`vX.Y.Z`) for the status check below.

### Step 2 — Local tags

```bash
git tag --list 'v*' --sort=-v:refname | head -5
```

Pass through the result space-separated; if empty, emit `(none)`.

### Step 3 — Origin tags

Use a 3-second timeout so an offline / unreachable remote does not stall the caller:

```bash
ORIGIN_TAGS=$(timeout 3 git ls-remote --tags --refs origin 'v*' 2>/dev/null \
    | awk '{print $2}' \
    | sed 's@^refs/tags/@@' \
    | sort -V -r \
    | head -5)
ORIGIN_RC=$?
```

If the command succeeds and produces non-empty output, emit the tags space-separated. If it succeeds but produces no output, emit `(none)`. If it fails, times out, or any error path is hit (`ORIGIN_RC` non-zero, or `command -v timeout` missing), emit `(unavailable — offline or remote unreachable)` and remember that origin state is unknown for the status decision below.

### Step 4 — Compute Status

| Last M-commit subject | Origin reachable? | Tag for that version on origin? | Status |
|---|---|---|---|
| (none) | yes | n/a | `in sync` |
| (none) | no | n/a | `local-only check` |
| non-release form | yes | n/a (no version implied) | `in sync` |
| non-release form | no | n/a | `local-only check` |
| release form `vX.Y.Z` | yes | yes | `in sync` |
| release form `vX.Y.Z` | yes | no | `M<n> pushed but origin tag missing` |
| release form `vX.Y.Z` | no | unknown | `local-only check` |

Treat origin as "reachable" only if Step 3 produced a non-error result, regardless of whether the tag list was empty.

### Step 5 — Emit and return

Print the four lines to stdout in the order listed above. Do nothing else — no follow-up commits, no edits, no recommendations. The caller decides what to do with the result.

## Out of scope

- Editing any file (including the docs ledgers).
- Pushing, fetching beyond `git ls-remote`, or otherwise mutating local or remote state.
- Deciding the next milestone or next version — that belongs to `/next-milestone`.
- Recording milestones into the ledgers — that belongs to `/milestone-update`.
- Resolving the situation when `Status: M<n> pushed but origin tag missing` is reported — the caller (or the human) decides whether to wait for CI, investigate, or proceed regardless.

## Related Files

- Caller (next-target picker): `.claude/skills/next-milestone/SKILL.md`
- Caller (ledger writer): `.claude/skills/milestone-update/SKILL.md`
- Release-tag automation that creates the origin tags this skill checks: `.github/workflows/release-tag.yml`
- Milestone-commit format definition: `CLAUDE.md` ("Milestone Conventions")
