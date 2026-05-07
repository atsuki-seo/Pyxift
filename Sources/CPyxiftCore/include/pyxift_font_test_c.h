#ifndef PYXIFT_FONT_TEST_C_H
#define PYXIFT_FONT_TEST_C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

bool pyxift_font_pixel(char ch, int32_t col, int32_t row);
int32_t pyxift_font_glyph_width(void);
int32_t pyxift_font_glyph_height(void);
int32_t pyxift_font_first_char(void);
int32_t pyxift_font_last_char(void);

#ifdef __cplusplus
}
#endif

#endif
