#ifndef PYXIFT_PLATFORM_WINDOW_HPP
#define PYXIFT_PLATFORM_WINDOW_HPP

#include <SDL3/SDL.h>

#include <cstdint>
#include <string>

namespace pyxift::platform {

class Window {
public:
    Window(int32_t logical_width, int32_t logical_height, const std::string &title);
    ~Window();

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    bool valid() const { return window_ != nullptr && renderer_ != nullptr && texture_ != nullptr; }

    void set_title(const std::string &title);

    void warp_mouse(int32_t logical_x, int32_t logical_y);

    void present(const uint8_t *index_buffer, const uint32_t *palette);

    void set_fullscreen(bool enabled);
    bool fullscreen() const { return fullscreen_; }

    void resize_logical(int32_t logical_width, int32_t logical_height);

    void set_integer_scale(bool enabled);
    bool integer_scale() const { return integer_scale_; }

    void set_linear_filtering(bool enabled);

    void set_icon(const uint8_t *rgba, int32_t w, int32_t h);

    SDL_Renderer *renderer() { return renderer_; }
    SDL_Window *sdl_window() { return window_; }

    static void set_cursor_visible(bool visible);

private:
    int32_t width_;
    int32_t height_;
    SDL_Window *window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;
    SDL_Texture *texture_ = nullptr;
    bool fullscreen_ = false;
    bool integer_scale_ = true;

    void apply_logical_presentation();
};

} // namespace pyxift::platform

#endif
