#!/usr/bin/env bash
# PreToolUse hook: C++ コア層 (Sources/CPyxiftCore/src/core/) に SDL3 を import しようとしたら警告。
#
# レイヤー方針:
#   - core/    : pure C++ ロジック層（SDL3 import 禁止）
#   - platform/: SDL3 アダプタ層（SDL3 import OK）
#
# 動作:
#   - core/ 配下のファイルで #include <SDL3/...> が編集後に存在 → exit 2
#   - 問題なし → exit 0

set -u

source "$(dirname "$0")/lib/hook-common.sh"

parse_hook_input

[ -z "$FILE_PATH" ] && exit 0

# 対象は C++ ソース・ヘッダのみ
case "$FILE_PATH" in
  *.cpp|*.hpp|*.cc|*.cxx|*.h) ;;
  *) exit 0 ;;
esac

# core 層のパスのみ対象（platform 層は SDL OK）
case "$FILE_PATH" in
  */CPyxiftCore/src/core/*) ;;
  *) exit 0 ;;
esac

extract_new_string_only

if printf '%s' "$NEW_CONTENT" | grep -qE '#\s*include\s*[<"]SDL3'; then
  cat >&2 <<EOF
[Pyxift hook] C++ コア層 (Sources/CPyxiftCore/src/core/) に SDL3 ヘッダを
include しようとしています。コア層は pure C++ で保ち、SDL3 依存は
platform/ アダプタ層に閉じ込めるのが Pyxift の設計方針です。

該当ファイル: $FILE_PATH

選択肢:
  1. SDL3 を使うロジックを Sources/CPyxiftCore/src/platform/ 配下に移す
  2. core 層では VirtualEvent などの中間型で受け取り、SDL3 アダプタ層で変換する

詳細: CLAUDE.md の「C++ コア層の SDL3 import 禁止ルール」を参照
EOF
  exit 2
fi

exit 0
