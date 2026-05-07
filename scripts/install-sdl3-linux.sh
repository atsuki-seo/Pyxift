#!/usr/bin/env bash
# SDL3 を Linux にインストールするスクリプト。
#
# 動作:
#   1. apt の libsdl3-dev を試す
#   2. 失敗したら SDL3 をソースから /usr/local 配下にビルド・インストール
#   3. 最後に `pkg-config --modversion sdl3` で確認
#
# 使い方:
#   bash scripts/install-sdl3-linux.sh
#
# sudo が必要なので途中でパスワードを聞かれます。

set -euo pipefail

SDL_BRANCH="${SDL_BRANCH:-release-3.2.x}"
SDL_SRC_DIR="${SDL_SRC_DIR:-/tmp/SDL}"

log() { printf '\033[1;34m[install-sdl3]\033[0m %s\n' "$*"; }
err() { printf '\033[1;31m[install-sdl3]\033[0m %s\n' "$*" >&2; }

if [[ "$(uname -s)" != "Linux" ]]; then
    err "このスクリプトは Linux 専用です。macOS は 'brew install sdl3' を使ってください。"
    exit 1
fi

if ! command -v sudo >/dev/null 2>&1; then
    err "sudo が必要です。"
    exit 1
fi

log "apt index を更新します..."
sudo apt-get update

log "apt の libsdl3-dev を試します..."
if sudo apt-get install -y libsdl3-dev pkg-config; then
    log "libsdl3-dev のインストールに成功しました。"
else
    log "libsdl3-dev が見つからないため、ソースビルドへフォールバックします。"

    log "ビルド依存をインストールします..."
    sudo apt-get install -y \
        build-essential cmake pkg-config git \
        libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev \
        libwayland-dev libxkbcommon-dev libgbm-dev libdrm-dev \
        libasound2-dev libpulse-dev

    if [[ -d "$SDL_SRC_DIR/.git" ]]; then
        log "既存ソース ($SDL_SRC_DIR) を更新します..."
        git -C "$SDL_SRC_DIR" fetch --depth 1 origin "$SDL_BRANCH"
        git -C "$SDL_SRC_DIR" checkout "$SDL_BRANCH"
        git -C "$SDL_SRC_DIR" reset --hard "origin/$SDL_BRANCH"
    else
        log "SDL3 ソースを取得します ($SDL_BRANCH)..."
        rm -rf "$SDL_SRC_DIR"
        git clone --depth 1 --branch "$SDL_BRANCH" https://github.com/libsdl-org/SDL.git "$SDL_SRC_DIR"
    fi

    log "SDL3 をビルドします..."
    cmake -S "$SDL_SRC_DIR" -B "$SDL_SRC_DIR/build" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$SDL_SRC_DIR/build" -j"$(nproc)"

    log "SDL3 をインストールします (/usr/local)..."
    sudo cmake --install "$SDL_SRC_DIR/build"
    sudo ldconfig
fi

log "確認: pkg-config --modversion sdl3"
if version=$(pkg-config --modversion sdl3 2>/dev/null); then
    log "SDL3 $version をインストールしました。"
else
    err "pkg-config から sdl3 が見えません。PKG_CONFIG_PATH を確認してください。"
    err "  例: export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:\$PKG_CONFIG_PATH"
    exit 1
fi

log "完了。'swift build' を実行できます。"
