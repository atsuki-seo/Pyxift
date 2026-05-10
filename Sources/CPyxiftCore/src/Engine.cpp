#include "Engine.hpp"

#include "core/AssetBundle.hpp"
#include "core/Font.hpp"
#include "core/ImageLoader.hpp"
#include "core/Palette.hpp"
#include "core/PngWriter.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <memory>
#include <thread>

namespace {

using clock_t_ = std::chrono::steady_clock;
using ns_t_ = std::chrono::nanoseconds;

bool ensure_sdl_init() {
    return SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO);
}

} // namespace

PyxiftEngine::PyxiftEngine(int32_t width, int32_t height, std::string title, int32_t fps)
    : canvas_(width, height),
      window_((ensure_sdl_init(), pyxift::platform::Window(width, height, title))),
      fps_(fps > 0 ? fps : 30) {
    sdl_initialized_ = true;
    event_translator_.set_renderer(window_.renderer());
    audio_output_.open(&audio_mixer_);
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
    audio_output_.close();
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

void PyxiftEngine::set_mouse_pos(int32_t x, int32_t y) {
    input_.set_mouse_pos(x, y);
    window_.warp_mouse(x, y);
}

void PyxiftEngine::set_fullscreen(bool enabled) {
    window_.set_fullscreen(enabled);
}

void PyxiftEngine::resize(int32_t width, int32_t height) {
    if (width <= 0 || height <= 0) return;
    canvas_.resize(width, height);
    window_.resize_logical(width, height);
    // resize_logical recreates the texture and resets it to NEAREST, so smooth/retro modes
    // would silently revert without re-applying the filter here.
    window_.set_linear_filtering(screen_mode_ != 0);
}

void PyxiftEngine::set_screen_mode(int32_t mode) {
    if (mode < 0 || mode > 2) return;
    screen_mode_ = mode;
    // Pyxift's SDL_Renderer backend lacks the GLSL shader pipeline that upstream uses for the
    // smooth/retro modes; mode 1 falls back to a linear texture filter as a coarse approximation,
    // mode 2 (CRT) currently aliases to mode 1 until a shader path is introduced.
    window_.set_linear_filtering(mode != 0);
}

void PyxiftEngine::set_integer_scale(bool enabled) {
    window_.set_integer_scale(enabled);
}

bool PyxiftEngine::screenshot(const std::string &path, int32_t scale) {
    if (scale < 1) scale = 1;
    return pyxift::save_indexed_png(path,
                                    canvas_.pixels(),
                                    canvas_.width(),
                                    canvas_.height(),
                                    pyxift::kDefaultPalette.data(),
                                    scale);
}

void PyxiftEngine::show() {
    show_mode_ = true;
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

namespace {

void draw_perf_overlay(pyxift::Canvas &canvas, double fps, double upd_ms, double draw_ms) {
    const auto saved = canvas.save_state();
    canvas.reset_clip();
    canvas.reset_camera();
    canvas.reset_pal();
    canvas.set_dither(1.0);

    char line[32];
    std::snprintf(line, sizeof(line), "%.2f", fps);
    canvas.text(1, 0, line, 7);
    std::snprintf(line, sizeof(line), "%.2f", upd_ms);
    canvas.text(1, 6, line, 7);
    std::snprintf(line, sizeof(line), "%.2f", draw_ms);
    canvas.text(1, 12, line, 7);

    canvas.restore_state(saved);
}

double ms_between(clock_t_::time_point a, clock_t_::time_point b) {
    return std::chrono::duration<double, std::milli>(b - a).count();
}

} // namespace

void PyxiftEngine::run(PyxiftUpdateFn update, PyxiftDrawFn draw, void *user) {
    if (!window_.valid()) {
        return;
    }

    const ns_t_ frame_duration = ns_t_(1'000'000'000LL / fps_);
    auto next_frame = clock_t_::now();
    auto last_present = clock_t_::now();
    constexpr int kMaxCatchUp = 2;
    constexpr double kEmaAlpha = 0.1;

    while (!quit_requested_) {
        const auto now = clock_t_::now();

        int catch_up = 0;
        double last_update_ms = avg_update_ms_;
        while (now >= next_frame && catch_up < kMaxCatchUp && !quit_requested_) {
            pump_events();
            if (quit_requested_) break;

            const auto upd_start = clock_t_::now();
            if (update != nullptr && !show_mode_) update(user);
            const auto upd_end = clock_t_::now();
            last_update_ms = ms_between(upd_start, upd_end);

            ++frame_count_;
            next_frame += frame_duration;
            ++catch_up;
        }

        // After a pause or breakpoint, do not try to catch up beyond kMaxCatchUp frames; resync
        // next_frame to "now" so the loop does not burn through a backlog of pending frames.
        if (catch_up == kMaxCatchUp && clock_t_::now() >= next_frame) {
            next_frame = clock_t_::now() + frame_duration;
        }

        if (quit_requested_) break;

        const auto draw_start = clock_t_::now();
        if (draw != nullptr && !show_mode_) draw(user);
        const auto draw_end = clock_t_::now();
        const double last_draw_ms = ms_between(draw_start, draw_end);

        if (catch_up > 0) {
            avg_update_ms_ = avg_update_ms_ * (1.0 - kEmaAlpha) + last_update_ms * kEmaAlpha;
        }
        avg_draw_ms_ = avg_draw_ms_ * (1.0 - kEmaAlpha) + last_draw_ms * kEmaAlpha;

        const auto present_now = clock_t_::now();
        const double inst_fps = 1000.0 / std::max(1e-3, ms_between(last_present, present_now));
        avg_fps_ = avg_fps_ * (1.0 - kEmaAlpha) + inst_fps * kEmaAlpha;
        last_present = present_now;

        if (perf_monitor_enabled_) {
            draw_perf_overlay(canvas_, avg_fps_, avg_update_ms_, avg_draw_ms_);
        }

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

uint32_t pyxift_default_palette(uint8_t index) {
    return pyxift::kDefaultPalette[index & 0x0f];
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
                       int32_t transparent,
                       double rotate_deg, double scale) {
    if (engine == nullptr) return;
    auto *img = engine->image(image_bank);
    if (img == nullptr) return;
    engine->canvas().blt(x, y, *img, u, v, w, h, transparent, rotate_deg, scale);
}

void pyxift_engine_bltm(PyxiftEngine *engine,
                        int32_t x, int32_t y,
                        int32_t tilemap_index,
                        int32_t u, int32_t v, int32_t w, int32_t h,
                        int32_t transparent,
                        double rotate_deg, double scale) {
    if (engine == nullptr) return;
    auto *tm = engine->tilemap(tilemap_index);
    if (tm == nullptr) return;
    auto *img = engine->image(tm->image_bank());
    if (img == nullptr) return;
    engine->canvas().bltm(x, y, *tm, *img, u, v, w, h, transparent, rotate_deg, scale);
}

void pyxift_engine_elli(PyxiftEngine *engine,
                        int32_t x, int32_t y, int32_t w, int32_t h,
                        uint8_t color) {
    if (engine != nullptr) engine->canvas().elli(x, y, w, h, color);
}

void pyxift_engine_ellib(PyxiftEngine *engine,
                         int32_t x, int32_t y, int32_t w, int32_t h,
                         uint8_t color) {
    if (engine != nullptr) engine->canvas().ellib(x, y, w, h, color);
}

void pyxift_engine_fill(PyxiftEngine *engine,
                        int32_t x, int32_t y, uint8_t color) {
    if (engine != nullptr) engine->canvas().fill(x, y, color);
}

void pyxift_engine_dither(PyxiftEngine *engine, double alpha) {
    if (engine != nullptr) engine->canvas().set_dither(alpha);
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

bool pyxift_engine_button_pressed(const PyxiftEngine *engine, uint8_t button, int32_t player,
                                  int32_t hold, int32_t repeat) {
    if (engine == nullptr) return false;
    return engine->input().button_pressed(static_cast<pyxift::Button>(button), player, hold, repeat);
}

bool pyxift_engine_button_released(const PyxiftEngine *engine, uint8_t button, int32_t player) {
    if (engine == nullptr) return false;
    return engine->input().button_released(static_cast<pyxift::Button>(button), player);
}

bool pyxift_engine_key(const PyxiftEngine *engine, int32_t keycode) {
    if (engine == nullptr) return false;
    return engine->input().key(keycode);
}

bool pyxift_engine_key_pressed(const PyxiftEngine *engine, int32_t keycode,
                               int32_t hold, int32_t repeat) {
    if (engine == nullptr) return false;
    return engine->input().key_pressed(keycode, hold, repeat);
}

bool pyxift_engine_key_released(const PyxiftEngine *engine, int32_t keycode) {
    if (engine == nullptr) return false;
    return engine->input().key_released(keycode);
}

float pyxift_engine_gamepad_axis(const PyxiftEngine *engine, int32_t player, uint8_t axis) {
    if (engine == nullptr) return 0.0f;
    return engine->input().gamepad_axis(player, static_cast<pyxift::GamepadAxis>(axis));
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

void pyxift_engine_set_fullscreen(PyxiftEngine *engine, bool enabled) {
    if (engine != nullptr) engine->set_fullscreen(enabled);
}

bool pyxift_engine_fullscreen(const PyxiftEngine *engine) {
    return engine != nullptr ? engine->fullscreen() : false;
}

void pyxift_engine_resize(PyxiftEngine *engine, int32_t width, int32_t height) {
    if (engine != nullptr) engine->resize(width, height);
}

void pyxift_engine_set_screen_mode(PyxiftEngine *engine, int32_t mode) {
    if (engine != nullptr) engine->set_screen_mode(mode);
}

int32_t pyxift_engine_screen_mode(const PyxiftEngine *engine) {
    return engine != nullptr ? engine->screen_mode() : 0;
}

void pyxift_engine_set_integer_scale(PyxiftEngine *engine, bool enabled) {
    if (engine != nullptr) engine->set_integer_scale(enabled);
}

bool pyxift_engine_integer_scale(const PyxiftEngine *engine) {
    return engine != nullptr ? engine->integer_scale() : false;
}

void pyxift_engine_set_perf_monitor(PyxiftEngine *engine, bool enabled) {
    if (engine != nullptr) engine->set_perf_monitor(enabled);
}

bool pyxift_engine_perf_monitor(const PyxiftEngine *engine) {
    return engine != nullptr ? engine->perf_monitor() : false;
}

void pyxift_engine_set_icon(PyxiftEngine *engine,
                            const uint8_t *rgba, int32_t width, int32_t height) {
    if (engine != nullptr) engine->set_icon(rgba, width, height);
}

bool pyxift_engine_screenshot(PyxiftEngine *engine, const char *path, int32_t scale) {
    if (engine == nullptr || path == nullptr) return false;
    return engine->screenshot(std::string(path), scale);
}

void pyxift_engine_show(PyxiftEngine *engine) {
    if (engine != nullptr) engine->show();
}

void pyxift_engine_set_mouse_pos(PyxiftEngine *engine, int32_t x, int32_t y) {
    if (engine != nullptr) engine->set_mouse_pos(x, y);
}

static int32_t copy_string_to_buf(const std::string &s, char *buf, int32_t buf_size) {
    const int32_t len = static_cast<int32_t>(s.size());
    if (buf != nullptr && buf_size > 0) {
        const int32_t copy = len < buf_size - 1 ? len : buf_size - 1;
        std::memcpy(buf, s.data(), static_cast<size_t>(copy));
        buf[copy] = '\0';
    }
    return len;
}

int32_t pyxift_engine_input_text(const PyxiftEngine *engine, char *buf, int32_t buf_size) {
    if (engine == nullptr) {
        if (buf != nullptr && buf_size > 0) buf[0] = '\0';
        return 0;
    }
    return copy_string_to_buf(engine->input().input_text(), buf, buf_size);
}

int32_t pyxift_engine_input_keys_count(const PyxiftEngine *engine) {
    if (engine == nullptr) return 0;
    return static_cast<int32_t>(engine->input().input_keys().size());
}

int32_t pyxift_engine_input_keys_at(const PyxiftEngine *engine, int32_t index) {
    if (engine == nullptr) return 0;
    const auto &keys = engine->input().input_keys();
    if (index < 0 || static_cast<size_t>(index) >= keys.size()) return 0;
    return keys[static_cast<size_t>(index)];
}

int32_t pyxift_engine_dropped_files_count(const PyxiftEngine *engine) {
    if (engine == nullptr) return 0;
    return static_cast<int32_t>(engine->input().dropped_files().size());
}

int32_t pyxift_engine_dropped_files_at(const PyxiftEngine *engine, int32_t index,
                                       char *buf, int32_t buf_size) {
    if (engine == nullptr) {
        if (buf != nullptr && buf_size > 0) buf[0] = '\0';
        return -1;
    }
    const auto &files = engine->input().dropped_files();
    if (index < 0 || static_cast<size_t>(index) >= files.size()) {
        if (buf != nullptr && buf_size > 0) buf[0] = '\0';
        return -1;
    }
    return copy_string_to_buf(files[static_cast<size_t>(index)], buf, buf_size);
}

void pyxift_engine_sound_set(PyxiftEngine *engine,
                             int32_t sound_index,
                             const char *notes,
                             const char *tones,
                             const char *volumes,
                             const char *effects,
                             int32_t speed) {
    if (engine == nullptr) return;
    pyxift::Sound s;
    s.set(notes != nullptr ? notes : "",
          tones != nullptr ? tones : "",
          volumes != nullptr ? volumes : "",
          effects != nullptr ? effects : "",
          speed);
    engine->audio_mixer().set_sound(sound_index, s);
}

void pyxift_engine_play(PyxiftEngine *engine,
                        int32_t channel,
                        int32_t sound_index,
                        bool loop) {
    if (engine == nullptr) return;
    engine->audio_mixer().play(channel, sound_index, loop);
}

void pyxift_engine_stop(PyxiftEngine *engine, int32_t channel) {
    if (engine == nullptr) return;
    engine->audio_mixer().stop(channel);
}

void pyxift_engine_stop_all(PyxiftEngine *engine) {
    if (engine == nullptr) return;
    engine->audio_mixer().stop_all();
}

bool pyxift_engine_is_playing(const PyxiftEngine *engine, int32_t channel) {
    if (engine == nullptr) return false;
    return engine->audio_mixer().is_playing(channel);
}

static std::vector<int32_t> engine_to_vec(const int32_t *p, int32_t n) {
    if (p == nullptr || n <= 0) return {};
    return std::vector<int32_t>(p, p + n);
}

void pyxift_engine_music_set(PyxiftEngine *engine,
                             int32_t music_index,
                             const int32_t *ch0, int32_t ch0_len,
                             const int32_t *ch1, int32_t ch1_len,
                             const int32_t *ch2, int32_t ch2_len,
                             const int32_t *ch3, int32_t ch3_len) {
    if (engine == nullptr) return;
    pyxift::Music m;
    m.set(engine_to_vec(ch0, ch0_len),
          engine_to_vec(ch1, ch1_len),
          engine_to_vec(ch2, ch2_len),
          engine_to_vec(ch3, ch3_len));
    engine->audio_mixer().set_music(music_index, m);
}

void pyxift_engine_play_music(PyxiftEngine *engine,
                              int32_t music_index,
                              bool loop) {
    if (engine == nullptr) return;
    engine->audio_mixer().play_music(music_index, loop);
}

bool pyxift_engine_play_pos(const PyxiftEngine *engine,
                            int32_t channel,
                            int32_t *out_sound_index,
                            float *out_sec) {
    if (engine == nullptr) return false;
    auto pos = engine->audio_mixer().play_pos(channel);
    if (!pos) return false;
    if (out_sound_index != nullptr) *out_sound_index = pos->first;
    if (out_sec != nullptr) *out_sec = pos->second;
    return true;
}

static pyxift::AssetBundleSlots make_slots(PyxiftEngine *engine) {
    pyxift::AssetBundleSlots slots;
    slots.images = engine->images_data();
    slots.image_count = PyxiftEngine::kImageBankCount;
    slots.tilemaps = engine->tilemaps_data();
    slots.tilemap_count = PyxiftEngine::kTilemapCount;
    slots.audio_mixer = &engine->audio_mixer();
    return slots;
}

bool pyxift_engine_load_bundle(PyxiftEngine *engine,
                               const char *path,
                               bool exclude_images,
                               bool exclude_tilemaps,
                               bool exclude_sounds,
                               bool exclude_musics) {
    if (engine == nullptr || path == nullptr) return false;
    pyxift::AssetBundleOptions opts;
    opts.exclude_images = exclude_images;
    opts.exclude_tilemaps = exclude_tilemaps;
    opts.exclude_sounds = exclude_sounds;
    opts.exclude_musics = exclude_musics;
    return pyxift::load_asset_bundle(std::string(path), make_slots(engine), opts);
}

bool pyxift_engine_save_bundle(PyxiftEngine *engine,
                               const char *path,
                               bool exclude_images,
                               bool exclude_tilemaps,
                               bool exclude_sounds,
                               bool exclude_musics) {
    if (engine == nullptr || path == nullptr) return false;
    pyxift::AssetBundleOptions opts;
    opts.exclude_images = exclude_images;
    opts.exclude_tilemaps = exclude_tilemaps;
    opts.exclude_sounds = exclude_sounds;
    opts.exclude_musics = exclude_musics;
    return pyxift::save_asset_bundle(std::string(path), make_slots(engine), opts);
}

namespace {

constexpr uint8_t kImageSentinel = 14;
constexpr uint8_t kTileSentinelX = 200;
constexpr uint8_t kTileSentinelY = 201;
constexpr int32_t kImgsrcSentinel = -42;
constexpr int8_t kNoteSentinel = -100;
constexpr int32_t kSpeedSentinel = -7;
constexpr int32_t kMusicSentinel = -55;

int32_t bundle_roundtrip_impl(const char *tmp_path,
                              int32_t img_x, int32_t img_y, uint8_t img_color,
                              int32_t cx, int32_t cy,
                              uint8_t tx, uint8_t ty, int32_t imgsrc,
                              int8_t note, int32_t speed,
                              int32_t music_value,
                              const pyxift::AssetBundleOptions &save_opts,
                              const pyxift::AssetBundleOptions &load_opts,
                              uint8_t *out_color,
                              uint8_t *out_tx, uint8_t *out_ty,
                              int32_t *out_imgsrc,
                              int8_t *out_note, int32_t *out_speed,
                              int32_t *out_music_value) {
    if (tmp_path == nullptr) return 0;

    pyxift::Image src_img;
    src_img.pset(img_x, img_y, img_color);
    pyxift::Tilemap src_tm;
    src_tm.set_image_bank(imgsrc);
    src_tm.set_cell(cx, cy, tx, ty);
    auto src_mixer = std::make_unique<pyxift::AudioMixer>();
    pyxift::Sound s;
    s.notes.push_back(note);
    s.speed = speed;
    src_mixer->set_sound(0, s);
    pyxift::Music m;
    m.seqs[0].push_back(music_value);
    src_mixer->set_music(0, m);

    pyxift::AssetBundleSlots src_slots;
    src_slots.images = &src_img;
    src_slots.image_count = 1;
    src_slots.tilemaps = &src_tm;
    src_slots.tilemap_count = 1;
    src_slots.audio_mixer = src_mixer.get();
    if (!pyxift::save_asset_bundle(std::string(tmp_path), src_slots, save_opts)) return 0;

    pyxift::Image dst_img;
    dst_img.pset(img_x, img_y, kImageSentinel);
    pyxift::Tilemap dst_tm;
    dst_tm.set_image_bank(kImgsrcSentinel);
    dst_tm.set_cell(cx, cy, kTileSentinelX, kTileSentinelY);
    auto dst_mixer = std::make_unique<pyxift::AudioMixer>();
    pyxift::Sound dst_sound;
    dst_sound.notes.push_back(kNoteSentinel);
    dst_sound.speed = kSpeedSentinel;
    dst_mixer->set_sound(0, dst_sound);
    pyxift::Music dst_music;
    dst_music.seqs[0].push_back(kMusicSentinel);
    dst_mixer->set_music(0, dst_music);

    pyxift::AssetBundleSlots dst_slots;
    dst_slots.images = &dst_img;
    dst_slots.image_count = 1;
    dst_slots.tilemaps = &dst_tm;
    dst_slots.tilemap_count = 1;
    dst_slots.audio_mixer = dst_mixer.get();
    if (!pyxift::load_asset_bundle(std::string(tmp_path), dst_slots, load_opts)) return 0;

    if (out_color != nullptr) *out_color = dst_img.pget(img_x, img_y);
    uint8_t rt = 0, ry = 0;
    dst_tm.get_cell(cx, cy, rt, ry);
    if (out_tx != nullptr) *out_tx = rt;
    if (out_ty != nullptr) *out_ty = ry;
    if (out_imgsrc != nullptr) *out_imgsrc = dst_tm.image_bank();
    auto rs = dst_mixer->get_sound(0);
    if (out_note != nullptr) *out_note = rs.notes.empty() ? -127 : rs.notes[0];
    if (out_speed != nullptr) *out_speed = rs.speed;
    auto rm = dst_mixer->get_music(0);
    if (out_music_value != nullptr) *out_music_value = rm.seqs[0].empty() ? -1 : rm.seqs[0][0];
    return 1;
}

} // namespace

int32_t pyxift_test_bundle_roundtrip(const char *tmp_path,
                                     int32_t img_x, int32_t img_y, uint8_t img_color,
                                     int32_t cx, int32_t cy,
                                     uint8_t tx, uint8_t ty, int32_t imgsrc,
                                     int8_t note, int32_t speed,
                                     int32_t music_value,
                                     bool exclude_images,
                                     bool exclude_tilemaps,
                                     bool exclude_sounds,
                                     bool exclude_musics,
                                     uint8_t *out_color,
                                     uint8_t *out_tx, uint8_t *out_ty,
                                     int32_t *out_imgsrc,
                                     int8_t *out_note, int32_t *out_speed,
                                     int32_t *out_music_value) {
    pyxift::AssetBundleOptions load_opts;
    load_opts.exclude_images = exclude_images;
    load_opts.exclude_tilemaps = exclude_tilemaps;
    load_opts.exclude_sounds = exclude_sounds;
    load_opts.exclude_musics = exclude_musics;
    return bundle_roundtrip_impl(tmp_path, img_x, img_y, img_color,
                                 cx, cy, tx, ty, imgsrc, note, speed, music_value,
                                 pyxift::AssetBundleOptions{}, load_opts,
                                 out_color, out_tx, out_ty, out_imgsrc,
                                 out_note, out_speed, out_music_value);
}

int32_t pyxift_test_bundle_save_exclude_roundtrip(const char *tmp_path,
                                                  int32_t img_x, int32_t img_y, uint8_t img_color,
                                                  int32_t cx, int32_t cy,
                                                  uint8_t tx, uint8_t ty, int32_t imgsrc,
                                                  int8_t note, int32_t speed,
                                                  int32_t music_value,
                                                  bool exclude_images,
                                                  bool exclude_tilemaps,
                                                  bool exclude_sounds,
                                                  bool exclude_musics,
                                                  uint8_t *out_color,
                                                  uint8_t *out_tx, uint8_t *out_ty,
                                                  int32_t *out_imgsrc,
                                                  int8_t *out_note, int32_t *out_speed,
                                                  int32_t *out_music_value) {
    pyxift::AssetBundleOptions save_opts;
    save_opts.exclude_images = exclude_images;
    save_opts.exclude_tilemaps = exclude_tilemaps;
    save_opts.exclude_sounds = exclude_sounds;
    save_opts.exclude_musics = exclude_musics;
    return bundle_roundtrip_impl(tmp_path, img_x, img_y, img_color,
                                 cx, cy, tx, ty, imgsrc, note, speed, music_value,
                                 save_opts, pyxift::AssetBundleOptions{},
                                 out_color, out_tx, out_ty, out_imgsrc,
                                 out_note, out_speed, out_music_value);
}

} // extern "C"
