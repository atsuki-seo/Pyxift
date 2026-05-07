#ifndef PYXIFT_CORE_PALETTE_HPP
#define PYXIFT_CORE_PALETTE_HPP

// Source: kitao/pyxel crates/pyxel-core/src/settings.rs (DEFAULT_COLORS)
// License: MIT (Copyright (c) 2018-2026 Takashi Kitao)

#include <array>
#include <cstdint>

namespace pyxift {

inline constexpr std::array<uint32_t, 16> kDefaultPalette = {
    0x000000u,
    0x2b335fu,
    0x7e2072u,
    0x19959cu,
    0x8b4852u,
    0x395c98u,
    0xa9c1ffu,
    0xeeeeeeu,
    0xd4186cu,
    0xd38441u,
    0xe9c35bu,
    0x70c6a9u,
    0x7696deu,
    0xa3a3a3u,
    0xff9798u,
    0xedc7b0u,
};

inline constexpr uint32_t kBackgroundColor = 0x202224u;

} // namespace pyxift

#endif
