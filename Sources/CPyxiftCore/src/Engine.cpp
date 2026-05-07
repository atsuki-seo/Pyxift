#include "Engine.hpp"

#include "core/ImageLoader.hpp"
#include "core/Palette.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <chrono>
#include <thread>

namespace {

using clock_t_ = std::chrono::steady_clock;
using ns_t_ = std::chrono::nanoseconds;

bool ensure_sdl_init() {
    return SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
}

} // namespace

PyxiftEngine::PyxiftEngine(int32_t width, int32_t height, std::string title, int32_t fps)
    : canvas_(width, height),
      window_((ensure_sdl_init(), pyxift::platform::Window(width, height, title))),
      fps_(fps > 0 ? fps : 30) {
    sdl_initialized_ = true;
    event_translator_.set_renderer(window_.renderer());
}

pyxift::Image *PyxiftEngine::image(int32_t bank) {
    if (bank < 0 || bank >= kImageBankCount) return nullptr;
    return &images_[bank];
}

pyxift::Tilemap *PyxiftEngine::tilemap(int32_t index) {
    if (index < 0 || index >= kTilemapCount) return nullptr;
    return &tilemaps_[index];
}

PyxiftEngine::~PyxiftEngine() {
    if (sdl_initialized_) {
        SDL_Quit();
    }
}

void PyxiftEngine::set_title(const std::string &title) {
    window_.set_title(title);
}

void PyxiftEngine::quit() {
    quit_requested_ = true;
}

void PyxiftEngine::pump_events() {
    input_.end_frame();

    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (event_translator_.translate(ev, input_)) {
            quit_requested_ = true;
        }
    }
}

