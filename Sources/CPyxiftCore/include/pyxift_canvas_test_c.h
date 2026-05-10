#ifndef PYXIFT_CANVAS_TEST_C_H
#define PYXIFT_CANVAS_TEST_C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct PyxiftCanvasHandle PyxiftCanvasHandle;
typedef struct PyxiftImageHandle PyxiftImageHandle;
typedef struct PyxiftTilemapHandle PyxiftTilemapHandle;

PyxiftCanvasHandle *pyxift_canvas_test_create(int32_t width, int32_t height);
void pyxift_canvas_test_destroy(PyxiftCanvasHandle *h);

int32_t pyxift_canvas_test_width(const PyxiftCanvasHandle *h);
int32_t pyxift_canvas_test_height(const PyxiftCanvasHandle *h);

void pyxift_canvas_test_resize(PyxiftCanvasHandle *h, int32_t width, int32_t height);
int32_t pyxift_canvas_test_save_png(const PyxiftCanvasHandle *h, const char *path, int32_t scale);

void pyxift_canvas_test_cls(PyxiftCanvasHandle *h, uint8_t color);
void pyxift_canvas_test_pset(PyxiftCanvasHandle *h, int32_t x, int32_t y, uint8_t color);
uint8_t pyxift_canvas_test_pget(const PyxiftCanvasHandle *h, int32_t x, int32_t y);

void pyxift_canvas_test_clip(PyxiftCanvasHandle *h, int32_t x, int32_t y, int32_t w, int32_t hh);
void pyxift_canvas_test_clip_reset(PyxiftCanvasHandle *h);
void pyxift_canvas_test_camera(PyxiftCanvasHandle *h, int32_t x, int32_t y);
void pyxift_canvas_test_camera_reset(PyxiftCanvasHandle *h);
void pyxift_canvas_test_pal(PyxiftCanvasHandle *h, uint8_t from, uint8_t to);
void pyxift_canvas_test_pal_reset(PyxiftCanvasHandle *h);
void pyxift_canvas_test_dither(PyxiftCanvasHandle *h, double alpha);

void pyxift_canvas_test_rect(PyxiftCanvasHandle *h, int32_t x, int32_t y, int32_t w, int32_t hh, uint8_t color);
void pyxift_canvas_test_rectb(PyxiftCanvasHandle *h, int32_t x, int32_t y, int32_t w, int32_t hh, uint8_t color);
void pyxift_canvas_test_circ(PyxiftCanvasHandle *h, int32_t x, int32_t y, int32_t r, uint8_t color);
void pyxift_canvas_test_circb(PyxiftCanvasHandle *h, int32_t x, int32_t y, int32_t r, uint8_t color);
void pyxift_canvas_test_elli(PyxiftCanvasHandle *h, int32_t x, int32_t y, int32_t w, int32_t hh, uint8_t color);
void pyxift_canvas_test_ellib(PyxiftCanvasHandle *h, int32_t x, int32_t y, int32_t w, int32_t hh, uint8_t color);
void pyxift_canvas_test_line(PyxiftCanvasHandle *h, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t color);
void pyxift_canvas_test_fill(PyxiftCanvasHandle *h, int32_t x, int32_t y, uint8_t color);

PyxiftImageHandle *pyxift_image_test_create(void);
void pyxift_image_test_destroy(PyxiftImageHandle *h);
void pyxift_image_test_pset(PyxiftImageHandle *h, int32_t x, int32_t y, uint8_t color);

PyxiftTilemapHandle *pyxift_tilemap_test_create(void);
void pyxift_tilemap_test_destroy(PyxiftTilemapHandle *h);
void pyxift_tilemap_test_set_cell(PyxiftTilemapHandle *h, int32_t cx, int32_t cy,
                                  uint8_t tile_x, uint8_t tile_y);

void pyxift_canvas_test_blt(PyxiftCanvasHandle *h,
                            int32_t x, int32_t y,
                            const PyxiftImageHandle *img,
                            int32_t u, int32_t v, int32_t w, int32_t hh,
                            int32_t transparent,
                            double rotate_deg, double scale);

void pyxift_canvas_test_bltm(PyxiftCanvasHandle *h,
                             int32_t x, int32_t y,
                             const PyxiftTilemapHandle *tm,
                             const PyxiftImageHandle *img,
                             int32_t tu, int32_t tv, int32_t tw, int32_t th,
                             int32_t transparent,
                             double rotate_deg, double scale);

#ifdef __cplusplus
}
#endif

#endif
