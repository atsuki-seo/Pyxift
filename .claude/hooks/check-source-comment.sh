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

INPUT=$(cat)
TOOL_NAME=$(printf '%s' "$INPUT" | jq -r '.tool_name // empty')
FILE_PATH=$(printf '%s' "$INPUT" | jq -r '.tool_input.file_path // empty')

[ -z "$FILE_PATH" ] && exit 0

# 編集後のファイル内容を組み立てる（ツールごとに違う）
case "$TOOL_NAME" in
  Write)
    NEW_CONTENT=$(printf '%s' "$INPUT" | jq -r '.tool_input.content // ""')
    ;;
  Edit)
    NEW_STRING=$(printf '%s' "$INPUT" | jq -r '.tool_input.new_string // ""')
    # 既存ファイル + 新規挿入分を結合（簡易チェック用）
    if [ -f "$FILE_PATH" ]; then
      NEW_CONTENT="$(cat "$FILE_PATH")
$NEW_STRING"
    else
      NEW_CONTENT="$NEW_STRING"
    fi
    ;;
  MultiEdit)
    EDITS=$(printf '%s' "$INPUT" | jq -r '[.tool_input.edits[]?.new_string] | join("\n")')
    if [ -f "$FILE_PATH" ]; then
      NEW_CONTENT="$(cat "$FILE_PATH")
$EDITS"
    else
      NEW_CONTENT="$EDITS"
    fi
    ;;
  *)
    exit 0
    ;;
esac

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
