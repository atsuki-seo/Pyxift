#!/usr/bin/env bash
# PreToolUse hook: two-stage check on Edit/Write/MultiEdit.
#   Stage 1: if the file contains an upstream-derived symbol
#            (FONT_DATA / DEFAULT_COLORS / DEFAULT_TONE_*), an attribution
#            comment is required.
#   Stage 2: if the file carries an attribution comment, the upstream path it
#            cites must already be registered in the tracked-files table of
#            docs/pyxel-reference.md.
#
# The two stages are independent. A violation of either causes exit 2 (block).

set -u

source "$(dirname "$0")/lib/hook-common.sh"

parse_hook_input

[ -z "$FILE_PATH" ] && exit 0

# Limit to C++ / Swift source and headers. Documentation files (Markdown, etc.)
# are excluded because merely mentioning a symbol name or the literal "Source:"
# in prose would trigger false positives.
case "$FILE_PATH" in
  *.cpp|*.hpp|*.cc|*.cxx|*.h|*.swift) ;;
  *) exit 0 ;;
esac

extract_full_content_after_edit

HEAD_PORTION=$(printf '%s' "$NEW_CONTENT" | head -20)

# --- Stage 1: derived symbols imply an attribution comment is required. ---
if printf '%s' "$NEW_CONTENT" | grep -qE '(FONT_DATA|DEFAULT_COLORS|DEFAULT_TONE_)'; then
  if ! printf '%s' "$HEAD_PORTION" | grep -q 'Source: kitao/pyxel'; then
    cat >&2 <<'EOF'
[Pyxift hook] This file contains an upstream-derived Pyxel symbol
(FONT_DATA / DEFAULT_COLORS / DEFAULT_TONE_*), but no attribution comment
appears in the first 20 lines.

Please add the following at the top of the file:

  // Source: kitao/pyxel crates/pyxel-core/src/settings.rs
  // License: MIT (Copyright (c) 2018-2026 Takashi Kitao)

(MIT license attribution requirement.)
EOF
    exit 2
  fi
fi

# --- Stage 2: attribution comment present implies the path must be in the table. ---
SOURCE_LINE=$(printf '%s' "$HEAD_PORTION" | grep -m1 'Source: kitao/pyxel' || true)
if [ -n "$SOURCE_LINE" ]; then
  # Extract just the path portion (up to the first whitespace) from
  # `Source: kitao/pyxel <path> [(note)]`.
  UPSTREAM_PATH=$(printf '%s' "$SOURCE_LINE" | sed -nE 's|.*Source: kitao/pyxel +([^ ]+).*|\1|p')

  PROJECT_DIR="${CLAUDE_PROJECT_DIR:-$(pwd)}"
  REFERENCE_MD="$PROJECT_DIR/docs/pyxel-reference.md"

  if [ ! -f "$REFERENCE_MD" ]; then
    echo "[Pyxift hook] $REFERENCE_MD not found." >&2
    exit 2
  fi

  mapfile -t TRACKED_UPSTREAM < <(awk '/pyxel-tracked-files:start/,/pyxel-tracked-files:end/' "$REFERENCE_MD" \
      | awk -F'|' 'NF>=6 && $3 !~ /^[ -]*$/ && $3 !~ /Upstream side/ {gsub(/^ +| +$/,"",$3); print $3}')

  if [ ${#TRACKED_UPSTREAM[@]} -eq 0 ]; then
    cat >&2 <<EOF
[Pyxift hook] Could not extract the tracked-files table from
docs/pyxel-reference.md. Check the markers
<!-- pyxel-tracked-files:start --> ... <!-- pyxel-tracked-files:end --> and
the table format.
EOF
    exit 2
  fi

  if [ -z "$UPSTREAM_PATH" ] || ! printf '%s\n' "${TRACKED_UPSTREAM[@]}" | grep -Fxq "$UPSTREAM_PATH"; then
    cat >&2 <<EOF
[Pyxift hook] The upstream path "$UPSTREAM_PATH" cited in the attribution
comment is not registered in the tracked-files table of
docs/pyxel-reference.md.

Add a row inside the markers <!-- pyxel-tracked-files:start/end -->:
  | $FILE_PATH | $UPSTREAM_PATH | <purpose> | ○ | implemented |

Allowed values for the Status column are documented in the
"tracked-files table" section of reference.md.
EOF
    exit 2
  fi
fi

exit 0
