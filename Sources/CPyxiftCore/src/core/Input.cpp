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
                key_press_frame_[kc] = frame_count_;
            }
            break;
        }
        case T::KeyUp: {
            const int32_t kc = ev.a;
            if (key_state_.erase(kc) > 0) {
                key_released_.insert(kc);
                key_press_frame_.erase(kc);
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
                btn_press_frame_[player][button_idx] = frame_count_;
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
        case T::GamepadAxis: {
            const int32_t player = ev.a;
            const int32_t axis_idx = ev.b;
            if (!valid_player(player) || axis_idx < 0 || axis_idx >= kAxisCount) break;
            int32_t v = ev.c;
            if (v < INT16_MIN) v = INT16_MIN;
            if (v > INT16_MAX) v = INT16_MAX;
            axis_value_[player][axis_idx] = static_cast<int16_t>(v);
            break;
        }
        case T::GamepadConnected:
        case T::GamepadDisconnected:
            break;
    }
}

void InputState::push_text(const char *utf8) {
    if (utf8 == nullptr) return;
    input_text_.append(utf8);
}

void InputState::push_input_key(int32_t keycode) {
    input_keys_.push_back(keycode);
}

void InputState::push_dropped_file(const char *path) {
    if (path == nullptr) return;
    dropped_files_.emplace_back(path);
}

void InputState::set_mouse_pos(int32_t x, int32_t y) {
    mouse_x_ = x;
    mouse_y_ = y;
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
    input_text_.clear();
    input_keys_.clear();
    dropped_files_.clear();
    ++frame_count_;
}

bool InputState::button(Button b, int32_t player) const {
    if (!valid_player(player)) return false;
    return (btn_state_[player] & mask(b)) != 0;
}

bool InputState::button_pressed(Button b, int32_t player) const {
    return button_pressed(b, player, 0, 0);
}

bool InputState::repeat_match(uint64_t press_frame, int32_t hold, int32_t repeat) const {
    if (repeat <= 0) return false;
    const uint64_t h = hold > 0 ? static_cast<uint64_t>(hold) : 0;
    if (frame_count_ < press_frame + h) return false;
    const uint64_t elapsed = frame_count_ - press_frame - h;
    return elapsed % static_cast<uint64_t>(repeat) == 0;
}

bool InputState::button_pressed(Button b, int32_t player, int32_t hold, int32_t repeat) const {
    if (!valid_player(player)) return false;
    if ((btn_pressed_[player] & mask(b)) != 0) return true;
    if ((btn_state_[player] & mask(b)) == 0) return false;
    return repeat_match(btn_press_frame_[player][static_cast<int32_t>(b)], hold, repeat);
}

bool InputState::button_released(Button b, int32_t player) const {
    if (!valid_player(player)) return false;
    return (btn_released_[player] & mask(b)) != 0;
}

bool InputState::key(int32_t keycode) const {
    return key_state_.count(keycode) > 0;
}

bool InputState::key_pressed(int32_t keycode) const {
    return key_pressed(keycode, 0, 0);
}

bool InputState::key_pressed(int32_t keycode, int32_t hold, int32_t repeat) const {
    if (key_pressed_.count(keycode) > 0) return true;
    auto it = key_press_frame_.find(keycode);
    if (it == key_press_frame_.end()) return false;
    return repeat_match(it->second, hold, repeat);
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

float InputState::gamepad_axis(int32_t player, GamepadAxis axis) const {
    if (!valid_player(player)) return 0.0f;
    const int32_t idx = static_cast<int32_t>(axis);
    if (idx < 0 || idx >= kAxisCount) return 0.0f;
    const int16_t raw = axis_value_[player][idx];
    const float norm = static_cast<float>(raw) / 32767.0f;
    if (axis == GamepadAxis::LeftTrigger || axis == GamepadAxis::RightTrigger) {
        // SDL trigger axes report 0..32767; clamp the normalized output to [0, 1] for the Swift surface.
        if (norm < 0.0f) return 0.0f;
        if (norm > 1.0f) return 1.0f;
        return norm;
    }
    if (norm < -1.0f) return -1.0f;
    if (norm > 1.0f) return 1.0f;
    return norm;
}

} // namespace pyxift
