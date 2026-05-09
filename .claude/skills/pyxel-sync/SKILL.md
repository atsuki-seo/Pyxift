---
name: pyxel-sync
description: Detect API and numeric-spec drift between upstream Pyxel (kitao/pyxel) and Pyxift's SSOT (`docs/pyxel-reference.md`), and assist in syncing them. Invoked when the user runs `/pyxel-sync`, or as part of the pre-release-tag checklist.
allowed-tools: Bash(git -C ../pyxel:*) Bash(grep:*) Bash(awk:*) Bash(sed:*)
---

# pyxel-sync

Pyxift is an independent implementation aiming for Pyxel API compatibility. This skill detects drift in the tracked upstream files between the previously-recorded release tag and the latest stable release tag of `kitao/pyxel`, and surfaces the diffs for human review.

Repository preparation (cloning / fetching / checking out a tag in `../pyxel/`) is **not** this skill's responsibility — it is delegated to the `pyxel-ref-update` skill.

## Tracked Files (SSOT)

The "tracked-files table" in `docs/pyxel-reference.md` is the SSOT for what is tracked. This skill parses that table into bash arrays. The table is delimited by the markers `<!-- pyxel-tracked-files:start -->` and `<!-- pyxel-tracked-files:end -->`.

The table is a **drift-detection filter**: only paths listed here are diffed between the previous and current upstream tags. It is no longer used to drive a sparse-checkout — `../pyxel/` is now a full checkout owned by `pyxel-ref-update`.

```bash
# All tracked upstream paths (used to filter the tag-to-tag diff).
mapfile -t TRACKED_FILES < <(awk '/pyxel-tracked-files:start/,/pyxel-tracked-files:end/' docs/pyxel-reference.md \
    | awk -F'|' 'NF>=6 && $3 !~ /^[ -]*$/ && $3 !~ /Upstream side/ {gsub(/^ +| +$/,"",$3); print $3}')

# Files for which to surface diffs (rows whose "Show diff" column is marked with ○).
mapfile -t TRACKED_DIFF_FILES < <(awk '/pyxel-tracked-files:start/,/pyxel-tracked-files:end/' docs/pyxel-reference.md \
    | awk -F'|' 'NF>=6 && $3 !~ /^[ -]*$/ && $3 !~ /Upstream side/ {gsub(/^ +| +$/,"",$3); gsub(/^ +| +$/,"",$5); if ($5 == "○") print $3}')

if [ ${#TRACKED_FILES[@]} -eq 0 ]; then
    echo "ERROR: failed to extract the tracked-files table from docs/pyxel-reference.md. Check that the markers <!-- pyxel-tracked-files:start/end --> are intact." >&2
    exit 1
fi
```

## Behavior on Launch

### 1. Refresh the reference checkout via `pyxel-ref-update`

Invoke the `pyxel-ref-update` skill via the Skill tool (no arguments). It clones / fetches `../pyxel/` and force-checks out the latest stable release tag of `kitao/pyxel`. Capture its single-line output, which has one of two forms:

```
synced ../pyxel to vX.Y.Z (YYYY-MM-DD)
already at vX.Y.Z
```

Extract the tag name from that line; it becomes `CURRENT_TAG` for the rest of this run. The sync date (`YYYY-MM-DD`) is taken from the same line when present, or otherwise resolved from the tag commit (`git -C ../pyxel log -1 --format=%cs "refs/tags/${CURRENT_TAG}"`).

The reference directory is treated as an artifact, so this skill never inspects local modifications there — `pyxel-ref-update` already discards them.

### 2. Detect drift between the tracked tag and the current tag

Read the previously-recorded `Tracked tag` from the `<!-- pyxel-upstream-sync -->` block of `docs/pyxel-reference.md`, and diff it against `CURRENT_TAG` over the tracked-files filter.

```bash
TRACKED_TAG=$(grep -oP 'Tracked tag\*\*: `\K[^`]+' docs/pyxel-reference.md)

if [ "$TRACKED_TAG" = "$CURRENT_TAG" ]; then
    echo "No upstream changes (Tracked tag = $TRACKED_TAG)."
    # Continue to the year-notation check anyway.
else
    git -C ../pyxel log --oneline "refs/tags/${TRACKED_TAG}..refs/tags/${CURRENT_TAG}" -- "${TRACKED_FILES[@]}"
fi
```

If `TRACKED_TAG` is the placeholder `(to be filled by the next /pyxel-ref-update)`, treat the current run as the initial population: skip the diff loop and ask the user to confirm before recording `CURRENT_TAG` as the new `Tracked tag`.

### 3. Surface a diff per changed file

If there are changes, present the diff for each "show-diff" file to the user and ask whether to incorporate it on the Pyxift side.

```bash
for f in "${TRACKED_DIFF_FILES[@]}"; do
    git -C ../pyxel diff "refs/tags/${TRACKED_TAG}..refs/tags/${CURRENT_TAG}" -- "$f"
done
```

The decision is made by a human:

- **Incorporate**: Update the corresponding Pyxift files (`Sources/...`, `docs/pyxel-reference.md`, etc.). Use commit messages of the form `chore(pyxel-sync): ...`.
- **Skip**: Record the rationale in `docs/decisions.md` or `docs/status.md`.

Once decisions are made, create a single dedicated commit that updates the `Tracked tag` and `Last synced` fields in `docs/pyxel-reference.md`:

```
chore(pyxel-sync): bump Tracked tag to <new-tag>
```

### 4. Auto-sync copyright-year notation

Copyright-year synchronization is delegated to the `copyright-sync` skill. Invoke `copyright-sync` with no arguments via the Skill tool to bring the upstream (Takashi Kitao) and Pyxift (atsuki.seo) year ranges into line with their respective SSOTs. For the detailed spec (target files, commit granularity, detection logic) see `.claude/skills/copyright-sync/SKILL.md`.

## Out of Scope (What This Skill Does NOT Do)

- Cloning, fetching, or checking out `../pyxel/`. That is the `pyxel-ref-update` skill's job.
- Tracking `main` or any moving branch. Drift is only computed between release tags.
- Bidirectional checks that the Pyxift implementation actually conforms to the SSOT (e.g. extracting API signatures).
  - Deferred until after the v0.1.0 tag. See "Future considerations" in `docs/status.md`.
- Syncing the full MIT license body.
  - The MIT license is versionless and historical grants do not change, so no follow-up is needed after the initial copy.
- Upstream files not listed in the tracked-files table.
  - Anything not in the table is excluded from diff surfacing. To start tracking it, add a row to the table.

## Related Files

- SSOT ledger: `docs/pyxel-reference.md`
- Reference-checkout sync primitive: `.claude/skills/pyxel-ref-update/SKILL.md`
- Copyright-year sync (delegated): `.claude/skills/copyright-sync/SKILL.md`
- Acknowledgments: `ACKNOWLEDGMENTS.md`
- Full license text: `THIRD_PARTY_LICENSES/pyxel-MIT.txt`
- Pre-release-tag checklist: `docs/status.md`
- Attribution-comment hook: `.claude/hooks/check-source-comment.sh`
- Pre-tag check hook: `.claude/hooks/check-release-tag.sh`
