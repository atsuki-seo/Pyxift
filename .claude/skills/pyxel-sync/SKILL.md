---
name: pyxel-sync
description: Detect API and numeric-spec drift between upstream Pyxel (kitao/pyxel) and Pyxift's SSOT (`docs/pyxel-reference.md`), and assist in syncing them. Invoked when the user runs `/pyxel-sync`, or as part of the pre-release-tag checklist.
allowed-tools: Bash(git -C ../pyxel:*) Bash(git clone:*) Bash(grep:*)
---

# pyxel-sync

Pyxift is an independent implementation aiming for Pyxel API compatibility. This skill tracks the upstream repository (`../pyxel/`) and surfaces any changes to the tracked files for human review.

## Tracked Files (SSOT)

The "tracked-files table" in `docs/pyxel-reference.md` is the SSOT for what is tracked. This skill parses that table into bash arrays. The table is delimited by the markers `<!-- pyxel-tracked-files:start -->` and `<!-- pyxel-tracked-files:end -->`.

```bash
# All tracked upstream files (used for sparse-checkout and drift detection).
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

### 1. Prepare the reference repository

Bring `../pyxel/` (in the parent directory) up to the latest `main` of kitao/pyxel.

```bash
if [ -d ../pyxel/.git ]; then
    # Verify origin points at kitao/pyxel.
    REMOTE_URL=$(git -C ../pyxel remote get-url origin 2>/dev/null || echo "")
    case "$REMOTE_URL" in
        *github.com[:/]kitao/pyxel*) ;;
        *)
            echo "ERROR: ../pyxel's origin is not kitao/pyxel: $REMOTE_URL"
            exit 1
            ;;
    esac
    # Re-set sparse-checkout each time so updates to the table are reflected (idempotent).
    git -C ../pyxel sparse-checkout set "${TRACKED_FILES[@]}"
    git -C ../pyxel fetch origin
    git -C ../pyxel reset --hard origin/main
else
    # Fetch only the necessary paths (history metadata is preserved).
    git clone --filter=blob:none --no-checkout https://github.com/kitao/pyxel.git ../pyxel
    git -C ../pyxel sparse-checkout init --no-cone
    git -C ../pyxel sparse-checkout set "${TRACKED_FILES[@]}"
    git -C ../pyxel checkout main
fi
```

The reference directory is not expected to be edited locally, so the local-modifications check is skipped (it gets overwritten unconditionally).

### 2. Detect drift against the tracked SHA

Read the current Tracked SHA from the `<!-- pyxel-upstream-sync -->` block of `docs/pyxel-reference.md`, and check whether any tracked file has changed since.

```bash
TRACKED_SHA=$(grep -oP 'Tracked SHA\*\*: `\K[a-f0-9]+' docs/pyxel-reference.md)
CURRENT_SHA=$(git -C ../pyxel rev-parse HEAD)

if [ "$TRACKED_SHA" = "$CURRENT_SHA" ]; then
    echo "No upstream changes (Tracked SHA = $TRACKED_SHA)."
    # Continue to the year-notation check anyway.
else
    git -C ../pyxel log --oneline "$TRACKED_SHA..HEAD" -- "${TRACKED_FILES[@]}"
fi
```

### 3. Surface a diff per changed file

If there are changes, present the diff for each file to the user and ask whether to incorporate it on the Pyxift side.

```bash
for f in "${TRACKED_DIFF_FILES[@]}"; do
    git -C ../pyxel diff "$TRACKED_SHA..HEAD" -- "$f"
done
```

The decision is made by a human:

- **Incorporate**: Update the corresponding Pyxift files (`Sources/...`, `docs/pyxel-reference.md`, etc.). Use commit messages of the form `chore(pyxel-sync): ...`.
- **Skip**: Record the rationale in `docs/decisions.md` or `docs/status.md`.

Once decisions are made, create a single dedicated commit that updates the Tracked SHA and Last-synced fields in `docs/pyxel-reference.md`:

```
chore(pyxel-sync): bump Tracked SHA to <new-sha>
```

### 4. Auto-sync copyright-year notation

Copyright-year synchronization is delegated to the `copyright-sync` skill. Invoke `copyright-sync` with no arguments via the Skill tool to bring the upstream (Takashi Kitao) and Pyxift (atsuki.seo) year ranges into line with their respective SSOTs. For the detailed spec (target files, commit granularity, detection logic) see `.claude/skills/copyright-sync/SKILL.md`.

## Out of Scope (What This Skill Does NOT Do)

- Bidirectional checks that the Pyxift implementation actually conforms to the SSOT (e.g. extracting API signatures).
  - Deferred until after the v0.1.0 tag. See "Future considerations" in `docs/status.md`.
- Syncing the full MIT license body.
  - The MIT license is versionless and historical grants do not change, so no follow-up is needed after the initial copy.
- Upstream files not listed in the tracked-files table.
  - Anything not in the table is excluded from both sparse-checkout and diff surfacing. To start tracking it, add a row to the table.

## Related Files

- SSOT ledger: `docs/pyxel-reference.md`
- Acknowledgments: `ACKNOWLEDGMENTS.md`
- Full license text: `THIRD_PARTY_LICENSES/pyxel-MIT.txt`
- Pre-release-tag checklist: `docs/status.md`
- Attribution-comment hook: `.claude/hooks/check-source-comment.sh`
- Pre-tag check hook: `.claude/hooks/check-release-tag.sh`
