#ifndef PYXIFT_ENGINE_HPP
#define PYXIFT_ENGINE_HPP

#include "core/Canvas.hpp"
#include "core/Image.hpp"
#include "core/Input.hpp"
#include "core/Tilemap.hpp"
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

    int32_t width() const { return canvas_.width(); }
    int32_t height() const { return canvas_.height(); }
    int32_t frame_count() const { return frame_count_; }

    pyxift::Canvas &canvas() { return canvas_; }
    const pyxift::Canvas &canvas() const { return canvas_; }

    pyxift::Image *image(int32_t bank);
    pyxift::Tilemap *tilemap(int32_t index);

    pyxift::InputState &input() { return input_; }
    const pyxift::InputState &input() const { return input_; }

private:
    pyxift::Canvas canvas_;
    pyxift::platform::Window window_;
    pyxift::InputState input_{};
    pyxift::platform::EventTranslator event_translator_{};
    int32_t fps_;
    int32_t frame_count_ = 0;
    bool quit_requested_ = false;
    bool sdl_initialized_ = false;

    std::array<pyxift::Image, kImageBankCount> images_{};
    std::array<pyxift::Tilemap, kTilemapCount> tilemaps_{};

    void pump_events();
};

#endif
