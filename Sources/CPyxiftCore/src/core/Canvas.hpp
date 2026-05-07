#ifndef PYXIFT_CORE_CANVAS_HPP
#define PYXIFT_CORE_CANVAS_HPP

#include <array>
#include <cstdint>
#include <vector>

namespace pyxift {

class Image;
class Tilemap;

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

    // 転送系。transparent は 0..15 の色インデックス、-1 は透明色なし。
    void blt(int32_t x, int32_t y, const Image &image,
             int32_t u, int32_t v, int32_t w, int32_t h,
             int32_t transparent);
    void bltm(int32_t x, int32_t y, const Tilemap &tilemap, const Image &image,
              int32_t tu, int32_t tv, int32_t tw, int32_t th,
              int32_t transparent);
    void text(int32_t x, int32_t y, const char *s, uint8_t color);

    void set_clip(int32_t x, int32_t y, int32_t w, int32_t h);
    void reset_clip();
    void set_camera(int32_t x, int32_t y);
    void reset_camera();
    void set_pal(uint8_t from, uint8_t to);
    void reset_pal();

private:
    int32_t width_;
    int32_t height_;
    std::vector<uint8_t> pixels_;

    // クリップ矩形は包含端 [x1,x2] × [y1,y2]。空クリップは x2<x1 で表す。
    int32_t clip_x1_;
    int32_t clip_y1_;
    int32_t clip_x2_;
    int32_t clip_y2_;

    int32_t camera_x_ = 0;
    int32_t camera_y_ = 0;

    std::array<uint8_t, 16> palette_;

    // put() は camera 適用済み座標を受け取る（呼び出し側で減算）。
    void put(int32_t x, int32_t y, uint8_t color);
    void hline(int32_t x1, int32_t x2, int32_t y, uint8_t color);
};

} // namespace pyxift

#endif
