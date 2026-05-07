#ifndef PYXIFT_CORE_IMAGE_HPP
#define PYXIFT_CORE_IMAGE_HPP

#include <cstdint>
#include <vector>

namespace pyxift {

// 16色インデックスのスプライトバンク。サイズは Pyxel 既定の 256×256 固定。
class Image {
public:
    static constexpr int32_t kWidth = 256;
    static constexpr int32_t kHeight = 256;

    Image();

    int32_t width() const { return kWidth; }
    int32_t height() const { return kHeight; }
    const uint8_t *pixels() const { return pixels_.data(); }

    // 書き込みは v0.1 では loadImage（M4）が主役だが、M2c では C-API のテスト用に直接 pset を露出する。
    // 範囲外は黙って無視（Canvas の put と同じ方針）。
    void pset(int32_t x, int32_t y, uint8_t color);
    uint8_t pget(int32_t x, int32_t y) const;

private:
    std::vector<uint8_t> pixels_;  // インデックスカラー 0..15
};

} // namespace pyxift

#endif
