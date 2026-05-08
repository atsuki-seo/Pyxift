#include "Canvas.hpp"

#include "Font.hpp"
#include "Image.hpp"
#include "Tilemap.hpp"

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
        // Match upstream: an empty rect collapses to a "draws nothing" state (x2 < x1).
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
    // Match upstream: cls ignores clip and camera and fills the entire framebuffer.
    std::fill(pixels_.begin(), pixels_.end(), palette_[color & 0x0f]);
}

void Canvas::pset(int32_t x, int32_t y, uint8_t color) {
    put(x - camera_x_, y - camera_y_, color);
}

uint8_t Canvas::pget(int32_t x, int32_t y) const {
    // Match upstream: pget applies the camera offset, returns 0 for out-of-screen reads, and ignores clip.
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
    line(x1, y1, x2, y2, color);
    line(x2, y2, x3, y3, color);
    line(x3, y3, x1, y1, color);
}

void Canvas::tri(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint8_t color) {
    x1 -= camera_x_; y1 -= camera_y_;
    x2 -= camera_x_; y2 -= camera_y_;
    x3 -= camera_x_; y3 -= camera_y_;
    if (y1 > y2) { std::swap(x1, x2); std::swap(y1, y2); }
    if (y1 > y3) { std::swap(x1, x3); std::swap(y1, y3); }
    if (y2 > y3) { std::swap(x2, x3); std::swap(y2, y3); }

    auto edge_x = [](int32_t ya, int32_t xa, int32_t yb, int32_t xb, int32_t y) -> int32_t {
        if (yb == ya) return xa;
        const int64_t num = static_cast<int64_t>(xb - xa) * (y - ya);
        const int64_t den = static_cast<int64_t>(yb - ya);
        return xa + static_cast<int32_t>(num / den);
    };

    if (y2 > y1) {
        for (int32_t y = y1; y <= y2; ++y) {
            const int32_t xa = edge_x(y1, x1, y3, x3, y);
            const int32_t xb = edge_x(y1, x1, y2, x2, y);
            hline(xa, xb, y, color);
        }
    }
    if (y3 > y2) {
        for (int32_t y = y2; y <= y3; ++y) {
            const int32_t xa = edge_x(y1, x1, y3, x3, y);
            const int32_t xb = edge_x(y2, x2, y3, x3, y);
            hline(xa, xb, y, color);
        }
    }
    if (y1 == y2 && y2 == y3) {
        const int32_t lo = std::min({x1, x2, x3});
        const int32_t hi = std::max({x1, x2, x3});
        hline(lo, hi, y1, color);
    }
}

void Canvas::blt(int32_t x, int32_t y, const Image &image,
                 int32_t u, int32_t v, int32_t w, int32_t h,
                 int32_t transparent) {
    if (w <= 0 || h <= 0) return;
    const int32_t dst_x = x - camera_x_;
    const int32_t dst_y = y - camera_y_;

    // Match upstream: negative u/v shifts the destination rather than clipping the source.
    int32_t src_x = u;
    int32_t src_y = v;
    int32_t src_w = w;
    int32_t src_h = h;
    int32_t shift_x = 0;
    int32_t shift_y = 0;
    if (src_x < 0) { shift_x = -src_x; src_w -= shift_x; src_x = 0; }
    if (src_y < 0) { shift_y = -src_y; src_h -= shift_y; src_y = 0; }
    if (src_x + src_w > image.width())  src_w = image.width()  - src_x;
    if (src_y + src_h > image.height()) src_h = image.height() - src_y;
    if (src_w <= 0 || src_h <= 0) return;

    const uint8_t *src_pixels = image.pixels();
    const int32_t src_stride = image.width();

    for (int32_t row = 0; row < src_h; ++row) {
        const int32_t dy = dst_y + shift_y + row;
        if (dy < clip_y1_ || dy > clip_y2_) continue;
        const uint8_t *src_row = src_pixels + static_cast<size_t>(src_y + row) * src_stride + src_x;
        uint8_t *dst_row = pixels_.data() + static_cast<size_t>(dy) * width_;
        for (int32_t col = 0; col < src_w; ++col) {
            const int32_t dx = dst_x + shift_x + col;
            if (dx < clip_x1_ || dx > clip_x2_) continue;
            const uint8_t s = src_row[col];
            // Match upstream: the transparency test is against the pre-pal-swap source color.
            if (transparent >= 0 && s == static_cast<uint8_t>(transparent)) continue;
            dst_row[dx] = palette_[s & 0x0f];
        }
    }
}

void Canvas::bltm(int32_t x, int32_t y, const Tilemap &tilemap, const Image &image,
                  int32_t tu, int32_t tv, int32_t tw, int32_t th,
                  int32_t transparent) {
    if (tw <= 0 || th <= 0) return;
    const int32_t ts = Tilemap::kTileSize;
    for (int32_t cy = 0; cy < th; ++cy) {
        for (int32_t cx = 0; cx < tw; ++cx) {
            uint8_t tx_idx, ty_idx;
            tilemap.get_cell(tu + cx, tv + cy, tx_idx, ty_idx);
            blt(x + cx * ts, y + cy * ts, image,
                static_cast<int32_t>(tx_idx) * ts, static_cast<int32_t>(ty_idx) * ts,
                ts, ts, transparent);
        }
    }
}

void Canvas::text(int32_t x, int32_t y, const char *s, uint8_t color) {
    if (s == nullptr) return;
    int32_t cx = x;
    int32_t cy = y;
    for (const char *p = s; *p != '\0'; ++p) {
        const char ch = *p;
        if (ch == '\n') {
            cx = x;
            cy += kFontLineHeight;
            continue;
        }
        for (int32_t row = 0; row < kFontGlyphHeight; ++row) {
            for (int32_t col = 0; col < kFontGlyphWidth; ++col) {
                if (font_pixel(ch, col, row)) {
                    pset(cx + col, cy + row, color);
                }
            }
        }
        cx += kFontAdvance;
    }
}

} // namespace pyxift
