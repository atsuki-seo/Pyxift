#ifndef PYXIFT_C_H
#define PYXIFT_C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct PyxiftEngine PyxiftEngine;

typedef void (*PyxiftUpdateFn)(void *user);
typedef void (*PyxiftDrawFn)(void *user);

PyxiftEngine *pyxift_engine_create(int32_t width,
                                   int32_t height,
                                   const char *title,
                                   int32_t fps);

void pyxift_engine_destroy(PyxiftEngine *engine);

void pyxift_engine_run(PyxiftEngine *engine,
                       PyxiftUpdateFn update,
                       PyxiftDrawFn draw,
                       void *user);

void pyxift_engine_quit(PyxiftEngine *engine);

void pyxift_engine_set_title(PyxiftEngine *engine, const char *title);

int32_t pyxift_engine_width(const PyxiftEngine *engine);
int32_t pyxift_engine_height(const PyxiftEngine *engine);
int32_t pyxift_engine_frame_count(const PyxiftEngine *engine);

void pyxift_engine_cls(PyxiftEngine *engine, uint8_t color);
void pyxift_engine_pset(PyxiftEngine *engine, int32_t x, int32_t y, uint8_t color);
uint8_t pyxift_engine_pget(const PyxiftEngine *engine, int32_t x, int32_t y);

void pyxift_engine_line(PyxiftEngine *engine,
                        int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                        uint8_t color);
void pyxift_engine_rect(PyxiftEngine *engine,
                        int32_t x, int32_t y, int32_t w, int32_t h,
                        uint8_t color);
void pyxift_engine_rectb(PyxiftEngine *engine,
                         int32_t x, int32_t y, int32_t w, int32_t h,
                         uint8_t color);
void pyxift_engine_circ(PyxiftEngine *engine,
                        int32_t x, int32_t y, int32_t r,
                        uint8_t color);
void pyxift_engine_circb(PyxiftEngine *engine,
                         int32_t x, int32_t y, int32_t r,
                         uint8_t color);
void pyxift_engine_tri(PyxiftEngine *engine,
                       int32_t x1, int32_t y1,
                       int32_t x2, int32_t y2,
                       int32_t x3, int32_t y3,
                       uint8_t color);
void pyxift_engine_trib(PyxiftEngine *engine,
                        int32_t x1, int32_t y1,
                        int32_t x2, int32_t y2,
                        int32_t x3, int32_t y3,
                        uint8_t color);

void pyxift_engine_clip(PyxiftEngine *engine,
                        int32_t x, int32_t y, int32_t w, int32_t h);
void pyxift_engine_clip_reset(PyxiftEngine *engine);

void pyxift_engine_camera(PyxiftEngine *engine, int32_t x, int32_t y);
void pyxift_engine_camera_reset(PyxiftEngine *engine);

void pyxift_engine_pal(PyxiftEngine *engine, uint8_t from, uint8_t to);
void pyxift_engine_pal_reset(PyxiftEngine *engine);

// 転送系 (M2c)。transparent は 0..15 の色インデックス、もしくは「透明色なし」を表す -1。
void pyxift_engine_blt(PyxiftEngine *engine,
                       int32_t x, int32_t y,
                       int32_t image_bank,
                       int32_t u, int32_t v, int32_t w, int32_t h,
                       int32_t transparent);
void pyxift_engine_bltm(PyxiftEngine *engine,
                        int32_t x, int32_t y,
                        int32_t tilemap_index,
                        int32_t u, int32_t v, int32_t w, int32_t h,
                        int32_t transparent);
void pyxift_engine_text(PyxiftEngine *engine,
                        int32_t x, int32_t y, const char *s, uint8_t color);

// 画像バンク・タイルマップ書き込み (M2c のテスト用 / M4 で loadImage が後から共用)。
void pyxift_engine_image_pset(PyxiftEngine *engine,
                              int32_t image_bank,
                              int32_t x, int32_t y, uint8_t color);
void pyxift_engine_tilemap_set(PyxiftEngine *engine,
                               int32_t tilemap_index,
                               int32_t cx, int32_t cy,
                               uint8_t tile_x, uint8_t tile_y);
void pyxift_engine_tilemap_set_image_bank(PyxiftEngine *engine,
                                          int32_t tilemap_index,
                                          int32_t image_bank);

// 入力 (M3)。
// Button インデックスは Pyxift 側 enum と一致 (left=0, right=1, up=2, down=3, a=4, b=5, start=6)。
// MouseButton は left=0, right=1, middle=2。
// Key は SDL3 SDL_Keycode の int32 値をそのまま渡す。
bool pyxift_engine_button(const PyxiftEngine *engine, uint8_t button, int32_t player);
bool pyxift_engine_button_pressed(const PyxiftEngine *engine, uint8_t button, int32_t player);
bool pyxift_engine_button_released(const PyxiftEngine *engine, uint8_t button, int32_t player);

bool pyxift_engine_key(const PyxiftEngine *engine, int32_t keycode);
bool pyxift_engine_key_pressed(const PyxiftEngine *engine, int32_t keycode);
bool pyxift_engine_key_released(const PyxiftEngine *engine, int32_t keycode);

void pyxift_engine_mouse(const PyxiftEngine *engine, int32_t *out_x, int32_t *out_y);
int32_t pyxift_engine_mouse_wheel(const PyxiftEngine *engine);
bool pyxift_engine_mouse_button(const PyxiftEngine *engine, uint8_t button);
bool pyxift_engine_mouse_button_pressed(const PyxiftEngine *engine, uint8_t button);
bool pyxift_engine_mouse_button_released(const PyxiftEngine *engine, uint8_t button);

void pyxift_engine_mouse_cursor(PyxiftEngine *engine, bool visible);

#ifdef __cplusplus
}
#endif

#endif
