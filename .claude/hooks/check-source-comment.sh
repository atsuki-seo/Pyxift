#!/usr/bin/env bash
# PreToolUse hook: Edit/Write/MultiEdit 時に2段階チェック。
#   段1: 本家流用シンボル（FONT_DATA / DEFAULT_COLORS / DEFAULT_TONE_*）を含むなら出典コメント必須
#   段2: 出典コメントを持つなら、その本家パスが docs/pyxel-reference.md の追跡対象テーブルに登録済み必須
#
# 段1・段2は独立。どちらか1つでも違反すれば exit 2 でブロック。

set -u

source "$(dirname "$0")/lib/hook-common.sh"

parse_hook_input

[ -z "$FILE_PATH" ] && exit 0

# 対象は C++ / Swift のソース・ヘッダのみ。Markdown 等の文書ファイルでは
# シンボル名や `Source:` 文字列を文中で言及するだけで誤検知するため除外。
case "$FILE_PATH" in
  *.cpp|*.hpp|*.cc|*.cxx|*.h|*.swift) ;;
  *) exit 0 ;;
esac

extract_full_content_after_edit

HEAD_PORTION=$(printf '%s' "$NEW_CONTENT" | head -20)

# --- 段1: 流用シンボル → 出典コメント必須 ---
if printf '%s' "$NEW_CONTENT" | grep -qE '(FONT_DATA|DEFAULT_COLORS|DEFAULT_TONE_)'; then
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

# --- 段2: 出典コメント有 → 表登録必須 ---
SOURCE_LINE=$(printf '%s' "$HEAD_PORTION" | grep -m1 'Source: kitao/pyxel' || true)
if [ -n "$SOURCE_LINE" ]; then
  # `Source: kitao/pyxel <path> [(注記)]` から path 部のみ（最初の空白前まで）抽出
  UPSTREAM_PATH=$(printf '%s' "$SOURCE_LINE" | sed -nE 's|.*Source: kitao/pyxel +([^ ]+).*|\1|p')

  PROJECT_DIR="${CLAUDE_PROJECT_DIR:-$(pwd)}"
  REFERENCE_MD="$PROJECT_DIR/docs/pyxel-reference.md"

  if [ ! -f "$REFERENCE_MD" ]; then
    echo "[Pyxift hook] $REFERENCE_MD が見つかりません。" >&2
    exit 2
  fi

  mapfile -t TRACKED_UPSTREAM < <(awk '/pyxel-tracked-files:start/,/pyxel-tracked-files:end/' "$REFERENCE_MD" \
      | awk -F'|' 'NF>=6 && $3 !~ /^[ -]*$/ && $3 !~ /本家側/ {gsub(/^ +| +$/,"",$3); print $3}')

  if [ ${#TRACKED_UPSTREAM[@]} -eq 0 ]; then
    cat >&2 <<EOF
[Pyxift hook] docs/pyxel-reference.md の追跡対象テーブルが抽出できません。
マーカー <!-- pyxel-tracked-files:start --> ～ <!-- pyxel-tracked-files:end --> と
表のフォーマットを確認してください。
EOF
    exit 2
  fi

  if [ -z "$UPSTREAM_PATH" ] || ! printf '%s\n' "${TRACKED_UPSTREAM[@]}" | grep -Fxq "$UPSTREAM_PATH"; then
    cat >&2 <<EOF
[Pyxift hook] 出典コメントの本家パス「$UPSTREAM_PATH」が
docs/pyxel-reference.md の追跡対象テーブルに登録されていません。

表に1行追加してください（マーカー <!-- pyxel-tracked-files:start/end --> 内）:
  | $FILE_PATH | $UPSTREAM_PATH | <用途> | ○ | 実装済み |

「状態」列の取り得る値は reference.md「追跡対象テーブル」節の説明を参照。
EOF
    exit 2
  fi
fi

exit 0
