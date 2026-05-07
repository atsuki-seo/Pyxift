#include "Canvas.hpp"

#include <algorithm>
#include <cstdlib>

namespace pyxift {

Canvas::Canvas(int32_t width, int32_t height)
    : width_(width), height_(height), pixels_(static_cast<size_t>(width) * height, 0) {}

void Canvas::cls(uint8_t color) {
    std::fill(pixels_.begin(), pixels_.end(), static_cast<uint8_t>(color & 0x0f));
}

void Canvas::pset(int32_t x, int32_t y, uint8_t color) {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
    pixels_[static_cast<size_t>(y) * width_ + x] = static_cast<uint8_t>(color & 0x0f);
}

uint8_t Canvas::pget(int32_t x, int32_t y) const {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return 0;
    return pixels_[static_cast<size_t>(y) * width_ + x];
}

void Canvas::hline(int32_t x1, int32_t x2, int32_t y, uint8_t color) {
    if (y < 0 || y >= height_) return;
    if (x1 > x2) std::swap(x1, x2);
    if (x2 < 0 || x1 >= width_) return;
    if (x1 < 0) x1 = 0;
    if (x2 >= width_) x2 = width_ - 1;
    const uint8_t c = static_cast<uint8_t>(color & 0x0f);
    auto *row = pixels_.data() + static_cast<size_t>(y) * width_;
    for (int32_t x = x1; x <= x2; ++x) {
        row[x] = c;
    }
}

void Canvas::line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t color) {
    // 整数 Bresenham（全8オクタント対応）。
    int32_t dx = std::abs(x2 - x1);
    int32_t dy = -std::abs(y2 - y1);
    int32_t sx = x1 < x2 ? 1 : -1;
    int32_t sy = y1 < y2 ? 1 : -1;
    int32_t err = dx + dy;
    int32_t x = x1;
    int32_t y = y1;
    while (true) {
        pset(x, y, color);
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
    int32_t x1 = x;
    int32_t y1 = y;
    int32_t x2 = x + w - 1;
    int32_t y2 = y + h - 1;
    if (x2 < 0 || y2 < 0 || x1 >= width_ || y1 >= height_) return;
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= width_) x2 = width_ - 1;
    if (y2 >= height_) y2 = height_ - 1;
    const uint8_t c = static_cast<uint8_t>(color & 0x0f);
    for (int32_t yy = y1; yy <= y2; ++yy) {
        auto *row = pixels_.data() + static_cast<size_t>(yy) * width_;
        for (int32_t xx = x1; xx <= x2; ++xx) {
            row[xx] = c;
        }
    }
}

void Canvas::rectb(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t color) {
    if (w <= 0 || h <= 0) return;
    const int32_t x2 = x + w - 1;
    const int32_t y2 = y + h - 1;
    hline(x, x2, y, color);
    hline(x, x2, y2, color);
    // 縦辺（既に上下2行分は描いたのでその間だけ）。
    for (int32_t yy = y + 1; yy <= y2 - 1; ++yy) {
        pset(x, yy, color);
        pset(x2, yy, color);
    }
}

void Canvas::circ(int32_t cx, int32_t cy, int32_t r, uint8_t color) {
    if (r < 0) return;
    if (r == 0) {
        pset(cx, cy, color);
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
    if (r == 0) {
        pset(cx, cy, color);
        return;
    }
    int32_t x = r;
    int32_t y = 0;
    int32_t err = 1 - r;
    while (x >= y) {
        pset(cx + x, cy + y, color);
        pset(cx - x, cy + y, color);
        pset(cx + x, cy - y, color);
        pset(cx - x, cy - y, color);
        pset(cx + y, cy + x, color);
        pset(cx - y, cy + x, color);
        pset(cx + y, cy - x, color);
        pset(cx - y, cy - x, color);
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
    line(x1, y1, x2, y2, color);
    line(x2, y2, x3, y3, color);
    line(x3, y3, x1, y1, color);
}

void Canvas::tri(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint8_t color) {
    // M2a: 塗りは未実装。線のみで暫定（M2c で scanline 塗りに置換）。
    trib(x1, y1, x2, y2, x3, y3, color);
}

} // namespace pyxift
