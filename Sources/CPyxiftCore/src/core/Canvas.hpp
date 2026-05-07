#ifndef PYXIFT_CORE_CANVAS_HPP
#define PYXIFT_CORE_CANVAS_HPP

#include <array>
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

    // M2b: 状態系
    void set_clip(int32_t x, int32_t y, int32_t w, int32_t h);
    void reset_clip();
    void set_camera(int32_t x, int32_t y);
    void reset_camera();
    void set_pal(uint8_t from, uint8_t to);
    void reset_pal();

private:
    int32_t width_;
    int32_t height_;
    std::vector<uint8_t> pixels_;  // インデックスカラー 0..15

    // クリップ矩形（包含端 [x1,x2] × [y1,y2]）。reset_clip() で画面全体に戻す。
    int32_t clip_x1_;
    int32_t clip_y1_;
    int32_t clip_x2_;
    int32_t clip_y2_;

    // カメラオフセット。描画 API の入力座標から減算してフレームバッファ座標に変換する。
    int32_t camera_x_ = 0;
    int32_t camera_y_ = 0;

    // pal リマップ。palette_[i] = 書き込み時に i を置き換える先のインデックス。
    std::array<uint8_t, 16> palette_;

    // 中央チョークポイント: clip + palette を適用してピクセル1つを書く。
    // 入力座標は既に camera 適用済みのフレームバッファ座標。
    void put(int32_t x, int32_t y, uint8_t color);
    void hline(int32_t x1, int32_t x2, int32_t y, uint8_t color);
};

} // namespace pyxift

#endif
