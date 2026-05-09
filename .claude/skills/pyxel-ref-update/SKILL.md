---
name: pyxel-ref-update
description: Sync the local upstream Pyxel reference checkout (`../pyxel/`) to the latest stable release tag of kitao/pyxel. Use this whenever a workflow needs an up-to-date copy of upstream Pyxel — invoked directly via `/pyxel-ref-update`, and also called internally from `pyxel-sync`, `pyxel-research`, or any other skill that reads `../pyxel/`. Always run this before diffing, researching, or comparing against upstream so the working copy is not stale.
allowed-tools: Bash(git -C ../pyxel:*) Bash(git clone:*) Bash(git ls-remote:*) Bash(rm:*) Bash(test:*)
---

# pyxel-ref-update

Bring the parent-directory checkout of `../pyxel/` into sync with the latest **stable release tag** of [kitao/pyxel](https://github.com/kitao/pyxel). This is the shared "fetch upstream" primitive used by every Pyxift skill that reads upstream sources, so it must be cheap, idempotent, and safe to call repeatedly.

The reference repository is treated as an *artifact*, not a workspace: local edits there are not expected, and this skill discards them without asking. If you ever need to preserve work in `../pyxel/`, do not call this skill — operate manually instead.

## What this skill does

- Resolves the latest stable release tag of `kitao/pyxel` from the remote (no full history needed).
- If `../pyxel/.git` already exists, validates that `origin` points at `kitao/pyxel`, then fetches just that tag and force-checks-it-out.
- Otherwise, performs a shallow clone (`--depth=1 --branch <tag>`) into `../pyxel/`.
- Skips work if the working tree is already at the resolved tag.
- Reports the synced tag name and tag commit date so the caller can show it to the user.

## What this skill does NOT do

- It does **not** track `main` or any other moving branch. Always pin to a release tag.
- It does **not** do sparse-checkout. Earlier iterations sparse-checked-out only the tracked files, but downstream skills (e.g. `pyxel-research`) need to read arbitrary upstream paths, so we take the whole tree.
- It does **not** touch any Pyxift files. Drift detection, copyright sync, etc. are the responsibility of the calling skill.
- It does **not** prompt before discarding local changes in `../pyxel/`. The reference checkout is owned by these skills.

## Resolving the latest stable tag

`kitao/pyxel` uses simple semver tags (`vX.Y.Z`). Filter out any pre-release labels (`-rc`, `-beta`, `-alpha`, `-pre`, etc.) and pick the highest version with `sort -V`.

```bash
# List remote tags, strip refs/tags/ prefix and any peeled `^{}` suffix,
# drop pre-releases, and pick the largest semver-ish tag.
LATEST_TAG=$(git ls-remote --tags --refs https://github.com/kitao/pyxel.git \
    | awk '{print $2}' \
    | sed 's@^refs/tags/@@; s/\^{}$//' \
    | grep -E '^v?[0-9]+\.[0-9]+\.[0-9]+$' \
    | sort -V \
    | tail -n1)

if [ -z "$LATEST_TAG" ]; then
    echo "ERROR: failed to resolve a stable release tag from kitao/pyxel." >&2
    exit 1
fi
```

The `grep -E '^v?[0-9]+\.[0-9]+\.[0-9]+$'` anchors exclude any tag containing a hyphenated suffix, which is how upstream marks pre-releases. If upstream ever changes that convention, update this regex rather than working around it in callers.

## Sync flow

```bash
REPO_URL=https://github.com/kitao/pyxel.git

if [ -d ../pyxel/.git ]; then
    # Validate that this checkout actually belongs to kitao/pyxel.
    REMOTE_URL=$(git -C ../pyxel remote get-url origin 2>/dev/null || echo "")
    case "$REMOTE_URL" in
        *github.com[:/]kitao/pyxel*) ;;
        *)
            echo "ERROR: ../pyxel's origin is not kitao/pyxel: $REMOTE_URL" >&2
            exit 1
            ;;
    esac

    # Skip work if we are already on the resolved tag.
    CURRENT_SHA=$(git -C ../pyxel rev-parse HEAD 2>/dev/null || echo "")
    TARGET_SHA=$(git -C ../pyxel rev-parse --verify "refs/tags/${LATEST_TAG}^{commit}" 2>/dev/null || echo "")
    if [ -n "$TARGET_SHA" ] && [ "$CURRENT_SHA" = "$TARGET_SHA" ]; then
        echo "already at ${LATEST_TAG}"
    else
        # Fetch just the target tag (depth=1) and force-check it out.
        git -C ../pyxel fetch --depth=1 origin "refs/tags/${LATEST_TAG}:refs/tags/${LATEST_TAG}"
        git -C ../pyxel checkout --force "refs/tags/${LATEST_TAG}"
        # Drop anything stale (untracked files, leftover sparse-checkout state, etc.).
        git -C ../pyxel reset --hard "refs/tags/${LATEST_TAG}"
    fi
else
    # Fresh shallow clone pinned directly to the tag.
    git clone --depth=1 --branch "$LATEST_TAG" "$REPO_URL" ../pyxel
fi
```

If `../pyxel/` exists but does not contain a `.git` directory, treat it as a corrupted state: ask the user before deleting it. We do not auto-delete because the user might have placed unrelated files there.

## Reporting the result

After the sync succeeds, print the tag name and the tag commit date so the calling skill can echo it back to the user:

```bash
TAG_DATE=$(git -C ../pyxel log -1 --format=%cs "refs/tags/${LATEST_TAG}")
echo "synced ../pyxel to ${LATEST_TAG} (${TAG_DATE})"
```

Callers (e.g. `pyxel-sync`) can capture this line and surface it in their own summary. The format is intentionally a single line so it is easy to grep / display.

## Related Files

- Upstream policy & rationale: `CLAUDE.md` ("Referencing the Upstream Pyxel Repository")
- Drift detection over the synced tree: `.claude/skills/pyxel-sync/SKILL.md`
- Copyright-year sync (delegated by `pyxel-sync`): `.claude/skills/copyright-sync/SKILL.md`
- SSOT for tracked upstream paths: `docs/pyxel-reference.md`
