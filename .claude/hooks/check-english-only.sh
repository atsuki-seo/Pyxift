#!/usr/bin/env bash
# Hook: enforce the repository-wide English-only policy declared in
# .claude/rules/language-policy.md.
#
# Modes (auto-detected from CLAUDE_HOOK_EVENT or hook payload):
#   PreToolUse(Edit|Write|MultiEdit): scan the new content of the file being
#     edited for CJK characters and warn before the change lands.
#   Stop: scan all working-tree files that differ from HEAD (modified or
#     untracked) for CJK characters and warn before the user wraps up.
#
# This hook NEVER blocks (always exits 0). It only emits warnings to stderr
# so the human or the assistant can decide whether to act on them.

set -u

PROJECT_DIR="${CLAUDE_PROJECT_DIR:-$(pwd)}"

# CJK ranges: Hiragana (3040-309F), Katakana (30A0-30FF, half-width FF65-FF9F),
# CJK Unified Ideographs (4E00-9FFF), and CJK extensions used in practice.
CJK_REGEX=$'[\xe3\x80-\xe9\xbf][\x80-\xbf][\x80-\xbf]|[\xef\xbd\xa5-\xef\xbd\xbf]|[\xef\xbe\x80-\xef\xbe\x9f]'

# Paths that are exempt from the policy (third-party content, generated
# artifacts, ephemeral scratch space).
is_exempt_path() {
  case "$1" in
    */THIRD_PARTY_LICENSES/*) return 0 ;;
    */Sources/CPyxiftCore/vendor/*) return 0 ;;
    */.build/*) return 0 ;;
    */.git/*) return 0 ;;
    */.claude/tmp/*) return 0 ;;
    */node_modules/*) return 0 ;;
  esac
  return 1
}

# Only scan files whose extensions are within the documented scope of the
# policy. Binary or unrelated formats are ignored.
is_in_scope() {
  case "$1" in
    *.md|*.swift|*.cpp|*.hpp|*.cc|*.cxx|*.h|*.c|*.sh|*.bash|*.zsh) return 0 ;;
    *.toml|*.yml|*.yaml|*.json) return 0 ;;
    *.txt) return 0 ;;
    */Package.swift|*/Makefile|*/Dockerfile) return 0 ;;
  esac
  return 1
}

scan_text_for_cjk() {
  # $1 = label, $2 = text content. Prints warning lines (path:line) to stderr.
  local label="$1" text="$2"
  printf '%s' "$text" | grep -nP '\p{Han}|\p{Hiragana}|\p{Katakana}' \
    | sed "s|^|  ${label}:|" >&2
}

warn_header() {
  cat >&2 <<'EOF'
[Pyxift hook] Non-English (CJK) text detected — see .claude/rules/language-policy.md.
The repository policy requires all tracked content to be written in English.
This is a warning only; the operation will proceed.
EOF
}

# --- PreToolUse path ----------------------------------------------------------

run_pretooluse() {
  local input tool_name file_path content
  input=$(cat)
  tool_name=$(printf '%s' "$input" | jq -r '.tool_name // empty')
  file_path=$(printf '%s' "$input" | jq -r '.tool_input.file_path // empty')

  [ -z "$file_path" ] && return 0
  is_exempt_path "$file_path" && return 0
  is_in_scope "$file_path" || return 0

  case "$tool_name" in
    Write)
      content=$(printf '%s' "$input" | jq -r '.tool_input.content // ""')
      ;;
    Edit)
      content=$(printf '%s' "$input" | jq -r '.tool_input.new_string // ""')
      ;;
    MultiEdit)
      content=$(printf '%s' "$input" | jq -r '[.tool_input.edits[]?.new_string] | join("\n")')
      ;;
    *)
      return 0
      ;;
  esac

  if printf '%s' "$content" | grep -qP '\p{Han}|\p{Hiragana}|\p{Katakana}'; then
    warn_header
    printf '  file: %s\n' "$file_path" >&2
    scan_text_for_cjk "$file_path" "$content"
  fi
}

# --- Stop path ----------------------------------------------------------------

run_stop() {
  cd "$PROJECT_DIR" 2>/dev/null || return 0
  command -v git >/dev/null 2>&1 || return 0
  git rev-parse --is-inside-work-tree >/dev/null 2>&1 || return 0

  local changed
  changed=$(git status --porcelain 2>/dev/null \
    | awk '{ sub(/^...| -> /, "", $0); print }' \
    | sort -u)

  [ -z "$changed" ] && return 0

  local found=0
  while IFS= read -r f; do
    [ -z "$f" ] && continue
    [ ! -f "$f" ] && continue
    is_exempt_path "$f" && continue
    is_in_scope "$f" || continue
    if grep -lP '\p{Han}|\p{Hiragana}|\p{Katakana}' "$f" >/dev/null 2>&1; then
      if [ "$found" -eq 0 ]; then
        warn_header
        found=1
      fi
      grep -nP '\p{Han}|\p{Hiragana}|\p{Katakana}' "$f" 2>/dev/null \
        | sed "s|^|  $f:|" >&2
    fi
  done <<< "$changed"
}

# --- dispatch -----------------------------------------------------------------

# The hook event is provided either via CLAUDE_HOOK_EVENT or implicitly by
# settings.json (Stop hooks receive no stdin payload of interest).
EVENT="${CLAUDE_HOOK_EVENT:-}"
if [ -z "$EVENT" ]; then
  if [ -t 0 ]; then
    EVENT="Stop"
  else
    # Peek at stdin: if it parses as a tool-call payload, treat as PreToolUse.
    PAYLOAD=$(cat)
    if printf '%s' "$PAYLOAD" | jq -e '.tool_name' >/dev/null 2>&1; then
      EVENT="PreToolUse"
      printf '%s' "$PAYLOAD" | run_pretooluse
      exit 0
    else
      EVENT="Stop"
    fi
  fi
fi

case "$EVENT" in
  PreToolUse) run_pretooluse ;;
  Stop) run_stop ;;
  *) ;;
esac

exit 0
