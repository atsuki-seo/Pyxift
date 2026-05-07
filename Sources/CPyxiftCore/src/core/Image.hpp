#ifndef PYXIFT_CORE_IMAGE_HPP
#define PYXIFT_CORE_IMAGE_HPP

#include <cstdint>
#include <vector>

namespace pyxift {

class Image {
public:
    static constexpr int32_t kWidth = 256;
    static constexpr int32_t kHeight = 256;

    Image();

    int32_t width() const { return kWidth; }
    int32_t height() const { return kHeight; }
    const uint8_t *pixels() const { return pixels_.data(); }

    void pset(int32_t x, int32_t y, uint8_t color);
    uint8_t pget(int32_t x, int32_t y) const;

private:
    std::vector<uint8_t> pixels_;
};

} // namespace pyxift

#endif
