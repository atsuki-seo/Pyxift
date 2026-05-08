#ifndef PYXIFT_PLATFORM_EVENT_TRANSLATOR_HPP
#define PYXIFT_PLATFORM_EVENT_TRANSLATOR_HPP

#include "../core/Input.hpp"

#include <SDL3/SDL.h>

#include <array>

namespace pyxift::platform {

class EventTranslator {
public:
    static constexpr int32_t kMaxPlayers = InputState::kMaxPlayers;

    explicit EventTranslator(SDL_Renderer *renderer = nullptr) : renderer_(renderer) {}

    void set_renderer(SDL_Renderer *renderer) { renderer_ = renderer; }

    // Returns true when a quit-request event (window close / ESC) is observed.
    bool translate(SDL_Event &ev, InputState &state);

private:
    SDL_Renderer *renderer_ = nullptr;

    struct Slot {
        bool used = false;
        SDL_JoystickID id = 0;
        SDL_Gamepad *gamepad = nullptr;
    };
    std::array<Slot, kMaxPlayers> slots_{};

    int32_t assign_player(SDL_JoystickID id);
    int32_t player_for(SDL_JoystickID id) const;
    void release_player(SDL_JoystickID id);

    void emit_key_button(InputState &state, SDL_Keycode key, bool down);
    void emit_gamepad_button(InputState &state, int32_t player, uint8_t button, bool down);
    void emit_gamepad_axis(InputState &state, int32_t player, uint8_t axis, int16_t value);

    // Tracks per-direction "currently pressed" flags so analog axis motion can be turned into
    // ButtonDown/ButtonUp edges, firing an event only when the dead zone boundary is crossed.
    struct AxisDir {
        bool neg = false;
        bool pos = false;
    };
    std::array<std::array<AxisDir, 2>, kMaxPlayers> axis_dirs_{};
};

} // namespace pyxift::platform

#endif
