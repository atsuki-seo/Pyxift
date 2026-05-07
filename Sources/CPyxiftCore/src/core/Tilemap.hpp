#ifndef PYXIFT_CORE_TILEMAP_HPP
#define PYXIFT_CORE_TILEMAP_HPP

#include <cstdint>
#include <vector>

namespace pyxift {

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

    void set_cell(int32_t cx, int32_t cy, uint8_t tile_x, uint8_t tile_y);
    void get_cell(int32_t cx, int32_t cy, uint8_t &out_tx, uint8_t &out_ty) const;

private:
    std::vector<uint8_t> cells_;
    int32_t image_bank_ = 0;
};

} // namespace pyxift

#endif
