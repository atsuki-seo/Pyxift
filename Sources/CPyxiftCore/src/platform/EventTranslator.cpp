#include "EventTranslator.hpp"

namespace pyxift::platform {

namespace {

// 1 つの仮想ボタンを複数のキーで重ね打ちできるよう、テーブル定義する。
struct KeyMap {
    SDL_Keycode key;
    Button button;
};

constexpr KeyMap kKeyMap[] = {
    {SDLK_LEFT,   Button::Left},
    {SDLK_A,      Button::Left},
    {SDLK_RIGHT,  Button::Right},
    {SDLK_D,      Button::Right},
    {SDLK_UP,     Button::Up},
    {SDLK_W,      Button::Up},
    {SDLK_DOWN,   Button::Down},
    {SDLK_S,      Button::Down},
    {SDLK_Z,      Button::A},
    {SDLK_X,      Button::B},
    {SDLK_RETURN, Button::Start},
};

// マッピングなしの場合は Button::Count を返す。
Button gamepad_button_to_virtual(uint8_t b) {
    switch (b) {
        case SDL_GAMEPAD_BUTTON_DPAD_LEFT:  return Button::Left;
        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: return Button::Right;
        case SDL_GAMEPAD_BUTTON_DPAD_UP:    return Button::Up;
        case SDL_GAMEPAD_BUTTON_DPAD_DOWN:  return Button::Down;
        case SDL_GAMEPAD_BUTTON_SOUTH:      return Button::A;
        case SDL_GAMEPAD_BUTTON_EAST:       return Button::B;
        case SDL_GAMEPAD_BUTTON_START:      return Button::Start;
        default:                            return Button::Count;
    }
}

constexpr int16_t kAxisDeadZone = 16384;

void push_button(InputState &state, Button b, int32_t player, bool down) {
    VirtualEvent ev{};
    ev.type = down ? VirtualEvent::Type::ButtonDown : VirtualEvent::Type::ButtonUp;
    ev.a = static_cast<int32_t>(b);
    ev.b = player;
    state.push(ev);
}

int32_t mouse_button_index(uint8_t sdl_button) {
    switch (sdl_button) {
        case SDL_BUTTON_LEFT:   return static_cast<int32_t>(MouseButton::Left);
        case SDL_BUTTON_RIGHT:  return static_cast<int32_t>(MouseButton::Right);
        case SDL_BUTTON_MIDDLE: return static_cast<int32_t>(MouseButton::Middle);
        default:                return -1;
    }
}

} // namespace

int32_t EventTranslator::assign_player(SDL_JoystickID id) {
    for (int32_t i = 0; i < kMaxPlayers; ++i) {
        if (slots_[i].used && slots_[i].id == id) return i;
    }
    for (int32_t i = 0; i < kMaxPlayers; ++i) {
        if (!slots_[i].used) {
            slots_[i].used = true;
            slots_[i].id = id;
            slots_[i].gamepad = SDL_OpenGamepad(id);
            return i;
        }
    }
    return -1;
}

int32_t EventTranslator::player_for(SDL_JoystickID id) const {
    for (int32_t i = 0; i < kMaxPlayers; ++i) {
        if (slots_[i].used && slots_[i].id == id) return i;
    }
    return -1;
}

void EventTranslator::release_player(SDL_JoystickID id) {
    for (int32_t i = 0; i < kMaxPlayers; ++i) {
        if (slots_[i].used && slots_[i].id == id) {
            if (slots_[i].gamepad != nullptr) {
                SDL_CloseGamepad(slots_[i].gamepad);
            }
            slots_[i] = Slot{};
            return;
        }
    }
}

void EventTranslator::emit_key_button(InputState &state, SDL_Keycode key, bool down) {
    // キーボード入力は player 0 のみへ流す。
    for (const auto &m : kKeyMap) {
        if (m.key == key) {
            push_button(state, m.button, 0, down);
        }
    }
}

void EventTranslator::emit_gamepad_button(InputState &state, int32_t player, uint8_t button, bool down) {
    const Button vb = gamepad_button_to_virtual(button);
    if (vb == Button::Count) return;
    push_button(state, vb, player, down);
}

void EventTranslator::emit_gamepad_axis(InputState &state, int32_t player, uint8_t axis, int16_t value) {
    int32_t idx = -1;
    Button neg_btn = Button::Count;
    Button pos_btn = Button::Count;
    if (axis == SDL_GAMEPAD_AXIS_LEFTX) {
        idx = 0; neg_btn = Button::Left; pos_btn = Button::Right;
    } else if (axis == SDL_GAMEPAD_AXIS_LEFTY) {
        idx = 1; neg_btn = Button::Up; pos_btn = Button::Down;
    } else {
        return;
    }

    AxisDir &dir = axis_dirs_[player][idx];
    const bool want_neg = value < -kAxisDeadZone;
    const bool want_pos = value >  kAxisDeadZone;

    if (want_neg != dir.neg) {
        push_button(state, neg_btn, player, want_neg);
        dir.neg = want_neg;
    }
    if (want_pos != dir.pos) {
        push_button(state, pos_btn, player, want_pos);
        dir.pos = want_pos;
    }
}

bool EventTranslator::translate(SDL_Event &ev, InputState &state) {
    // マウス系イベントの座標をロジカル解像度に正規化する。
    if (renderer_ != nullptr) {
        SDL_ConvertEventToRenderCoordinates(renderer_, &ev);
    }

    switch (ev.type) {
        case SDL_EVENT_QUIT:
            return true;
        case SDL_EVENT_KEY_DOWN: {
            if (ev.key.key == SDLK_ESCAPE) return true;
            if (!ev.key.repeat) {
                VirtualEvent ve{};
                ve.type = VirtualEvent::Type::KeyDown;
                ve.a = static_cast<int32_t>(ev.key.key);
                state.push(ve);
                emit_key_button(state, ev.key.key, true);
            }
            break;
        }
        case SDL_EVENT_KEY_UP: {
            VirtualEvent ve{};
            ve.type = VirtualEvent::Type::KeyUp;
            ve.a = static_cast<int32_t>(ev.key.key);
            state.push(ve);
            emit_key_button(state, ev.key.key, false);
            break;
        }
        case SDL_EVENT_MOUSE_MOTION: {
            VirtualEvent ve{};
            ve.type = VirtualEvent::Type::MouseMove;
            ve.a = static_cast<int32_t>(ev.motion.x);
            ve.b = static_cast<int32_t>(ev.motion.y);
            state.push(ve);
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            const int32_t mb = mouse_button_index(ev.button.button);
            if (mb < 0) break;
            VirtualEvent ve{};
            ve.type = ev.button.down ? VirtualEvent::Type::MouseButtonDown
                                     : VirtualEvent::Type::MouseButtonUp;
            ve.a = mb;
            state.push(ve);
            break;
        }
        case SDL_EVENT_MOUSE_WHEEL: {
            VirtualEvent ve{};
            ve.type = VirtualEvent::Type::MouseWheel;
            ve.a = ev.wheel.integer_y;
            state.push(ve);
            break;
        }
        case SDL_EVENT_GAMEPAD_ADDED: {
            assign_player(ev.gdevice.which);
            break;
        }
        case SDL_EVENT_GAMEPAD_REMOVED: {
            release_player(ev.gdevice.which);
            break;
        }
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP: {
            const int32_t player = player_for(ev.gbutton.which);
            if (player < 0) break;
            emit_gamepad_button(state, player, ev.gbutton.button, ev.gbutton.down);
            break;
        }
        case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
            const int32_t player = player_for(ev.gaxis.which);
            if (player < 0) break;
            emit_gamepad_axis(state, player, ev.gaxis.axis, ev.gaxis.value);
            break;
        }
        default:
            break;
    }
    return false;
}

} // namespace pyxift::platform
