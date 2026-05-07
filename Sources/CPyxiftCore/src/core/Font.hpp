#ifndef PYXIFT_CORE_FONT_HPP
#define PYXIFT_CORE_FONT_HPP

#include <cstdint>

// 内蔵フォント。M2c 時点ではダミー（全グリフ 4×6 ベタ塗り）。
// M4 で本家 FONT_DATA（kitao/pyxel crates/pyxel-core/src/settings.rs）を流し込む際は
// このヘッダにビットパッキング配列を置き、glyph_row_bits で抽出する形に置き換える。
namespace pyxift {

constexpr int32_t kFontGlyphWidth = 4;
constexpr int32_t kFontGlyphHeight = 6;
constexpr int32_t kFontAdvance = 4;        // グリフ間ピッチ。本家と同じ詰め幅
constexpr int32_t kFontLineHeight = 6;
constexpr int32_t kFontFirstChar = 0x20;
constexpr int32_t kFontLastChar = 0x7f;

// (col, row) のドットを描くべきか。M2c のダミー実装はスペースだけ空白、それ以外はベタ塗り。
inline bool font_pixel(char ch, int32_t col, int32_t row) {
    (void)col;
    (void)row;
    if (ch == ' ') return false;
    if (static_cast<unsigned char>(ch) < kFontFirstChar) return false;
    if (static_cast<unsigned char>(ch) > kFontLastChar) return false;
    return true;
}

} // namespace pyxift

#endif
