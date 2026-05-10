#ifndef PYXIFT_ENGINE_HPP
#define PYXIFT_ENGINE_HPP

#include "core/AudioMixer.hpp"
#include "core/Canvas.hpp"
#include "core/Image.hpp"
#include "core/Input.hpp"
#include "core/Tilemap.hpp"
#include "platform/AudioOutput.hpp"
#include "platform/EventTranslator.hpp"
#include "platform/Window.hpp"

#include "pyxift_c.h"

#include <array>
#include <cstdint>
#include <memory>
#include <string>

struct PyxiftEngine {
    static constexpr int32_t kImageBankCount = 3;
    static constexpr int32_t kTilemapCount = 8;

    PyxiftEngine(int32_t width, int32_t height, std::string title, int32_t fps);
    ~PyxiftEngine();

    void run(PyxiftUpdateFn update, PyxiftDrawFn draw, void *user);
    void quit();
    void set_title(const std::string &title);
    void set_mouse_pos(int32_t x, int32_t y);

    void set_fullscreen(bool enabled);
    bool fullscreen() const { return window_.fullscreen(); }

    void resize(int32_t width, int32_t height);

    void set_screen_mode(int32_t mode);
    int32_t screen_mode() const { return screen_mode_; }

    void set_integer_scale(bool enabled);
    bool integer_scale() const { return window_.integer_scale(); }

    void set_perf_monitor(bool enabled) { perf_monitor_enabled_ = enabled; }
    bool perf_monitor() const { return perf_monitor_enabled_; }

    void set_icon(const uint8_t *rgba, int32_t w, int32_t h) { window_.set_icon(rgba, w, h); }

    bool screenshot(const std::string &path, int32_t scale);

    void show();

    int32_t width() const { return canvas_.width(); }
    int32_t height() const { return canvas_.height(); }
    int32_t frame_count() const { return frame_count_; }

    pyxift::Canvas &canvas() { return canvas_; }
    const pyxift::Canvas &canvas() const { return canvas_; }

    pyxift::Image *image(int32_t bank);
    pyxift::Tilemap *tilemap(int32_t index);

    pyxift::Image *images_data() { return images_.data(); }
    pyxift::Tilemap *tilemaps_data() { return tilemaps_.data(); }

    pyxift::InputState &input() { return input_; }
    const pyxift::InputState &input() const { return input_; }

    pyxift::AudioMixer &audio_mixer() { return audio_mixer_; }
    const pyxift::AudioMixer &audio_mixer() const { return audio_mixer_; }

private:
    pyxift::Canvas canvas_;
    pyxift::platform::Window window_;
    pyxift::InputState input_{};
    pyxift::platform::EventTranslator event_translator_{};
    pyxift::AudioMixer audio_mixer_{};
    pyxift::platform::AudioOutput audio_output_{};
    int32_t fps_;
    int32_t frame_count_ = 0;
    bool quit_requested_ = false;
    bool sdl_initialized_ = false;
    bool perf_monitor_enabled_ = false;
    bool show_mode_ = false;
    int32_t screen_mode_ = 0;

    double avg_update_ms_ = 0.0;
    double avg_draw_ms_ = 0.0;
    double avg_fps_ = 0.0;

    std::array<pyxift::Image, kImageBankCount> images_{};
    std::array<pyxift::Tilemap, kTilemapCount> tilemaps_{};

    void pump_events();
};

#endif
