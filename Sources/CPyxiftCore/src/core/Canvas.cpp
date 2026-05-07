#include "Canvas.hpp"

#include <algorithm>
#include <cstdlib>

namespace pyxift {

Canvas::Canvas(int32_t width, int32_t height)
    : width_(width),
      height_(height),
      pixels_(static_cast<size_t>(width) * height, 0),
      clip_x1_(0),
      clip_y1_(0),
      clip_x2_(width - 1),
      clip_y2_(height - 1) {
    for (int i = 0; i < 16; ++i) palette_[i] = static_cast<uint8_t>(i);
}

void Canvas::set_clip(int32_t x, int32_t y, int32_t w, int32_t h) {
    if (w <= 0 || h <= 0) {
        // 負/0 サイズはクリップ無効化扱い（何も描けない領域）にせず、本家同様に画面全体と交差させる。
        // 本家の挙動: 矩形と画面の交差を取る。空矩形は何も描けない状態にする。
        clip_x1_ = 0;
        clip_y1_ = 0;
        clip_x2_ = -1;
        clip_y2_ = -1;
        return;
    }
    int32_t x1 = x;
    int32_t y1 = y;
    int32_t x2 = x + w - 1;
    int32_t y2 = y + h - 1;
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= width_) x2 = width_ - 1;
    if (y2 >= height_) y2 = height_ - 1;
    clip_x1_ = x1;
    clip_y1_ = y1;
    clip_x2_ = x2;
    clip_y2_ = y2;
}

void Canvas::reset_clip() {
    clip_x1_ = 0;
    clip_y1_ = 0;
    clip_x2_ = width_ - 1;
    clip_y2_ = height_ - 1;
}

void Canvas::set_camera(int32_t x, int32_t y) {
    camera_x_ = x;
    camera_y_ = y;
}

void Canvas::reset_camera() {
    camera_x_ = 0;
    camera_y_ = 0;
}

void Canvas::set_pal(uint8_t from, uint8_t to) {
    palette_[from & 0x0f] = static_cast<uint8_t>(to & 0x0f);
}

void Canvas::reset_pal() {
    for (int i = 0; i < 16; ++i) palette_[i] = static_cast<uint8_t>(i);
}

void Canvas::put(int32_t x, int32_t y, uint8_t color) {
    if (x < clip_x1_ || x > clip_x2_ || y < clip_y1_ || y > clip_y2_) return;
    pixels_[static_cast<size_t>(y) * width_ + x] = palette_[color & 0x0f];
}

void Canvas::cls(uint8_t color) {
    // 本家準拠: cls はパレット適用後の色で全画面を塗る。clip/camera は無視（画面全体）。
    std::fill(pixels_.begin(), pixels_.end(), palette_[color & 0x0f]);
}

void Canvas::pset(int32_t x, int32_t y, uint8_t color) {
    put(x - camera_x_, y - camera_y_, color);
}

uint8_t Canvas::pget(int32_t x, int32_t y) const {
    // 本家準拠: pget は camera 適用、clip 範囲外は 0。フレームバッファに既に書かれた値を返す。
    const int32_t fx = x - camera_x_;
    const int32_t fy = y - camera_y_;
    if (fx < 0 || fy < 0 || fx >= width_ || fy >= height_) return 0;
    return pixels_[static_cast<size_t>(fy) * width_ + fx];
}

void Canvas::hline(int32_t x1, int32_t x2, int32_t y, uint8_t color) {
    if (y < clip_y1_ || y > clip_y2_) return;
    if (x1 > x2) std::swap(x1, x2);
    if (x2 < clip_x1_ || x1 > clip_x2_) return;
    if (x1 < clip_x1_) x1 = clip_x1_;
    if (x2 > clip_x2_) x2 = clip_x2_;
    const uint8_t c = palette_[color & 0x0f];
    auto *row = pixels_.data() + static_cast<size_t>(y) * width_;
    for (int32_t x = x1; x <= x2; ++x) {
        row[x] = c;
    }
}

