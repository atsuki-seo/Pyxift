#ifndef PYXIFT_CORE_INPUT_HPP
#define PYXIFT_CORE_INPUT_HPP

#include <array>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

namespace pyxift {

enum class Button : uint8_t {
    Left = 0,
    Right,
    Up,
    Down,
    A,
    B,
    Start,
    Count,
};

enum class MouseButton : uint8_t {
    Left = 0,
    Right,
    Middle,
    Count,
};

// VirtualEvent: SDL3 から切り離されたイベント表現。
// アダプタ層が SDL_Event を翻訳して InputState::push に流す。
struct VirtualEvent {
    enum class Type : uint8_t {
        KeyDown,
        KeyUp,
        ButtonDown,        // 仮想ボタン押下（player フィールドが 0..3）
        ButtonUp,
        MouseMove,
        MouseButtonDown,
        MouseButtonUp,
        MouseWheel,
        GamepadConnected,  // gamepad_id をプレイヤー番号にマッピング登録するシグナル
        GamepadDisconnected,
    };

    Type type;
    int32_t a = 0;
    int32_t b = 0;
    int32_t c = 0;
};

// pure C++、SDL3 非依存。VirtualEvent 列を注入してテスト可能。
class InputState {
public:
    static constexpr int32_t kMaxPlayers = 4;

    void push(const VirtualEvent &ev);

    // フレーム末尾。pressed/released の bit を消し、button/key の最終状態のみ残す。
    void end_frame();

    bool button(Button b, int32_t player) const;
    bool button_pressed(Button b, int32_t player) const;
    bool button_released(Button b, int32_t player) const;

    bool key(int32_t keycode) const;
    bool key_pressed(int32_t keycode) const;
    bool key_released(int32_t keycode) const;

    int32_t mouse_x() const { return mouse_x_; }
    int32_t mouse_y() const { return mouse_y_; }
    int32_t mouse_wheel() const { return mouse_wheel_; }
    bool mouse_button(MouseButton b) const;
    bool mouse_button_pressed(MouseButton b) const;
    bool mouse_button_released(MouseButton b) const;

private:
    std::array<uint16_t, kMaxPlayers> btn_state_{};
    std::array<uint16_t, kMaxPlayers> btn_pressed_{};
    std::array<uint16_t, kMaxPlayers> btn_released_{};

    // SDL3 keycode は疎なのでハッシュ集合で保持。
    std::unordered_set<int32_t> key_state_;
    std::unordered_set<int32_t> key_pressed_;
    std::unordered_set<int32_t> key_released_;

    int32_t mouse_x_ = 0;
    int32_t mouse_y_ = 0;
    int32_t mouse_wheel_ = 0;
    uint8_t mb_state_ = 0;
    uint8_t mb_pressed_ = 0;
    uint8_t mb_released_ = 0;

    static uint16_t mask(Button b) {
        return static_cast<uint16_t>(1u << static_cast<uint8_t>(b));
    }
    static uint8_t mask(MouseButton b) {
        return static_cast<uint8_t>(1u << static_cast<uint8_t>(b));
    }
};

} // namespace pyxift

#endif
