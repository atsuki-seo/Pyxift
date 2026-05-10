#include "PngWriter.hpp"

#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO_LARGE
#include "../../vendor/stb_image_write.h"

namespace pyxift {

bool save_indexed_png(const std::string &path,
                      const uint8_t *index_buffer,
                      int32_t width,
                      int32_t height,
                      const uint32_t *palette_rgb,
                      int32_t scale) {
    if (index_buffer == nullptr || palette_rgb == nullptr) return false;
    if (width <= 0 || height <= 0) return false;
    if (scale < 1) scale = 1;

    const int32_t out_w = width * scale;
    const int32_t out_h = height * scale;
    std::vector<uint8_t> rgb(static_cast<size_t>(out_w) * out_h * 3, 0);

    for (int32_t y = 0; y < height; ++y) {
        const uint8_t *src_row = index_buffer + static_cast<size_t>(y) * width;
        for (int32_t x = 0; x < width; ++x) {
            const uint32_t c = palette_rgb[src_row[x] & 0x0f];
            const uint8_t r = static_cast<uint8_t>((c >> 16) & 0xff);
            const uint8_t g = static_cast<uint8_t>((c >> 8) & 0xff);
            const uint8_t b = static_cast<uint8_t>(c & 0xff);
            for (int32_t dy = 0; dy < scale; ++dy) {
                uint8_t *dst_row = rgb.data() +
                    (static_cast<size_t>(y) * scale + dy) * out_w * 3 +
                    static_cast<size_t>(x) * scale * 3;
                for (int32_t dx = 0; dx < scale; ++dx) {
                    dst_row[dx * 3 + 0] = r;
                    dst_row[dx * 3 + 1] = g;
                    dst_row[dx * 3 + 2] = b;
                }
            }
        }
    }

    return stbi_write_png(path.c_str(), out_w, out_h, 3, rgb.data(), out_w * 3) != 0;
}

} // namespace pyxift
