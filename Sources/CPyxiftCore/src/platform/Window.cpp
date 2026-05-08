#include "Window.hpp"

#include <vector>

namespace pyxift::platform {

namespace {

int32_t initial_scale(int32_t logical_w, int32_t logical_h) {
    // Start from 4x so the low logical resolution stays readable on modern displays, and only
    // step down when the scaled window would not fit within 9/10 of the display.
    SDL_DisplayID display = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(display);
    int32_t scale = 4;
    if (mode != nullptr) {
        while (scale > 1 &&
               (logical_w * scale > mode->w * 9 / 10 ||
                logical_h * scale > mode->h * 9 / 10)) {
            --scale;
        }
    }
    return scale;
}

} // namespace

Window::Window(int32_t logical_width, int32_t logical_height, const std::string &title)
    : width_(logical_width), height_(logical_height) {
    const int32_t scale = initial_scale(logical_width, logical_height);
    window_ = SDL_CreateWindow(title.c_str(),
                               logical_width * scale,
                               logical_height * scale,
                               SDL_WINDOW_RESIZABLE);
    if (window_ == nullptr) return;

    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (renderer_ == nullptr) return;

    SDL_SetRenderVSync(renderer_, 1);
    SDL_SetRenderLogicalPresentation(renderer_,
                                     logical_width,
                                     logical_height,
                                     SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

    texture_ = SDL_CreateTexture(renderer_,
                                 SDL_PIXELFORMAT_RGBA32,
                                 SDL_TEXTUREACCESS_STREAMING,
                                 logical_width,
                                 logical_height);
    if (texture_ != nullptr) {
        SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_NEAREST);
    }
}

Window::~Window() {
    if (texture_ != nullptr) SDL_DestroyTexture(texture_);
    if (renderer_ != nullptr) SDL_DestroyRenderer(renderer_);
    if (window_ != nullptr) SDL_DestroyWindow(window_);
}

void Window::set_cursor_visible(bool visible) {
    if (visible) {
        SDL_ShowCursor();
    } else {
        SDL_HideCursor();
    }
}

void Window::set_title(const std::string &title) {
    if (window_ == nullptr) return;
    SDL_SetWindowTitle(window_, title.c_str());
}

void Window::present(const uint8_t *index_buffer, const uint32_t *palette) {
    if (!valid()) return;

    void *texture_pixels = nullptr;
    int pitch = 0;
    if (!SDL_LockTexture(texture_, nullptr, &texture_pixels, &pitch)) {
        return;
    }

    auto *out = static_cast<uint8_t *>(texture_pixels);
    for (int32_t y = 0; y < height_; ++y) {
        uint8_t *row = out + static_cast<size_t>(y) * pitch;
        const uint8_t *in_row = index_buffer + static_cast<size_t>(y) * width_;
        for (int32_t x = 0; x < width_; ++x) {
            const uint32_t rgb = palette[in_row[x] & 0x0f];
            row[x * 4 + 0] = static_cast<uint8_t>((rgb >> 16) & 0xff);
            row[x * 4 + 1] = static_cast<uint8_t>((rgb >> 8) & 0xff);
            row[x * 4 + 2] = static_cast<uint8_t>(rgb & 0xff);
            row[x * 4 + 3] = 0xff;
        }
    }
    SDL_UnlockTexture(texture_);

    SDL_SetRenderDrawColor(renderer_, 0x20, 0x22, 0x24, 0xff);
    SDL_RenderClear(renderer_);
    SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}

} // namespace pyxift::platform
