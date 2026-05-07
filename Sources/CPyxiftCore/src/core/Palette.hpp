#ifndef PYXIFT_CORE_PALETTE_HPP
#define PYXIFT_CORE_PALETTE_HPP

// Source: kitao/pyxel crates/pyxel-core/src/settings.rs (DEFAULT_COLORS)
// License: MIT (Copyright (c) 2018-2026 Takashi Kitao)

#include <array>
#include <cstdint>

namespace pyxift {

// Pyxel既定16色パレット (0xRRGGBB)。
inline constexpr std::array<uint32_t, 16> kDefaultPalette = {
    0x000000u, // 0  BLACK
    0x2b335fu, // 1  NAVY
    0x7e2072u, // 2  PURPLE
    0x19959cu, // 3  GREEN
    0x8b4852u, // 4  BROWN
    0x395c98u, // 5  DARK_BLUE
    0xa9c1ffu, // 6  LIGHT_BLUE
    0xeeeeeeu, // 7  WHITE
    0xd4186cu, // 8  RED
    0xd38441u, // 9  ORANGE
    0xe9c35bu, // 10 YELLOW
    0x70c6a9u, // 11 LIME
    0x7696deu, // 12 CYAN
    0xa3a3a3u, // 13 GRAY
    0xff9798u, // 14 PINK
    0xedc7b0u, // 15 PEACH
};

inline constexpr uint32_t kBackgroundColor = 0x202224u;

} // namespace pyxift

#endif
