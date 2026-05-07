#include "Tilemap.hpp"

namespace pyxift {

Tilemap::Tilemap() : cells_(static_cast<size_t>(kWidth) * kHeight * 2, 0) {}

void Tilemap::set_cell(int32_t cx, int32_t cy, uint8_t tile_x, uint8_t tile_y) {
    if (cx < 0 || cy < 0 || cx >= kWidth || cy >= kHeight) return;
    const size_t idx = (static_cast<size_t>(cy) * kWidth + cx) * 2;
    cells_[idx + 0] = tile_x;
    cells_[idx + 1] = tile_y;
}

void Tilemap::get_cell(int32_t cx, int32_t cy, uint8_t &out_tx, uint8_t &out_ty) const {
    if (cx < 0 || cy < 0 || cx >= kWidth || cy >= kHeight) {
        out_tx = 0;
        out_ty = 0;
        return;
    }
    const size_t idx = (static_cast<size_t>(cy) * kWidth + cx) * 2;
    out_tx = cells_[idx + 0];
    out_ty = cells_[idx + 1];
}

} // namespace pyxift
