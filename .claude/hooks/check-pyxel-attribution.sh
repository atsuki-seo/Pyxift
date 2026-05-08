#!/usr/bin/env bash
# PreToolUse hook: ensures the upstream Pyxel (kitao/pyxel) attribution at the top
# of docs/pyxel-reference.md is not removed by an edit. This mechanically enforces
# the MIT license's attribution requirement on the Markdown side as well.
#
# Behavior:
#   - If, after the edit, the string "kitao/pyxel" no longer appears in the first 20 lines
#     of docs/pyxel-reference.md, exit 2.
#   - Otherwise, exit 0.

set -u

source "$(dirname "$0")/lib/hook-common.sh"

parse_hook_input

[ -z "$FILE_PATH" ] && exit 0

case "$FILE_PATH" in
  */docs/pyxel-reference.md) ;;
  *) exit 0 ;;
esac

extract_full_content_after_edit

HEAD_PORTION=$(printf '%s' "$NEW_CONTENT" | head -20)
if ! printf '%s' "$HEAD_PORTION" | grep -q 'kitao/pyxel'; then
  cat >&2 <<'EOF'
[Pyxift hook] The upstream Pyxel (kitao/pyxel) attribution has been removed from
the first 20 lines of docs/pyxel-reference.md. To keep the MIT license's
attribution requirement satisfied, please retain a notice near the top such as:

  Source: [kitao/pyxel](https://github.com/kitao/pyxel) (MIT License, Copyright (c) 2018-2026 Takashi Kitao)

EOF
  exit 2
fi

exit 0
