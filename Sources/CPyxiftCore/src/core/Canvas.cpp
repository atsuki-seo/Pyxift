// Source: kitao/pyxel crates/pyxel-core/src/canvas.rs
//   - DITHERING_MATRIX, ELLIPSE_ROUNDING_BIAS constants
//   - Algorithmic structure for elli/ellib (two-pass scan), fill (scanline span fill),
//     and blt_transformed (inverse-mapped rotate+scale blit).
// License: MIT (Copyright (c) 2018-2026 Takashi Kitao)

#include "Canvas.hpp"

#include "Font.hpp"
#include "Image.hpp"
#include "Tilemap.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <utility>
#include <vector>

namespace pyxift {

namespace {

// 4x4 Bayer ordered-dithering threshold matrix; reproduces upstream Pyxel exactly so callers see
// the same masking pattern as the Rust reference engine.
constexpr double DITHERING_MATRIX[4][4] = {
    { 1.0 / 16.0,  9.0 / 16.0,  3.0 / 16.0, 11.0 / 16.0},
    {13.0 / 16.0,  5.0 / 16.0, 15.0 / 16.0,  7.0 / 16.0},
    { 3.0 / 16.0, 11.0 / 16.0,  1.0 / 16.0,  9.0 / 16.0},
    {15.0 / 16.0,  7.0 / 16.0, 13.0 / 16.0,  5.0 / 16.0},
};

// Upstream nudges ellipse boundary pixels outward by this much before rounding so symmetric
// pixels are not lost to ties at the .5 boundary.
constexpr double ELLIPSE_ROUNDING_BIAS = 0.01;

inline int32_t round_to_i32(double v) {
    return static_cast<int32_t>(std::lround(v));
}

inline int32_t pmod4(int32_t v) {
    return ((v % 4) + 4) % 4;
}

} // namespace

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

void Canvas::resize(int32_t width, int32_t height) {
    if (width <= 0 || height <= 0) return;
    width_ = width;
    height_ = height;
    pixels_.assign(static_cast<size_t>(width) * height, 0);
    clip_x1_ = 0;
    clip_y1_ = 0;
    clip_x2_ = width - 1;
    clip_y2_ = height - 1;
    camera_x_ = 0;
    camera_y_ = 0;
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

Canvas::State Canvas::save_state() const {
    return State{clip_x1_, clip_y1_, clip_x2_, clip_y2_,
                 camera_x_, camera_y_, palette_, dither_alpha_};
}

void Canvas::restore_state(const State &s) {
    clip_x1_ = s.clip_x1; clip_y1_ = s.clip_y1;
    clip_x2_ = s.clip_x2; clip_y2_ = s.clip_y2;
    camera_x_ = s.camera_x; camera_y_ = s.camera_y;
    palette_ = s.palette;
    dither_alpha_ = s.dither_alpha;
}

void Canvas::set_dither(double alpha) {
    if (alpha < 0.0) alpha = 0.0;
    if (alpha > 1.0) alpha = 1.0;
    dither_alpha_ = alpha;
}

bool Canvas::dither_pass(int32_t x, int32_t y) const {
    if (dither_alpha_ >= 1.0) return true;
    if (dither_alpha_ <= 0.0) return false;
    // Match upstream: strict greater-than against the 4x4 Bayer threshold at (x mod 4, y mod 4).
    return dither_alpha_ > DITHERING_MATRIX[pmod4(y)][pmod4(x)];
}

void Canvas::put(int32_t x, int32_t y, uint8_t color) {
    if (x < clip_x1_ || x > clip_x2_ || y < clip_y1_ || y > clip_y2_) return;
    if (!dither_pass(x, y)) return;
    pixels_[static_cast<size_t>(y) * width_ + x] = palette_[color & 0x0f];
}

void Canvas::cls(uint8_t color) {
    // Match upstream: cls ignores clip, camera, and dither, and fills the entire framebuffer.
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
    if (dither_alpha_ >= 1.0) {
        for (int32_t x = x1; x <= x2; ++x) {
            row[x] = c;
        }
    } else {
        for (int32_t x = x1; x <= x2; ++x) {
            if (dither_pass(x, y)) row[x] = c;
        }
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
    if (dither_alpha_ >= 1.0) {
        for (int32_t yy = y1; yy <= y2; ++yy) {
            auto *row = pixels_.data() + static_cast<size_t>(yy) * width_;
            for (int32_t xx = x1; xx <= x2; ++xx) {
                row[xx] = c;
            }
        }
    } else {
        for (int32_t yy = y1; yy <= y2; ++yy) {
            auto *row = pixels_.data() + static_cast<size_t>(yy) * width_;
            for (int32_t xx = x1; xx <= x2; ++xx) {
                if (dither_pass(xx, yy)) row[xx] = c;
            }
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

void Canvas::elli(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t color) {
    if (w <= 0 || h <= 0) return;
    x -= camera_x_;
    y -= camera_y_;
    const double ra = (w - 1) / 2.0;
    const double rb = (h - 1) / 2.0;
    const double cx = x + ra;
    const double cy = y + rb;

    auto y_extent = [ra, rb](double dx) -> double {
        if (ra <= 0.0) return rb;
        const double t = 1.0 - (dx * dx) / (ra * ra);
        return t > 0.0 ? rb * std::sqrt(t) : 0.0;
    };
    auto x_extent = [ra, rb](double dy) -> double {
        if (rb <= 0.0) return ra;
        const double t = 1.0 - (dy * dy) / (rb * rb);
        return t > 0.0 ? ra * std::sqrt(t) : 0.0;
    };

    // Match upstream: two-pass scan (vertical strips for the left half, horizontal scans for the
    // top half) so corner pixels missed by either single pass are still covered.
    const int32_t xi_end = x + (w / 2);
    for (int32_t xi = x; xi <= xi_end; ++xi) {
        const double dx = xi - cx;
        const double dy = y_extent(dx);
        const int32_t y1 = round_to_i32(cy - dy - ELLIPSE_ROUNDING_BIAS);
        const int32_t y2 = round_to_i32(cy + dy + ELLIPSE_ROUNDING_BIAS);
        const int32_t xi2 = round_to_i32(2.0 * cx - xi);
        for (int32_t yy = y1; yy <= y2; ++yy) {
            put(xi, yy, color);
            if (xi2 != xi) put(xi2, yy, color);
        }
    }
    const int32_t yi_end = y + (h / 2);
    for (int32_t yi = y; yi <= yi_end; ++yi) {
        const double dy = yi - cy;
        const double dx = x_extent(dy);
        const int32_t x1 = round_to_i32(cx - dx - ELLIPSE_ROUNDING_BIAS);
        const int32_t x2 = round_to_i32(cx + dx + ELLIPSE_ROUNDING_BIAS);
        const int32_t yi2 = round_to_i32(2.0 * cy - yi);
        hline(x1, x2, yi, color);
        if (yi2 != yi) hline(x1, x2, yi2, color);
    }
}

void Canvas::ellib(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t color) {
    if (w <= 0 || h <= 0) return;
    x -= camera_x_;
    y -= camera_y_;
    const double ra = (w - 1) / 2.0;
    const double rb = (h - 1) / 2.0;
    const double cx = x + ra;
    const double cy = y + rb;

    auto y_extent = [ra, rb](double dx) -> double {
        if (ra <= 0.0) return rb;
        const double t = 1.0 - (dx * dx) / (ra * ra);
        return t > 0.0 ? rb * std::sqrt(t) : 0.0;
    };
    auto x_extent = [ra, rb](double dy) -> double {
        if (rb <= 0.0) return ra;
        const double t = 1.0 - (dy * dy) / (rb * rb);
        return t > 0.0 ? ra * std::sqrt(t) : 0.0;
    };

    const int32_t xi_end = x + (w / 2);
    for (int32_t xi = x; xi <= xi_end; ++xi) {
        const double dx = xi - cx;
        const double dy = y_extent(dx);
        const int32_t y1 = round_to_i32(cy - dy - ELLIPSE_ROUNDING_BIAS);
        const int32_t y2 = round_to_i32(cy + dy + ELLIPSE_ROUNDING_BIAS);
        const int32_t xi2 = round_to_i32(2.0 * cx - xi);
        put(xi, y1, color);
        put(xi, y2, color);
        if (xi2 != xi) {
            put(xi2, y1, color);
            put(xi2, y2, color);
        }
    }
    const int32_t yi_end = y + (h / 2);
    for (int32_t yi = y; yi <= yi_end; ++yi) {
        const double dy = yi - cy;
        const double dx = x_extent(dy);
        const int32_t x1 = round_to_i32(cx - dx - ELLIPSE_ROUNDING_BIAS);
        const int32_t x2 = round_to_i32(cx + dx + ELLIPSE_ROUNDING_BIAS);
        const int32_t yi2 = round_to_i32(2.0 * cy - yi);
        put(x1, yi, color);
        put(x2, yi, color);
        if (yi2 != yi) {
            put(x1, yi2, color);
            put(x2, yi2, color);
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

void Canvas::fill(int32_t x, int32_t y, uint8_t color) {
    x -= camera_x_;
    y -= camera_y_;
    if (x < clip_x1_ || x > clip_x2_ || y < clip_y1_ || y > clip_y2_) return;
    const uint8_t target = pixels_[static_cast<size_t>(y) * width_ + x];
    const uint8_t mapped = palette_[color & 0x0f];
    if (target == mapped) return;

    // Dither-skipped pixels stay at the target color, so a naive flood would re-seed the same row
    // forever. Track visited cells in a side bitmap that records run membership independently of
    // the framebuffer.
    std::vector<uint8_t> visited(static_cast<size_t>(width_) * height_, 0);
    auto mark_visited = [&](int32_t px, int32_t py) {
        visited[static_cast<size_t>(py) * width_ + px] = 1;
    };
    auto is_visited = [&](int32_t px, int32_t py) {
        return visited[static_cast<size_t>(py) * width_ + px] != 0;
    };
    auto same = [&](int32_t px, int32_t py) {
        return !is_visited(px, py)
            && pixels_[static_cast<size_t>(py) * width_ + px] == target;
    };

    std::vector<std::pair<int32_t, int32_t>> stack;
    stack.reserve(64);
    stack.emplace_back(x, y);

    while (!stack.empty()) {
        auto [sx, sy] = stack.back();
        stack.pop_back();
        if (!same(sx, sy)) continue;

        int32_t left = sx;
        while (left > clip_x1_ && same(left - 1, sy)) --left;
        int32_t right = sx;
        while (right < clip_x2_ && same(right + 1, sy)) ++right;

        auto *row = pixels_.data() + static_cast<size_t>(sy) * width_;
        if (dither_alpha_ >= 1.0) {
            for (int32_t xx = left; xx <= right; ++xx) {
                row[xx] = mapped;
                mark_visited(xx, sy);
            }
        } else {
            for (int32_t xx = left; xx <= right; ++xx) {
                if (dither_pass(xx, sy)) row[xx] = mapped;
                mark_visited(xx, sy);
            }
        }

        auto seed_row = [&](int32_t ny) {
            if (ny < clip_y1_ || ny > clip_y2_) return;
            int32_t xx = left;
            while (xx <= right) {
                while (xx <= right && !same(xx, ny)) ++xx;
                if (xx > right) break;
                int32_t run_end = xx;
                while (run_end + 1 <= right && same(run_end + 1, ny)) ++run_end;
                stack.emplace_back(run_end, ny);
                xx = run_end + 1;
            }
        };
        seed_row(sy - 1);
        seed_row(sy + 1);
    }
}

void Canvas::blt(int32_t x, int32_t y, const Image &image,
                 int32_t u, int32_t v, int32_t w, int32_t h,
                 int32_t transparent,
                 double rotate_deg, double scale) {
    if (w <= 0 || h <= 0) return;
    if (scale <= 0.0) return;
    if (rotate_deg != 0.0 || scale != 1.0) {
        blt_transformed(x, y, image, u, v, w, h, transparent, rotate_deg, scale);
        return;
    }
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
            if (!dither_pass(dx, dy)) continue;
            dst_row[dx] = palette_[s & 0x0f];
        }
    }
}

void Canvas::blt_transformed(int32_t x, int32_t y, const Image &image,
                             int32_t u, int32_t v, int32_t w, int32_t h,
                             int32_t transparent,
                             double rotate_deg, double scale) {
    const double half_w = (w - 1) / 2.0;
    const double half_h = (h - 1) / 2.0;
    const double dst_cx = (x - camera_x_) + half_w;
    const double dst_cy = (y - camera_y_) + half_h;
    const double src_cx = u + half_w;
    const double src_cy = v + half_h;

    // Match upstream: angle is in degrees, clockwise positive (sin gets a sign flip).
    const double rad = rotate_deg * 3.14159265358979323846 / 180.0;
    const double sin_t = -std::sin(rad);
    const double cos_t = std::cos(rad);

    const double abs_cos = std::fabs(cos_t);
    const double abs_sin = std::fabs(sin_t);
    const double bound_x = (half_w * abs_cos + half_h * abs_sin + 1.0) * scale;
    const double bound_y = (half_w * abs_sin + half_h * abs_cos + 1.0) * scale;
    int32_t y0 = std::max(clip_y1_, round_to_i32(dst_cy - bound_y));
    int32_t y1 = std::min(clip_y2_, round_to_i32(dst_cy + bound_y));
    int32_t x0 = std::max(clip_x1_, round_to_i32(dst_cx - bound_x));
    int32_t x1 = std::min(clip_x2_, round_to_i32(dst_cx + bound_x));
    if (x0 > x1 || y0 > y1) return;

    const int32_t img_w = image.width();
    const int32_t img_h = image.height();
    const uint8_t *src_pixels = image.pixels();
    const double inv_scale = 1.0 / scale;

    for (int32_t dy = y0; dy <= y1; ++dy) {
        const double oy = dy - dst_cy;
        for (int32_t dx = x0; dx <= x1; ++dx) {
            const double ox = dx - dst_cx;
            const double sx = src_cx + (ox * cos_t - oy * sin_t) * inv_scale;
            const double sy = src_cy + (ox * sin_t + oy * cos_t) * inv_scale;
            const int32_t sxi = round_to_i32(sx);
            const int32_t syi = round_to_i32(sy);
            if (sxi < u || sxi >= u + w) continue;
            if (syi < v || syi >= v + h) continue;
            if (sxi < 0 || sxi >= img_w || syi < 0 || syi >= img_h) continue;
            const uint8_t s = src_pixels[static_cast<size_t>(syi) * img_w + sxi];
            if (transparent >= 0 && s == static_cast<uint8_t>(transparent)) continue;
            if (!dither_pass(dx, dy)) continue;
            pixels_[static_cast<size_t>(dy) * width_ + dx] = palette_[s & 0x0f];
        }
    }
}

void Canvas::bltm(int32_t x, int32_t y, const Tilemap &tilemap, const Image &image,
                  int32_t tu, int32_t tv, int32_t tw, int32_t th,
                  int32_t transparent,
                  double rotate_deg, double scale) {
    if (tw <= 0 || th <= 0) return;
    if (scale <= 0.0) return;
    const int32_t ts = Tilemap::kTileSize;

    if (rotate_deg == 0.0 && scale == 1.0) {
        for (int32_t cy = 0; cy < th; ++cy) {
            for (int32_t cx = 0; cx < tw; ++cx) {
                uint8_t tx_idx, ty_idx;
                tilemap.get_cell(tu + cx, tv + cy, tx_idx, ty_idx);
                blt(x + cx * ts, y + cy * ts, image,
                    static_cast<int32_t>(tx_idx) * ts, static_cast<int32_t>(ty_idx) * ts,
                    ts, ts, transparent);
            }
        }
        return;
    }

    const int32_t pixel_w = tw * ts;
    const int32_t pixel_h = th * ts;
    // Scratch Image is fixed at Image::kWidth x kHeight; refuse to rasterize regions that exceed
    // it rather than overflow the backing buffer.
    if (pixel_w > Image::kWidth || pixel_h > Image::kHeight) return;
    Image scratch;
    for (int32_t cy = 0; cy < th; ++cy) {
        for (int32_t cx = 0; cx < tw; ++cx) {
            uint8_t tx_idx, ty_idx;
            tilemap.get_cell(tu + cx, tv + cy, tx_idx, ty_idx);
            const int32_t src_x0 = static_cast<int32_t>(tx_idx) * ts;
            const int32_t src_y0 = static_cast<int32_t>(ty_idx) * ts;
            for (int32_t row = 0; row < ts; ++row) {
                for (int32_t col = 0; col < ts; ++col) {
                    const uint8_t c = image.pget(src_x0 + col, src_y0 + row);
                    scratch.pset(cx * ts + col, cy * ts + row, c);
                }
            }
        }
    }
    blt_transformed(x, y, scratch, 0, 0, pixel_w, pixel_h, transparent, rotate_deg, scale);
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
