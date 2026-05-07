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

    void present(const uint8_t *index_buffer, const uint32_t *palette);

    SDL_Renderer *renderer() { return renderer_; }
    SDL_Window *sdl_window() { return window_; }

    static void set_cursor_visible(bool visible);

private:
    int32_t width_;
    int32_t height_;
    SDL_Window *window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;
    SDL_Texture *texture_ = nullptr;
};

} // namespace pyxift::platform

#endif
