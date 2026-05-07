#!/usr/bin/env bash
# PreToolUse hook: Edit/Write/MultiEdit 時に、Pyxel 本家流用シンボルを含むファイルが
# 出典コメント（"Source: kitao/pyxel"）を持っているかチェック。
#
# 動作:
#   - 出典コメントが無い、または編集後に消えそうな場合 → exit 2 で警告表示・操作ブロック
#   - 問題なし → exit 0
#
# 検出対象シンボル: FONT_DATA / DEFAULT_COLORS / DEFAULT_TONE_

set -u

source "$(dirname "$0")/lib/hook-common.sh"

parse_hook_input

[ -z "$FILE_PATH" ] && exit 0

# 編集後のファイル内容を組み立てる（ツールごとに違う）
extract_full_content_after_edit

# 本家流用シンボルを含むか
if printf '%s' "$NEW_CONTENT" | grep -qE '(FONT_DATA|DEFAULT_COLORS|DEFAULT_TONE_)'; then
  # 出典コメントが冒頭近く（先頭20行以内）にあるか
  HEAD_PORTION=$(printf '%s' "$NEW_CONTENT" | head -20)
  if ! printf '%s' "$HEAD_PORTION" | grep -q 'Source: kitao/pyxel'; then
    cat >&2 <<'EOF'
[Pyxift hook] Pyxel 本家流用シンボル（FONT_DATA / DEFAULT_COLORS / DEFAULT_TONE_*）が
このファイルに含まれていますが、ファイル冒頭20行以内に出典コメントがありません。

以下のコメントをファイル先頭に追加してください:

  // Source: kitao/pyxel crates/pyxel-core/src/settings.rs
  // License: MIT (Copyright (c) 2018-2026 Takashi Kitao)

(MIT ライセンスの帰属表示義務)
EOF
    exit 2
  fi
fi

exit 0
