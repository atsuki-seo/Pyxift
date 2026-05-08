#ifndef PYXIFT_CORE_FONT_HPP
#define PYXIFT_CORE_FONT_HPP

// Source: kitao/pyxel crates/pyxel-core/src/settings.rs (FONT_DATA, FONT_WIDTH, FONT_HEIGHT)
// License: MIT (Copyright (c) 2018-2026 Takashi Kitao)

#include <array>
#include <cstdint>

namespace pyxift {

constexpr int32_t kFontGlyphWidth = 4;
constexpr int32_t kFontGlyphHeight = 6;
constexpr int32_t kFontAdvance = 4;
constexpr int32_t kFontLineHeight = 6;
constexpr int32_t kFontFirstChar = 0x20;
constexpr int32_t kFontLastChar = 0x7f;
constexpr int32_t kFontGlyphCount = kFontLastChar - kFontFirstChar + 1;

// Each u32 packs 24 bits in row-major order starting from the high bit (0x0080_0000), matching upstream FONT_DATA's storage layout.
inline constexpr std::array<uint32_t, kFontGlyphCount> kFontData = {
    0x000000u, 0x444040u, 0xaa0000u, 0xaeaea0u, 0x6c6c40u, 0x824820u, 0x4a4ac0u, 0x440000u, 0x244420u,
    0x844480u, 0xa4e4a0u, 0x04e400u, 0x000480u, 0x00e000u, 0x000040u, 0x224880u, 0x6aaac0u, 0x4c4440u,
    0xc248e0u, 0xc242c0u, 0xaae220u, 0xe8c2c0u, 0x68eae0u, 0xe24880u, 0xeaeae0u, 0xeae2c0u, 0x040400u,
    0x040480u, 0x248420u, 0x0e0e00u, 0x842480u, 0xe24040u, 0x4aa860u, 0x4aeaa0u, 0xcacac0u, 0x688860u,
    0xcaaac0u, 0xe8e8e0u, 0xe8e880u, 0x68ea60u, 0xaaeaa0u, 0xe444e0u, 0x222a40u, 0xaacaa0u, 0x8888e0u,
    0xaeeaa0u, 0xcaaaa0u, 0x4aaa40u, 0xcac880u, 0x4aae60u, 0xcaeca0u, 0x6842c0u, 0xe44440u, 0xaaaa60u,
    0xaaaa40u, 0xaaeea0u, 0xaa4aa0u, 0xaa4440u, 0xe248e0u, 0x644460u, 0x884220u, 0xc444c0u, 0x4a0000u,
    0x0000e0u, 0x840000u, 0x06aa60u, 0x8caac0u, 0x068860u, 0x26aa60u, 0x06ac60u, 0x24e440u, 0x06ae24u,
    0x8caaa0u, 0x404440u, 0x2022a4u, 0x8acca0u, 0xc444e0u, 0x0eeea0u, 0x0caaa0u, 0x04aa40u, 0x0caac8u,
    0x06aa62u, 0x068880u, 0x06c6c0u, 0x4e4460u, 0x0aaa60u, 0x0aaa40u, 0x0aaee0u, 0x0a44a0u, 0x0aa624u,
    0x0e24e0u, 0x64c460u, 0x444440u, 0xc464c0u, 0x6c0000u, 0xeeeee0u,
};

inline bool font_pixel(char ch, int32_t col, int32_t row) {
    if (col < 0 || col >= kFontGlyphWidth) return false;
    if (row < 0 || row >= kFontGlyphHeight) return false;
    const auto code = static_cast<unsigned char>(ch);
    if (code < kFontFirstChar || code > kFontLastChar) return false;
    const uint32_t bits = kFontData[code - kFontFirstChar];
    const int32_t shift = 23 - (row * kFontGlyphWidth + col);
    return ((bits >> shift) & 1u) != 0u;
}

} // namespace pyxift

#endif
