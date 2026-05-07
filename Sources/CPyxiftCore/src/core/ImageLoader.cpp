#include "ImageLoader.hpp"

#include "Palette.hpp"

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_LINEAR
#define STBI_NO_HDR
#include "../../vendor/stb_image.h"

#include <cstdint>
#include <unordered_map>

namespace pyxift {

namespace {

uint8_t nearest_palette_index(uint32_t rgb) {
    const int32_t r = static_cast<int32_t>((rgb >> 16) & 0xff);
    const int32_t g = static_cast<int32_t>((rgb >> 8) & 0xff);
    const int32_t b = static_cast<int32_t>(rgb & 0xff);
    int32_t best_dist = INT32_MAX;
    uint8_t best_index = 0;
    for (size_t i = 0; i < kDefaultPalette.size(); ++i) {
        const uint32_t pc = kDefaultPalette[i];
        const int32_t pr = static_cast<int32_t>((pc >> 16) & 0xff);
        const int32_t pg = static_cast<int32_t>((pc >> 8) & 0xff);
        const int32_t pb = static_cast<int32_t>(pc & 0xff);
        const int32_t dr = r - pr;
        const int32_t dg = g - pg;
        const int32_t db = b - pb;
        const int32_t dist = dr * dr + dg * dg + db * db;
        if (dist < best_dist) {
            best_dist = dist;
            best_index = static_cast<uint8_t>(i);
        }
    }
    return best_index;
}

} // namespace

bool load_png_into_image(const std::string &path, Image &dest) {
    int width = 0;
    int height = 0;
    int channels = 0;
    // 本家 Pyxel が α を扱わないため、stb に 3ch 強制させて α を破棄する。
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 3);
    if (data == nullptr) return false;

    std::unordered_map<uint32_t, uint8_t> cache;
    cache.reserve(64);

    const int32_t copy_w = width < dest.width() ? width : dest.width();
    const int32_t copy_h = height < dest.height() ? height : dest.height();

    for (int32_t y = 0; y < copy_h; ++y) {
        const unsigned char *row = data + static_cast<size_t>(y) * width * 3;
        for (int32_t x = 0; x < copy_w; ++x) {
            const unsigned char *p = row + x * 3;
            const uint32_t rgb =
                (static_cast<uint32_t>(p[0]) << 16) |
                (static_cast<uint32_t>(p[1]) << 8) |
                static_cast<uint32_t>(p[2]);
            uint8_t idx;
            auto it = cache.find(rgb);
            if (it != cache.end()) {
                idx = it->second;
            } else {
                idx = nearest_palette_index(rgb);
                cache.emplace(rgb, idx);
            }
            dest.pset(x, y, idx);
        }
    }

    stbi_image_free(data);
    return true;
}

} // namespace pyxift
