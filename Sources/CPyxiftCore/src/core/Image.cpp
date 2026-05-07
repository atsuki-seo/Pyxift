#include "Image.hpp"

namespace pyxift {

Image::Image() : pixels_(static_cast<size_t>(kWidth) * kHeight, 0) {}

void Image::pset(int32_t x, int32_t y, uint8_t color) {
    if (x < 0 || y < 0 || x >= kWidth || y >= kHeight) return;
    pixels_[static_cast<size_t>(y) * kWidth + x] = static_cast<uint8_t>(color & 0x0f);
}

uint8_t Image::pget(int32_t x, int32_t y) const {
    if (x < 0 || y < 0 || x >= kWidth || y >= kHeight) return 0;
    return pixels_[static_cast<size_t>(y) * kWidth + x];
}

} // namespace pyxift
