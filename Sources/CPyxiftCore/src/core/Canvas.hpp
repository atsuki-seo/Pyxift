#ifndef PYXIFT_CORE_CANVAS_HPP
#define PYXIFT_CORE_CANVAS_HPP

#include <cstdint>
#include <vector>

namespace pyxift {

// 16色インデックスのフレームバッファ。SDL3 非依存（コア層）。
class Canvas {
public:
    Canvas(int32_t width, int32_t height);

    int32_t width() const { return width_; }
    int32_t height() const { return height_; }
    const uint8_t *pixels() const { return pixels_.data(); }

    void cls(uint8_t color);
    void pset(int32_t x, int32_t y, uint8_t color);
    uint8_t pget(int32_t x, int32_t y) const;

private:
    int32_t width_;
    int32_t height_;
    std::vector<uint8_t> pixels_;  // インデックスカラー 0..15
};

} // namespace pyxift

#endif
