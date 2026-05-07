#!/usr/bin/env bash
# PreToolUse hook: Bash ツールで `git tag v*.*.*` 形式のリリースタグを打とうとしたら
# `docs/status.md` の「リリースタグ前チェックリスト」が満たされているか機械的に検査する。
#
# 検査項目:
#   1. docs/pyxel-reference.md の Tracked SHA が ../pyxel の origin/main HEAD と一致
#   2. （swift build 警告ゼロは外部状態のため hook では検査しない。/pyxel-sync 完了は SHA 一致で代替）
#
# 動作:
#   - 検査失敗 → exit 2 で操作ブロック
#   - 検査成功またはタグコマンドではない → exit 0

set -u

INPUT=$(cat)
TOOL_NAME=$(printf '%s' "$INPUT" | jq -r '.tool_name // empty')
[ "$TOOL_NAME" = "Bash" ] || exit 0

COMMAND=$(printf '%s' "$INPUT" | jq -r '.tool_input.command // ""')

# 正式リリースタグ作成パターン: `git tag v0.1.0`, `git tag -a v1.2.3 ...` など
# 中間タグ（M1: ... のようなコミットメッセージ）や `git tag -d`（削除）は対象外
if ! printf '%s' "$COMMAND" | grep -qE '\bgit[[:space:]]+tag\b([^|;&]*[[:space:]])?v[0-9]+\.[0-9]+\.[0-9]+'; then
  exit 0
fi

# 削除系オプションは対象外
if printf '%s' "$COMMAND" | grep -qE '\bgit[[:space:]]+tag\b[^|;&]*-d\b'; then
  exit 0
fi

PROJECT_DIR="${CLAUDE_PROJECT_DIR:-$(pwd)}"
ERRORS=()

# 1. ../pyxel/ の存在確認
PYXEL_DIR="$PROJECT_DIR/../pyxel"
if [ ! -d "$PYXEL_DIR/.git" ]; then
  ERRORS+=("../pyxel/ が存在しません。先に /pyxel-sync を実行してください。")
else
  # 2. Tracked SHA と origin/main HEAD の一致確認
  TRACKED_SHA=$(grep -oP 'Tracked SHA\*\*: `\K[a-f0-9]+' "$PROJECT_DIR/docs/pyxel-reference.md" 2>/dev/null || echo "")
  if [ -z "$TRACKED_SHA" ]; then
    ERRORS+=("docs/pyxel-reference.md から Tracked SHA を読み取れません。")
  else
    git -C "$PYXEL_DIR" fetch origin --quiet 2>/dev/null || true
    UPSTREAM_SHA=$(git -C "$PYXEL_DIR" rev-parse origin/main 2>/dev/null || echo "")
    if [ -z "$UPSTREAM_SHA" ]; then
      ERRORS+=("../pyxel の origin/main を解決できません。")
    elif [ "$TRACKED_SHA" != "$UPSTREAM_SHA" ]; then
      ERRORS+=("Tracked SHA ($TRACKED_SHA) が ../pyxel の origin/main ($UPSTREAM_SHA) と一致しません。/pyxel-sync を実行してください。")
    fi
  fi
fi

if [ ${#ERRORS[@]} -gt 0 ]; then
  cat >&2 <<EOF
[Pyxift hook] リリースタグ前チェックリストが満たされていません。

未達項目:
EOF
  for e in "${ERRORS[@]}"; do
    printf '  - %s\n' "$e" >&2
  done
  cat >&2 <<'EOF'

詳細は docs/status.md「リリースタグ前チェックリスト」を参照してください。
swift build -Xswiftc -warnings-as-errors の警告ゼロ確認は、人間が手動で済ませている前提です。
EOF
  exit 2
fi

exit 0
