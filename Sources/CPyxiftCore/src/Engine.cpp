#include "Engine.hpp"

#include "core/Palette.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <chrono>
#include <thread>

namespace {

using clock_t_ = std::chrono::steady_clock;
using ns_t_ = std::chrono::nanoseconds;

bool ensure_sdl_init() {
    return SDL_Init(SDL_INIT_VIDEO);
}

} // namespace

PyxiftEngine::PyxiftEngine(int32_t width, int32_t height, std::string title, int32_t fps)
    : canvas_(width, height),
      window_((ensure_sdl_init(), pyxift::platform::Window(width, height, title))),
      fps_(fps > 0 ? fps : 30) {
    sdl_initialized_ = true;
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
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_EVENT_QUIT:
                quit_requested_ = true;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (ev.key.key == SDLK_ESCAPE) {
                    quit_requested_ = true;
                }
                break;
            default:
                break;
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

        // 大きく遅延した場合は追従を諦めて next_frame を現在に揃える
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

// ---- C API ----

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

} // extern "C"