void Canvas::line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t color) {
    // 整数 Bresenham（全8オクタント対応）。
    // 入力座標は API 座標。camera は put() ではなくここで適用する。
    x1 -= camera_x_; y1 -= camera_y_;
    x2 -= camera_x_; y2 -= camera_y_;
    int32_t dx = std::abs(x2 - x1);
    int32_t dy = -std::abs(y2 - y1);
    int32_t sx = x1 < x2 ? 1 : -1;
    int32_t sy = y1 < y2 ? 1 : -1;
    int32_t err = dx + dy;
    int32_t x = x1;
    int32_t y = y1;
    while (true) {
        put(x, y, color);
        if (x == x2 && y == y2) break;
        int32_t e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y += sy;
        }
    }
}

void Canvas::rect(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t color) {
    if (w <= 0 || h <= 0) return;
    x -= camera_x_;
    y -= camera_y_;
    int32_t x1 = x;
    int32_t y1 = y;
    int32_t x2 = x + w - 1;
    int32_t y2 = y + h - 1;
    if (x2 < clip_x1_ || y2 < clip_y1_ || x1 > clip_x2_ || y1 > clip_y2_) return;
    if (x1 < clip_x1_) x1 = clip_x1_;
    if (y1 < clip_y1_) y1 = clip_y1_;
    if (x2 > clip_x2_) x2 = clip_x2_;
    if (y2 > clip_y2_) y2 = clip_y2_;
    const uint8_t c = palette_[color & 0x0f];
    for (int32_t yy = y1; yy <= y2; ++yy) {
        auto *row = pixels_.data() + static_cast<size_t>(yy) * width_;
        for (int32_t xx = x1; xx <= x2; ++xx) {
            row[xx] = c;
        }
    }
}

void Canvas::rectb(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t color) {
    if (w <= 0 || h <= 0) return;
    x -= camera_x_;
    y -= camera_y_;
    const int32_t x2 = x + w - 1;
    const int32_t y2 = y + h - 1;
    hline(x, x2, y, color);
    hline(x, x2, y2, color);
    // 縦辺（既に上下2行分は描いたのでその間だけ）。
    for (int32_t yy = y + 1; yy <= y2 - 1; ++yy) {
        put(x, yy, color);
        put(x2, yy, color);
    }
}

void Canvas::circ(int32_t cx, int32_t cy, int32_t r, uint8_t color) {
    if (r < 0) return;
    cx -= camera_x_;
    cy -= camera_y_;
    if (r == 0) {
        put(cx, cy, color);
        return;
    }
    // midpoint circle、各 y についてスパンを hline で塗る。
    int32_t x = r;
    int32_t y = 0;
    int32_t err = 1 - r;
    while (x >= y) {
        hline(cx - x, cx + x, cy + y, color);
        hline(cx - x, cx + x, cy - y, color);
        hline(cx - y, cx + y, cy + x, color);
        hline(cx - y, cx + y, cy - x, color);
        ++y;
        if (err < 0) {
            err += 2 * y + 1;
        } else {
            --x;
            err += 2 * (y - x) + 1;
        }
    }
}

void Canvas::circb(int32_t cx, int32_t cy, int32_t r, uint8_t color) {
    if (r < 0) return;
    cx -= camera_x_;
    cy -= camera_y_;
    if (r == 0) {
        put(cx, cy, color);
        return;
    }
    int32_t x = r;
    int32_t y = 0;
    int32_t err = 1 - r;
    while (x >= y) {
        put(cx + x, cy + y, color);
        put(cx - x, cy + y, color);
        put(cx + x, cy - y, color);
        put(cx - x, cy - y, color);
        put(cx + y, cy + x, color);
        put(cx - y, cy + x, color);
        put(cx + y, cy - x, color);
        put(cx - y, cy - x, color);
        ++y;
        if (err < 0) {
            err += 2 * y + 1;
        } else {
            --x;
            err += 2 * (y - x) + 1;
        }
    }
}

void Canvas::trib(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint8_t color) {
    // line() が camera を内部適用するので、ここではそのまま渡す。
    line(x1, y1, x2, y2, color);
    line(x2, y2, x3, y3, color);
    line(x3, y3, x1, y1, color);
}

void Canvas::tri(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint8_t color) {
    // M2a: 塗りは未実装。線のみで暫定（M2c で scanline 塗りに置換）。
    trib(x1, y1, x2, y2, x3, y3, color);
}

} // namespace pyxift
