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

#ifdef __cplusplus
}
#endif

#endif
