#ifndef PYXIFT_ENGINE_HPP
#define PYXIFT_ENGINE_HPP

#include "core/Canvas.hpp"
#include "platform/Window.hpp"

#include "pyxift_c.h"

#include <cstdint>
#include <memory>
#include <string>

struct PyxiftEngine {
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

private:
    pyxift::Canvas canvas_;
    pyxift::platform::Window window_;
    int32_t fps_;
    int32_t frame_count_ = 0;
    bool quit_requested_ = false;
    bool sdl_initialized_ = false;

    void pump_events();
};

#endif
