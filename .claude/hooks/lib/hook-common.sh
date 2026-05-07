#!/usr/bin/env bash
# 共通ヘルパー: PreToolUse hook の入力 JSON 解析と編集後コンテンツ組み立て。
#
# 利用方法:
#   source "$(dirname "$0")/lib/hook-common.sh"
#   parse_hook_input              # INPUT / TOOL_NAME / FILE_PATH を設定
#   extract_new_string_only       # NEW_CONTENT に新規挿入分のみを格納
#   extract_full_content_after_edit  # NEW_CONTENT に「既存ファイル + 新規挿入分」を格納

# 標準入力から hook payload を読み、INPUT/TOOL_NAME/FILE_PATH をエクスポートする。
parse_hook_input() {
  INPUT=$(cat)
  TOOL_NAME=$(printf '%s' "$INPUT" | jq -r '.tool_name // empty')
  FILE_PATH=$(printf '%s' "$INPUT" | jq -r '.tool_input.file_path // empty')
}

# Write/Edit/MultiEdit の「新規挿入分のみ」を NEW_CONTENT に格納する。
# 未対応ツール種別の場合は exit 0 で抜ける。
extract_new_string_only() {
  case "$TOOL_NAME" in
    Write)
      NEW_CONTENT=$(printf '%s' "$INPUT" | jq -r '.tool_input.content // ""')
      ;;
    Edit)
      NEW_STRING=$(printf '%s' "$INPUT" | jq -r '.tool_input.new_string // ""')
      NEW_CONTENT="$NEW_STRING"
      ;;
    MultiEdit)
      NEW_CONTENT=$(printf '%s' "$INPUT" | jq -r '[.tool_input.edits[]?.new_string] | join("\n")')
      ;;
    *)
      exit 0
      ;;
  esac
}

# Write の場合は新規コンテンツ、Edit/MultiEdit の場合は「既存ファイル + 新規挿入分」を
# NEW_CONTENT に格納する。冒頭の出典コメント有無のような「ファイル全体に対する確認」用途。
# 未対応ツール種別の場合は exit 0 で抜ける。
extract_full_content_after_edit() {
  case "$TOOL_NAME" in
    Write)
      NEW_CONTENT=$(printf '%s' "$INPUT" | jq -r '.tool_input.content // ""')
      ;;
    Edit)
      NEW_STRING=$(printf '%s' "$INPUT" | jq -r '.tool_input.new_string // ""')
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
}
