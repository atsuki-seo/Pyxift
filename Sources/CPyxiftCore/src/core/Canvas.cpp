#include "Canvas.hpp"

#include <algorithm>

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

} // namespace pyxift
