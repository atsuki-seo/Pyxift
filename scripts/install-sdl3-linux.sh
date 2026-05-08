#!/usr/bin/env bash
# Install SDL3 on Linux.
#
# Behavior:
#   1. Try the libsdl3-dev package via apt.
#   2. On failure, build SDL3 from source and install under /usr/local.
#   3. Verify with `pkg-config --modversion sdl3`.
#
# Usage:
#   bash scripts/install-sdl3-linux.sh
#
# Requires sudo; you will be prompted for your password.

set -euo pipefail

SDL_BRANCH="${SDL_BRANCH:-release-3.2.x}"
SDL_SRC_DIR="${SDL_SRC_DIR:-/tmp/SDL}"

log() { printf '\033[1;34m[install-sdl3]\033[0m %s\n' "$*"; }
err() { printf '\033[1;31m[install-sdl3]\033[0m %s\n' "$*" >&2; }

if [[ "$(uname -s)" != "Linux" ]]; then
    err "This script is Linux-only. On macOS use 'brew install sdl3' instead."
    exit 1
fi

if ! command -v sudo >/dev/null 2>&1; then
    err "sudo is required."
    exit 1
fi

log "Updating apt index..."
sudo apt-get update

log "Trying libsdl3-dev via apt..."
if sudo apt-get install -y libsdl3-dev pkg-config; then
    log "libsdl3-dev installed successfully."
else
    log "libsdl3-dev not found; falling back to a source build."

    log "Installing build dependencies..."
    sudo apt-get install -y \
        build-essential cmake pkg-config git \
        libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev \
        libwayland-dev libxkbcommon-dev libgbm-dev libdrm-dev \
        libasound2-dev libpulse-dev

    if [[ -d "$SDL_SRC_DIR/.git" ]]; then
        log "Updating existing source tree ($SDL_SRC_DIR)..."
        git -C "$SDL_SRC_DIR" fetch --depth 1 origin "$SDL_BRANCH"
        git -C "$SDL_SRC_DIR" checkout "$SDL_BRANCH"
        git -C "$SDL_SRC_DIR" reset --hard "origin/$SDL_BRANCH"
    else
        log "Fetching SDL3 source ($SDL_BRANCH)..."
        rm -rf "$SDL_SRC_DIR"
        git clone --depth 1 --branch "$SDL_BRANCH" https://github.com/libsdl-org/SDL.git "$SDL_SRC_DIR"
    fi

    log "Building SDL3..."
    cmake -S "$SDL_SRC_DIR" -B "$SDL_SRC_DIR/build" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$SDL_SRC_DIR/build" -j"$(nproc)"

    log "Installing SDL3 to /usr/local..."
    sudo cmake --install "$SDL_SRC_DIR/build"
    sudo ldconfig
fi

log "Verifying with: pkg-config --modversion sdl3"
if version=$(pkg-config --modversion sdl3 2>/dev/null); then
    log "Installed SDL3 $version."
else
    err "pkg-config cannot find sdl3. Check your PKG_CONFIG_PATH."
    err "  e.g. export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:\$PKG_CONFIG_PATH"
    exit 1
fi

log "Done. You can now run 'swift build'."
