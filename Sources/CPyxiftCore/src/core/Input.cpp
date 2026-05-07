#include "Input.hpp"

namespace pyxift {

namespace {
constexpr int32_t kButtonCount = static_cast<int32_t>(Button::Count);

bool valid_player(int32_t p) {
    return p >= 0 && p < InputState::kMaxPlayers;
}
} // namespace

void InputState::push(const VirtualEvent &ev) {
    using T = VirtualEvent::Type;
    switch (ev.type) {
        case T::KeyDown: {
            const int32_t kc = ev.a;
            if (key_state_.insert(kc).second) {
                key_pressed_.insert(kc);
            }
            break;
        }
        case T::KeyUp: {
            const int32_t kc = ev.a;
            if (key_state_.erase(kc) > 0) {
                key_released_.insert(kc);
            }
            break;
        }
        case T::ButtonDown: {
            const int32_t button_idx = ev.a;
            const int32_t player = ev.b;
            if (!valid_player(player) || button_idx < 0 || button_idx >= kButtonCount) break;
            const uint16_t m = static_cast<uint16_t>(1u << button_idx);
            if ((btn_state_[player] & m) == 0) {
                btn_state_[player] |= m;
                btn_pressed_[player] |= m;
            }
            break;
        }
        case T::ButtonUp: {
            const int32_t button_idx = ev.a;
            const int32_t player = ev.b;
            if (!valid_player(player) || button_idx < 0 || button_idx >= kButtonCount) break;
            const uint16_t m = static_cast<uint16_t>(1u << button_idx);
            if ((btn_state_[player] & m) != 0) {
                btn_state_[player] &= static_cast<uint16_t>(~m);
                btn_released_[player] |= m;
            }
            break;
        }
        case T::MouseMove:
            mouse_x_ = ev.a;
            mouse_y_ = ev.b;
            break;
        case T::MouseButtonDown: {
            const int32_t mb = ev.a;
            if (mb < 0 || mb >= static_cast<int32_t>(MouseButton::Count)) break;
            const uint8_t m = static_cast<uint8_t>(1u << mb);
            if ((mb_state_ & m) == 0) {
                mb_state_ |= m;
                mb_pressed_ |= m;
            }
            break;
        }
        case T::MouseButtonUp: {
            const int32_t mb = ev.a;
            if (mb < 0 || mb >= static_cast<int32_t>(MouseButton::Count)) break;
            const uint8_t m = static_cast<uint8_t>(1u << mb);
            if ((mb_state_ & m) != 0) {
                mb_state_ &= static_cast<uint8_t>(~m);
                mb_released_ |= m;
            }
            break;
        }
        case T::MouseWheel:
            mouse_wheel_ += ev.a;
            break;
        case T::GamepadConnected:
        case T::GamepadDisconnected:
            // コア層は接続管理を持たない。アダプタ層が ButtonDown/Up に翻訳する。
            break;
    }
}

void InputState::end_frame() {
    for (int32_t p = 0; p < kMaxPlayers; ++p) {
        btn_pressed_[p] = 0;
        btn_released_[p] = 0;
    }
    key_pressed_.clear();
    key_released_.clear();
    mb_pressed_ = 0;
    mb_released_ = 0;
    mouse_wheel_ = 0;
}

bool InputState::button(Button b, int32_t player) const {
    if (!valid_player(player)) return false;
    return (btn_state_[player] & mask(b)) != 0;
}

bool InputState::button_pressed(Button b, int32_t player) const {
    if (!valid_player(player)) return false;
    return (btn_pressed_[player] & mask(b)) != 0;
}

bool InputState::button_released(Button b, int32_t player) const {
    if (!valid_player(player)) return false;
    return (btn_released_[player] & mask(b)) != 0;
}

bool InputState::key(int32_t keycode) const {
    return key_state_.count(keycode) > 0;
}

bool InputState::key_pressed(int32_t keycode) const {
    return key_pressed_.count(keycode) > 0;
}

bool InputState::key_released(int32_t keycode) const {
    return key_released_.count(keycode) > 0;
}

bool InputState::mouse_button(MouseButton b) const {
    return (mb_state_ & mask(b)) != 0;
}

bool InputState::mouse_button_pressed(MouseButton b) const {
    return (mb_pressed_ & mask(b)) != 0;
}

bool InputState::mouse_button_released(MouseButton b) const {
    return (mb_released_ & mask(b)) != 0;
}

} // namespace pyxift
