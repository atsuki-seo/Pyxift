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

    void line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t color);
    void rect(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t color);
    void rectb(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t color);
    void circ(int32_t x, int32_t y, int32_t r, uint8_t color);
    void circb(int32_t x, int32_t y, int32_t r, uint8_t color);
    void tri(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint8_t color);
    void trib(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint8_t color);

private:
    int32_t width_;
    int32_t height_;
    std::vector<uint8_t> pixels_;  // インデックスカラー 0..15

    void hline(int32_t x1, int32_t x2, int32_t y, uint8_t color);
};

} // namespace pyxift

#endif
