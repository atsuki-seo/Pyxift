#!/usr/bin/env bash
# PreToolUse hook: warns when C++ code under the core layer
# (Sources/CPyxiftCore/src/core/) tries to import SDL3.
#
# Layer policy:
#   - core/    : pure C++ logic layer (SDL3 imports forbidden)
#   - platform/: SDL3 adapter layer (SDL3 imports allowed)
#
# Behavior:
#   - If a file under core/ ends up containing #include <SDL3/...> after the edit, exit 2.
#   - Otherwise, exit 0.

set -u

source "$(dirname "$0")/lib/hook-common.sh"

parse_hook_input

[ -z "$FILE_PATH" ] && exit 0

# Only consider C++ source/header files.
case "$FILE_PATH" in
  *.cpp|*.hpp|*.cc|*.cxx|*.h) ;;
  *) exit 0 ;;
esac

# Only consider paths inside the core layer (the platform layer is allowed to use SDL).
case "$FILE_PATH" in
  */CPyxiftCore/src/core/*) ;;
  *) exit 0 ;;
esac

extract_new_string_only

if printf '%s' "$NEW_CONTENT" | grep -qE '#\s*include\s*[<"]SDL3'; then
  cat >&2 <<EOF
[Pyxift hook] You are trying to include an SDL3 header in the C++ core layer
(Sources/CPyxiftCore/src/core/). Pyxift's design keeps the core layer in
pure C++ and confines all SDL3 dependencies to the platform/ adapter layer.

File: $FILE_PATH

Options:
  1. Move the SDL3-using logic under Sources/CPyxiftCore/src/platform/.
  2. Have the core layer accept an intermediate type (e.g. VirtualEvent), and
     translate from SDL3 in the adapter layer.

See "C++ Core Layer: No SDL3 Imports" in CLAUDE.md for details.
EOF
  exit 2
fi

exit 0
