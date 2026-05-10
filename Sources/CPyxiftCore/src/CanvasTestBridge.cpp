#include "pyxift_canvas_test_c.h"

#include "core/Canvas.hpp"
#include "core/Image.hpp"
#include "core/Palette.hpp"
#include "core/PngWriter.hpp"
#include "core/Tilemap.hpp"

struct PyxiftCanvasHandle {
    pyxift::Canvas canvas;
    PyxiftCanvasHandle(int32_t w, int32_t h) : canvas(w, h) {}
};

struct PyxiftImageHandle {
    pyxift::Image image;
};

struct PyxiftTilemapHandle {
    pyxift::Tilemap tilemap;
};

extern "C" {

PyxiftCanvasHandle *pyxift_canvas_test_create(int32_t width, int32_t height) {
    if (width <= 0 || height <= 0) return nullptr;
    return new PyxiftCanvasHandle(width, height);
}

void pyxift_canvas_test_destroy(PyxiftCanvasHandle *h) { delete h; }

int32_t pyxift_canvas_test_width(const PyxiftCanvasHandle *h) {
    return h != nullptr ? h->canvas.width() : 0;
}

int32_t pyxift_canvas_test_height(const PyxiftCanvasHandle *h) {
    return h != nullptr ? h->canvas.height() : 0;
}

void pyxift_canvas_test_resize(PyxiftCanvasHandle *h, int32_t width, int32_t height) {
    if (h != nullptr) h->canvas.resize(width, height);
}

int32_t pyxift_canvas_test_save_png(const PyxiftCanvasHandle *h, const char *path, int32_t scale) {
    if (h == nullptr || path == nullptr) return 0;
    const bool ok = pyxift::save_indexed_png(std::string(path),
                                             h->canvas.pixels(),
                                             h->canvas.width(),
                                             h->canvas.height(),
                                             pyxift::kDefaultPalette.data(),
                                             scale);
    return ok ? 1 : 0;
}

void pyxift_canvas_test_cls(PyxiftCanvasHandle *h, uint8_t color) {
    if (h != nullptr) h->canvas.cls(color);
}

void pyxift_canvas_test_pset(PyxiftCanvasHandle *h, int32_t x, int32_t y, uint8_t color) {
    if (h != nullptr) h->canvas.pset(x, y, color);
}

uint8_t pyxift_canvas_test_pget(const PyxiftCanvasHandle *h, int32_t x, int32_t y) {
    return h != nullptr ? h->canvas.pget(x, y) : 0;
}

void pyxift_canvas_test_clip(PyxiftCanvasHandle *h, int32_t x, int32_t y, int32_t w, int32_t hh) {
    if (h != nullptr) h->canvas.set_clip(x, y, w, hh);
}

void pyxift_canvas_test_clip_reset(PyxiftCanvasHandle *h) {
    if (h != nullptr) h->canvas.reset_clip();
}

void pyxift_canvas_test_camera(PyxiftCanvasHandle *h, int32_t x, int32_t y) {
    if (h != nullptr) h->canvas.set_camera(x, y);
}

void pyxift_canvas_test_camera_reset(PyxiftCanvasHandle *h) {
    if (h != nullptr) h->canvas.reset_camera();
}

void pyxift_canvas_test_pal(PyxiftCanvasHandle *h, uint8_t from, uint8_t to) {
    if (h != nullptr) h->canvas.set_pal(from, to);
}

void pyxift_canvas_test_pal_reset(PyxiftCanvasHandle *h) {
    if (h != nullptr) h->canvas.reset_pal();
}

void pyxift_canvas_test_dither(PyxiftCanvasHandle *h, double alpha) {
    if (h != nullptr) h->canvas.set_dither(alpha);
}

void pyxift_canvas_test_rect(PyxiftCanvasHandle *h, int32_t x, int32_t y, int32_t w, int32_t hh, uint8_t color) {
    if (h != nullptr) h->canvas.rect(x, y, w, hh, color);
}

void pyxift_canvas_test_rectb(PyxiftCanvasHandle *h, int32_t x, int32_t y, int32_t w, int32_t hh, uint8_t color) {
    if (h != nullptr) h->canvas.rectb(x, y, w, hh, color);
}

void pyxift_canvas_test_circ(PyxiftCanvasHandle *h, int32_t x, int32_t y, int32_t r, uint8_t color) {
    if (h != nullptr) h->canvas.circ(x, y, r, color);
}

void pyxift_canvas_test_circb(PyxiftCanvasHandle *h, int32_t x, int32_t y, int32_t r, uint8_t color) {
    if (h != nullptr) h->canvas.circb(x, y, r, color);
}

void pyxift_canvas_test_elli(PyxiftCanvasHandle *h, int32_t x, int32_t y, int32_t w, int32_t hh, uint8_t color) {
    if (h != nullptr) h->canvas.elli(x, y, w, hh, color);
}

void pyxift_canvas_test_ellib(PyxiftCanvasHandle *h, int32_t x, int32_t y, int32_t w, int32_t hh, uint8_t color) {
    if (h != nullptr) h->canvas.ellib(x, y, w, hh, color);
}

void pyxift_canvas_test_line(PyxiftCanvasHandle *h, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t color) {
    if (h != nullptr) h->canvas.line(x1, y1, x2, y2, color);
}

void pyxift_canvas_test_fill(PyxiftCanvasHandle *h, int32_t x, int32_t y, uint8_t color) {
    if (h != nullptr) h->canvas.fill(x, y, color);
}

PyxiftImageHandle *pyxift_image_test_create(void) { return new PyxiftImageHandle{}; }
void pyxift_image_test_destroy(PyxiftImageHandle *h) { delete h; }
void pyxift_image_test_pset(PyxiftImageHandle *h, int32_t x, int32_t y, uint8_t color) {
    if (h != nullptr) h->image.pset(x, y, color);
}

PyxiftTilemapHandle *pyxift_tilemap_test_create(void) { return new PyxiftTilemapHandle{}; }
void pyxift_tilemap_test_destroy(PyxiftTilemapHandle *h) { delete h; }
void pyxift_tilemap_test_set_cell(PyxiftTilemapHandle *h, int32_t cx, int32_t cy,
                                  uint8_t tile_x, uint8_t tile_y) {
    if (h != nullptr) h->tilemap.set_cell(cx, cy, tile_x, tile_y);
}

void pyxift_canvas_test_blt(PyxiftCanvasHandle *h,
                            int32_t x, int32_t y,
                            const PyxiftImageHandle *img,
                            int32_t u, int32_t v, int32_t w, int32_t hh,
                            int32_t transparent,
                            double rotate_deg, double scale) {
    if (h == nullptr || img == nullptr) return;
    h->canvas.blt(x, y, img->image, u, v, w, hh, transparent, rotate_deg, scale);
}

void pyxift_canvas_test_bltm(PyxiftCanvasHandle *h,
                             int32_t x, int32_t y,
                             const PyxiftTilemapHandle *tm,
                             const PyxiftImageHandle *img,
                             int32_t tu, int32_t tv, int32_t tw, int32_t th,
                             int32_t transparent,
                             double rotate_deg, double scale) {
    if (h == nullptr || tm == nullptr || img == nullptr) return;
    h->canvas.bltm(x, y, tm->tilemap, img->image, tu, tv, tw, th, transparent, rotate_deg, scale);
}

} // extern "C"
