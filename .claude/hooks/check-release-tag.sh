#!/usr/bin/env bash
# PreToolUse hook: when the Bash tool is used to create a release tag of the form
# `git tag v*.*.*`, mechanically verify that the "pre-release-tag checklist" in
# docs/status.md is satisfied.
#
# Checks:
#   1. The Tracked SHA in docs/pyxel-reference.md matches origin/main HEAD of ../pyxel.
#   2. Every row of the tracked-files table has a valid Status value
#      (implemented / reserved:vX.Y.Z / pending / declined).
#   3. No row still has a `reserved:<tag>` matching the tag being created
#      (it should already have transitioned to `implemented`).
#   4. Any row marked `declined` has a reference link to docs/decisions.md or
#      docs/status.md in its Pyxift-side column.
#   (Zero swift-build warnings is external state, so the hook does not check it.
#    /pyxel-sync completion is approximated by the SHA-match check.)
#
# Behavior:
#   - On check failure: exit 2 to block the operation.
#   - On success or for non-tag commands: exit 0.

set -u

INPUT=$(cat)
TOOL_NAME=$(printf '%s' "$INPUT" | jq -r '.tool_name // empty')
[ "$TOOL_NAME" = "Bash" ] || exit 0

COMMAND=$(printf '%s' "$INPUT" | jq -r '.tool_input.command // ""')

# Pattern for an official release-tag creation command: `git tag v0.1.0`,
# `git tag -a v1.2.3 ...`, etc. Intermediate tags (commit messages like "M1: ...")
# and `git tag -d` (deletions) are out of scope.
if ! printf '%s' "$COMMAND" | grep -qE '\bgit[[:space:]]+tag\b([^|;&]*[[:space:]])?v[0-9]+\.[0-9]+\.[0-9]+'; then
  exit 0
fi

# Ignore deletion options.
if printf '%s' "$COMMAND" | grep -qE '\bgit[[:space:]]+tag\b[^|;&]*-d\b'; then
  exit 0
fi

# Extract the tag name being created (e.g. "v0.1.0").
RELEASE_TAG=$(printf '%s' "$COMMAND" | grep -oE 'v[0-9]+\.[0-9]+\.[0-9]+' | head -1)

PROJECT_DIR="${CLAUDE_PROJECT_DIR:-$(pwd)}"
ERRORS=()

# 1. Confirm ../pyxel/ exists.
PYXEL_DIR="$PROJECT_DIR/../pyxel"
if [ ! -d "$PYXEL_DIR/.git" ]; then
  ERRORS+=("../pyxel/ does not exist. Run /pyxel-sync first.")
else
  # 2. Verify Tracked SHA matches origin/main HEAD.
  TRACKED_SHA=$(grep -oP 'Tracked SHA\*\*: `\K[a-f0-9]+' "$PROJECT_DIR/docs/pyxel-reference.md" 2>/dev/null || echo "")
  if [ -z "$TRACKED_SHA" ]; then
    ERRORS+=("Could not read the Tracked SHA from docs/pyxel-reference.md.")
  else
    git -C "$PYXEL_DIR" fetch origin --quiet 2>/dev/null || true
    UPSTREAM_SHA=$(git -C "$PYXEL_DIR" rev-parse origin/main 2>/dev/null || echo "")
    if [ -z "$UPSTREAM_SHA" ]; then
      ERRORS+=("Could not resolve origin/main in ../pyxel.")
    elif [ "$TRACKED_SHA" != "$UPSTREAM_SHA" ]; then
      ERRORS+=("Tracked SHA ($TRACKED_SHA) does not match origin/main of ../pyxel ($UPSTREAM_SHA). Run /pyxel-sync.")
    fi
  fi
fi

# 3. Inspect the Status column of the tracked-files table.
REFERENCE_MD="$PROJECT_DIR/docs/pyxel-reference.md"
if [ ! -f "$REFERENCE_MD" ]; then
  ERRORS+=("$REFERENCE_MD not found.")
else
  TABLE_BODY=$(awk '/pyxel-tracked-files:start/,/pyxel-tracked-files:end/' "$REFERENCE_MD" \
      | awk -F'|' 'NF>=6 && $3 !~ /^[ -]*$/ && $3 !~ /Upstream side/ {print}')

  if [ -z "$TABLE_BODY" ]; then
    ERRORS+=("Could not extract the tracked-files table from docs/pyxel-reference.md (check the markers).")
  else
    while IFS= read -r ROW; do
      [ -z "$ROW" ] && continue
      PYXIFT_COL=$(printf '%s' "$ROW" | awk -F'|' '{gsub(/^ +| +$/,"",$2); print $2}')
      UPSTREAM_COL=$(printf '%s' "$ROW" | awk -F'|' '{gsub(/^ +| +$/,"",$3); print $3}')
      STATUS_COL=$(printf '%s' "$ROW" | awk -F'|' '{gsub(/^ +| +$/,"",$6); print $6}')

      case "$STATUS_COL" in
        implemented|pending)
          ;;
        declined)
          if ! printf '%s' "$PYXIFT_COL" | grep -qE 'docs/(decisions|status)\.md'; then
            ERRORS+=("tracked-files table: $UPSTREAM_COL has Status \"declined\" but its Pyxift-side column has no reference link to docs/decisions.md or docs/status.md.")
          fi
          ;;
        reserved:v*.*.*)
          RESERVED_TAG="${STATUS_COL#reserved:}"
          if [ "$RESERVED_TAG" = "$RELEASE_TAG" ]; then
            ERRORS+=("tracked-files table: $UPSTREAM_COL is still in Status \"$STATUS_COL\". Update it to \"implemented\" before creating tag $RELEASE_TAG.")
          fi
          ;;
        *)
          ERRORS+=("tracked-files table: $UPSTREAM_COL has invalid Status \"$STATUS_COL\" (allowed: implemented / reserved:vX.Y.Z / pending / declined).")
          ;;
      esac
    done <<< "$TABLE_BODY"
  fi
fi

if [ ${#ERRORS[@]} -gt 0 ]; then
  cat >&2 <<EOF
[Pyxift hook] The pre-release-tag checklist is not satisfied.

Outstanding items:
EOF
  for e in "${ERRORS[@]}"; do
    printf '  - %s\n' "$e" >&2
  done
  cat >&2 <<'EOF'

See the "pre-release-tag checklist" in docs/status.md for details.
The zero-warning check for `swift build -Xswiftc -warnings-as-errors` is assumed
to have been performed manually by a human.
EOF
  exit 2
fi

exit 0
