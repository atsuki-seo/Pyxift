#ifndef PYXIFT_CORE_TILEMAP_HPP
#define PYXIFT_CORE_TILEMAP_HPP

#include <cstdint>
#include <vector>

namespace pyxift {

// タイルマップ。Pyxel 既定: 256×256 セル、各セルは 8×8 ピクセルのタイル参照。
// 各セルは (tile_x, tile_y) のペアで「画像バンク内のどこから 8×8 を取るか」を指す。
// 参照先の画像バンクはマップ全体に対して 1 つ持つ（本家準拠: tilemap.imgsrc）。
class Tilemap {
public:
    static constexpr int32_t kWidth = 256;
    static constexpr int32_t kHeight = 256;
    static constexpr int32_t kTileSize = 8;

    Tilemap();

    int32_t width() const { return kWidth; }
    int32_t height() const { return kHeight; }

    int32_t image_bank() const { return image_bank_; }
    void set_image_bank(int32_t bank) { image_bank_ = bank; }

    // セルへの (tile_x, tile_y) 書き込み。範囲外は黙って無視。
    void set_cell(int32_t cx, int32_t cy, uint8_t tile_x, uint8_t tile_y);

    // 取得。範囲外は (0, 0) を返す。
    void get_cell(int32_t cx, int32_t cy, uint8_t &out_tx, uint8_t &out_ty) const;

private:
    // セルあたり 2 バイト（tx, ty）。連続バイト配列で 64KB×2 = 128KB 程度。
    std::vector<uint8_t> cells_;
    int32_t image_bank_ = 0;
};

} // namespace pyxift

#endif
