#ifndef PYXIFT_PLATFORM_WINDOW_HPP
#define PYXIFT_PLATFORM_WINDOW_HPP

#include <SDL3/SDL.h>

#include <cstdint>
#include <string>

namespace pyxift::platform {

// SDL3 ウィンドウ + テクスチャ転送のラッパー。
// 16色インデックスバッファを RGBA8 に変換してテクスチャに upload し、
// nearest neighbor で整数倍に拡大表示する。
class Window {
public:
    Window(int32_t logical_width, int32_t logical_height, const std::string &title);
    ~Window();

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    bool valid() const { return window_ != nullptr && renderer_ != nullptr && texture_ != nullptr; }

    void set_title(const std::string &title);

    // index_buffer: width*height バイト、各値はパレットインデックス 0..15
    // palette: RGB値 16 個
    void present(const uint8_t *index_buffer, const uint32_t *palette);

private:
    int32_t width_;
    int32_t height_;
    SDL_Window *window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;
    SDL_Texture *texture_ = nullptr;
};

} // namespace pyxift::platform

#endif
