#include "pyxift_font_test_c.h"

#include "core/Font.hpp"

extern "C" {

bool pyxift_font_pixel(char ch, int32_t col, int32_t row) {
    return pyxift::font_pixel(ch, col, row);
}

int32_t pyxift_font_glyph_width(void)  { return pyxift::kFontGlyphWidth; }
int32_t pyxift_font_glyph_height(void) { return pyxift::kFontGlyphHeight; }
int32_t pyxift_font_first_char(void)   { return pyxift::kFontFirstChar; }
int32_t pyxift_font_last_char(void)    { return pyxift::kFontLastChar; }

} // extern "C"
