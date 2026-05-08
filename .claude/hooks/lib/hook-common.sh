#!/usr/bin/env bash
# Shared helpers: parse the PreToolUse hook input JSON and assemble the post-edit content.
#
# Usage:
#   source "$(dirname "$0")/lib/hook-common.sh"
#   parse_hook_input              # Sets INPUT / TOOL_NAME / FILE_PATH
#   extract_new_string_only       # Sets NEW_CONTENT to just the newly inserted text
#   extract_full_content_after_edit  # Sets NEW_CONTENT to "existing file + newly inserted text"

# Read the hook payload from stdin and export INPUT / TOOL_NAME / FILE_PATH.
parse_hook_input() {
  INPUT=$(cat)
  TOOL_NAME=$(printf '%s' "$INPUT" | jq -r '.tool_name // empty')
  FILE_PATH=$(printf '%s' "$INPUT" | jq -r '.tool_input.file_path // empty')
}

# Place "only the newly inserted text" from Write/Edit/MultiEdit into NEW_CONTENT.
# For unsupported tool kinds, exit 0.
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

# For Write, place the new content into NEW_CONTENT. For Edit/MultiEdit, place
# "the existing file contents + the newly inserted text" into NEW_CONTENT.
# Useful for whole-file checks such as confirming an attribution comment is at the top.
# For unsupported tool kinds, exit 0.
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