void PyxiftEngine::run(PyxiftUpdateFn update, PyxiftDrawFn draw, void *user) {
    if (!window_.valid()) {
        return;
    }

    const ns_t_ frame_duration = ns_t_(1'000'000'000LL / fps_);
    auto next_frame = clock_t_::now();
    constexpr int kMaxCatchUp = 2;

    while (!quit_requested_) {
        const auto now = clock_t_::now();

        int catch_up = 0;
        while (now >= next_frame && catch_up < kMaxCatchUp && !quit_requested_) {
            pump_events();
            if (quit_requested_) break;
            if (update != nullptr) update(user);
            ++frame_count_;
            next_frame += frame_duration;
            ++catch_up;
        }

        // 一時停止やブレーク後の連続フレーム消化を防ぐため、kMaxCatchUp を超えた遅延は
        // 補正せず next_frame を現在時刻に揃えてリスタートする。
        if (catch_up == kMaxCatchUp && clock_t_::now() >= next_frame) {
            next_frame = clock_t_::now() + frame_duration;
        }

        if (quit_requested_) break;

        if (draw != nullptr) draw(user);
        window_.present(canvas_.pixels(), pyxift::kDefaultPalette.data());

        const auto sleep_until = next_frame;
        const auto t = clock_t_::now();
        if (t < sleep_until) {
            std::this_thread::sleep_for(sleep_until - t);
        }
    }
}

extern "C" {

PyxiftEngine *pyxift_engine_create(int32_t width, int32_t height, const char *title, int32_t fps) {
    try {
        return new PyxiftEngine(width, height, title != nullptr ? title : "Pyxift", fps);
    } catch (...) {
        return nullptr;
    }
}

void pyxift_engine_destroy(PyxiftEngine *engine) {
    delete engine;
}

void pyxift_engine_run(PyxiftEngine *engine, PyxiftUpdateFn update, PyxiftDrawFn draw, void *user) {
    if (engine != nullptr) engine->run(update, draw, user);
}

void pyxift_engine_quit(PyxiftEngine *engine) {
    if (engine != nullptr) engine->quit();
}

void pyxift_engine_set_title(PyxiftEngine *engine, const char *title) {
    if (engine != nullptr && title != nullptr) engine->set_title(title);
}

int32_t pyxift_engine_width(const PyxiftEngine *engine) {
    return engine != nullptr ? engine->width() : 0;
}

int32_t pyxift_engine_height(const PyxiftEngine *engine) {
    return engine != nullptr ? engine->height() : 0;
}

int32_t pyxift_engine_frame_count(const PyxiftEngine *engine) {
    return engine != nullptr ? engine->frame_count() : 0;
}

void pyxift_engine_cls(PyxiftEngine *engine, uint8_t color) {
    if (engine != nullptr) engine->canvas().cls(color);
}

void pyxift_engine_pset(PyxiftEngine *engine, int32_t x, int32_t y, uint8_t color) {
    if (engine != nullptr) engine->canvas().pset(x, y, color);
}

uint8_t pyxift_engine_pget(const PyxiftEngine *engine, int32_t x, int32_t y) {
    return engine != nullptr ? engine->canvas().pget(x, y) : 0;
}

void pyxift_engine_line(PyxiftEngine *engine,
                        int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                        uint8_t color) {
    if (engine != nullptr) engine->canvas().line(x1, y1, x2, y2, color);
}

void pyxift_engine_rect(PyxiftEngine *engine,
                        int32_t x, int32_t y, int32_t w, int32_t h,
                        uint8_t color) {
    if (engine != nullptr) engine->canvas().rect(x, y, w, h, color);
}

void pyxift_engine_rectb(PyxiftEngine *engine,
                         int32_t x, int32_t y, int32_t w, int32_t h,
                         uint8_t color) {
    if (engine != nullptr) engine->canvas().rectb(x, y, w, h, color);
}

void pyxift_engine_circ(PyxiftEngine *engine,
                        int32_t x, int32_t y, int32_t r,
                        uint8_t color) {
    if (engine != nullptr) engine->canvas().circ(x, y, r, color);
}

void pyxift_engine_circb(PyxiftEngine *engine,
                         int32_t x, int32_t y, int32_t r,
                         uint8_t color) {
    if (engine != nullptr) engine->canvas().circb(x, y, r, color);
}

void pyxift_engine_tri(PyxiftEngine *engine,
                       int32_t x1, int32_t y1,
                       int32_t x2, int32_t y2,
                       int32_t x3, int32_t y3,
                       uint8_t color) {
    if (engine != nullptr) engine->canvas().tri(x1, y1, x2, y2, x3, y3, color);
}

void pyxift_engine_trib(PyxiftEngine *engine,
                        int32_t x1, int32_t y1,
                        int32_t x2, int32_t y2,
                        int32_t x3, int32_t y3,
                        uint8_t color) {
    if (engine != nullptr) engine->canvas().trib(x1, y1, x2, y2, x3, y3, color);
}

void pyxift_engine_clip(PyxiftEngine *engine,
                        int32_t x, int32_t y, int32_t w, int32_t h) {
    if (engine != nullptr) engine->canvas().set_clip(x, y, w, h);
}

void pyxift_engine_clip_reset(PyxiftEngine *engine) {
    if (engine != nullptr) engine->canvas().reset_clip();
}

void pyxift_engine_camera(PyxiftEngine *engine, int32_t x, int32_t y) {
    if (engine != nullptr) engine->canvas().set_camera(x, y);
}

void pyxift_engine_camera_reset(PyxiftEngine *engine) {
    if (engine != nullptr) engine->canvas().reset_camera();
}

void pyxift_engine_pal(PyxiftEngine *engine, uint8_t from, uint8_t to) {
    if (engine != nullptr) engine->canvas().set_pal(from, to);
}

void pyxift_engine_pal_reset(PyxiftEngine *engine) {
    if (engine != nullptr) engine->canvas().reset_pal();
}

void pyxift_engine_blt(PyxiftEngine *engine,
                       int32_t x, int32_t y,
                       int32_t image_bank,
                       int32_t u, int32_t v, int32_t w, int32_t h,
                       int32_t transparent) {
    if (engine == nullptr) return;
    auto *img = engine->image(image_bank);
    if (img == nullptr) return;
    engine->canvas().blt(x, y, *img, u, v, w, h, transparent);
}

void pyxift_engine_bltm(PyxiftEngine *engine,
                        int32_t x, int32_t y,
                        int32_t tilemap_index,
                        int32_t u, int32_t v, int32_t w, int32_t h,
                        int32_t transparent) {
    if (engine == nullptr) return;
    auto *tm = engine->tilemap(tilemap_index);
    if (tm == nullptr) return;
    auto *img = engine->image(tm->image_bank());
    if (img == nullptr) return;
    engine->canvas().bltm(x, y, *tm, *img, u, v, w, h, transparent);
}

void pyxift_engine_text(PyxiftEngine *engine,
                        int32_t x, int32_t y, const char *s, uint8_t color) {
    if (engine != nullptr && s != nullptr) engine->canvas().text(x, y, s, color);
}

void pyxift_engine_image_pset(PyxiftEngine *engine,
                              int32_t image_bank,
                              int32_t x, int32_t y, uint8_t color) {
    if (engine == nullptr) return;
    auto *img = engine->image(image_bank);
    if (img == nullptr) return;
    img->pset(x, y, color);
}

bool pyxift_engine_load_image(PyxiftEngine *engine,
                              int32_t image_bank,
                              const char *path) {
    if (engine == nullptr || path == nullptr) return false;
    auto *img = engine->image(image_bank);
    if (img == nullptr) return false;
    return pyxift::load_png_into_image(std::string(path), *img);
}

void pyxift_engine_tilemap_set(PyxiftEngine *engine,
                               int32_t tilemap_index,
                               int32_t cx, int32_t cy,
                               uint8_t tile_x, uint8_t tile_y) {
    if (engine == nullptr) return;
    auto *tm = engine->tilemap(tilemap_index);
    if (tm == nullptr) return;
    tm->set_cell(cx, cy, tile_x, tile_y);
}

void pyxift_engine_tilemap_set_image_bank(PyxiftEngine *engine,
                                          int32_t tilemap_index,
                                          int32_t image_bank) {
    if (engine == nullptr) return;
    auto *tm = engine->tilemap(tilemap_index);
    if (tm == nullptr) return;
    tm->set_image_bank(image_bank);
}

bool pyxift_engine_button(const PyxiftEngine *engine, uint8_t button, int32_t player) {
    if (engine == nullptr) return false;
    return engine->input().button(static_cast<pyxift::Button>(button), player);
}

bool pyxift_engine_button_pressed(const PyxiftEngine *engine, uint8_t button, int32_t player) {
    if (engine == nullptr) return false;
    return engine->input().button_pressed(static_cast<pyxift::Button>(button), player);
}

bool pyxift_engine_button_released(const PyxiftEngine *engine, uint8_t button, int32_t player) {
    if (engine == nullptr) return false;
    return engine->input().button_released(static_cast<pyxift::Button>(button), player);
}

bool pyxift_engine_key(const PyxiftEngine *engine, int32_t keycode) {
    if (engine == nullptr) return false;
    return engine->input().key(keycode);
}

bool pyxift_engine_key_pressed(const PyxiftEngine *engine, int32_t keycode) {
    if (engine == nullptr) return false;
    return engine->input().key_pressed(keycode);
}

bool pyxift_engine_key_released(const PyxiftEngine *engine, int32_t keycode) {
    if (engine == nullptr) return false;
    return engine->input().key_released(keycode);
}

void pyxift_engine_mouse(const PyxiftEngine *engine, int32_t *out_x, int32_t *out_y) {
    if (engine == nullptr) {
        if (out_x != nullptr) *out_x = 0;
        if (out_y != nullptr) *out_y = 0;
        return;
    }
    if (out_x != nullptr) *out_x = engine->input().mouse_x();
    if (out_y != nullptr) *out_y = engine->input().mouse_y();
}

int32_t pyxift_engine_mouse_wheel(const PyxiftEngine *engine) {
    return engine != nullptr ? engine->input().mouse_wheel() : 0;
}

bool pyxift_engine_mouse_button(const PyxiftEngine *engine, uint8_t button) {
    if (engine == nullptr) return false;
    return engine->input().mouse_button(static_cast<pyxift::MouseButton>(button));
}

bool pyxift_engine_mouse_button_pressed(const PyxiftEngine *engine, uint8_t button) {
    if (engine == nullptr) return false;
    return engine->input().mouse_button_pressed(static_cast<pyxift::MouseButton>(button));
}

bool pyxift_engine_mouse_button_released(const PyxiftEngine *engine, uint8_t button) {
    if (engine == nullptr) return false;
    return engine->input().mouse_button_released(static_cast<pyxift::MouseButton>(button));
}

void pyxift_engine_mouse_cursor(PyxiftEngine *engine, bool visible) {
    (void)engine;
    pyxift::platform::Window::set_cursor_visible(visible);
}

} // extern "C"
