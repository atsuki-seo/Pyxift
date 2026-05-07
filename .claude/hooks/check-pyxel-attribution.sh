#!/usr/bin/env bash
# PreToolUse hook: docs/pyxel-reference.md 冒頭の Pyxel本家 (kitao/pyxel) 出典表記が
# 編集後に消えていないかをチェックする。MITライセンスの帰属表示義務を Markdown 側でも
# 機械的に担保する。
#
# 動作:
#   - docs/pyxel-reference.md の編集後に冒頭20行から "kitao/pyxel" 文字列が消える → exit 2
#   - 問題なし → exit 0

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
[Pyxift hook] docs/pyxel-reference.md の冒頭20行から Pyxel本家 (kitao/pyxel) への
出典表記が消えています。MITライセンスの帰属表示義務を満たすため、冒頭付近に
以下のような表記を残してください:

  出典: [kitao/pyxel](https://github.com/kitao/pyxel) （MIT License, Copyright (c) 2018-2026 Takashi Kitao）

EOF
  exit 2
fi

exit 0
