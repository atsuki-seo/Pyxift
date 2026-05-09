#ifndef PYXIFT_CORE_INPUT_HPP
#define PYXIFT_CORE_INPUT_HPP

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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

enum class GamepadAxis : uint8_t {
    LeftX = 0,
    LeftY,
    RightX,
    RightY,
    LeftTrigger,
    RightTrigger,
    Count,
};

struct VirtualEvent {
    enum class Type : uint8_t {
        KeyDown,
        KeyUp,
        ButtonDown,
        ButtonUp,
        MouseMove,
        MouseButtonDown,
        MouseButtonUp,
        MouseWheel,
        GamepadConnected,
        GamepadDisconnected,
        GamepadAxis,
    };

    Type type;
    int32_t a = 0;
    int32_t b = 0;
    int32_t c = 0;
};

class InputState {
public:
    static constexpr int32_t kMaxPlayers = 4;
    static constexpr int32_t kAxisCount = static_cast<int32_t>(GamepadAxis::Count);

    void push(const VirtualEvent &ev);

    void push_text(const char *utf8);
    void push_input_key(int32_t keycode);
    void push_dropped_file(const char *path);
    void set_mouse_pos(int32_t x, int32_t y);

    void end_frame();

    bool button(Button b, int32_t player) const;
    bool button_pressed(Button b, int32_t player) const;
    bool button_pressed(Button b, int32_t player, int32_t hold, int32_t repeat) const;
    bool button_released(Button b, int32_t player) const;

    bool key(int32_t keycode) const;
    bool key_pressed(int32_t keycode) const;
    bool key_pressed(int32_t keycode, int32_t hold, int32_t repeat) const;
    bool key_released(int32_t keycode) const;

    int32_t mouse_x() const { return mouse_x_; }
    int32_t mouse_y() const { return mouse_y_; }
    int32_t mouse_wheel() const { return mouse_wheel_; }
    bool mouse_button(MouseButton b) const;
    bool mouse_button_pressed(MouseButton b) const;
    bool mouse_button_released(MouseButton b) const;

    float gamepad_axis(int32_t player, GamepadAxis axis) const;

    const std::string &input_text() const { return input_text_; }
    const std::vector<int32_t> &input_keys() const { return input_keys_; }
    const std::vector<std::string> &dropped_files() const { return dropped_files_; }

    uint64_t frame_count() const { return frame_count_; }

private:
    std::array<uint16_t, kMaxPlayers> btn_state_{};
    std::array<uint16_t, kMaxPlayers> btn_pressed_{};
    std::array<uint16_t, kMaxPlayers> btn_released_{};
    std::array<std::array<uint64_t, static_cast<int32_t>(Button::Count)>, kMaxPlayers> btn_press_frame_{};

    // SDL3 SDL_Keycode values span a wide, non-contiguous range, so we hold them in a hash set rather than an array.
    std::unordered_set<int32_t> key_state_;
    std::unordered_set<int32_t> key_pressed_;
    std::unordered_set<int32_t> key_released_;
    std::unordered_map<int32_t, uint64_t> key_press_frame_;

    int32_t mouse_x_ = 0;
    int32_t mouse_y_ = 0;
    int32_t mouse_wheel_ = 0;
    uint8_t mb_state_ = 0;
    uint8_t mb_pressed_ = 0;
    uint8_t mb_released_ = 0;

    std::array<std::array<int16_t, kAxisCount>, kMaxPlayers> axis_value_{};

    std::string input_text_;
    std::vector<int32_t> input_keys_;
    std::vector<std::string> dropped_files_;

    uint64_t frame_count_ = 0;

    bool repeat_match(uint64_t press_frame, int32_t hold, int32_t repeat) const;

    static uint16_t mask(Button b) {
        return static_cast<uint16_t>(1u << static_cast<uint8_t>(b));
    }
    static uint8_t mask(MouseButton b) {
        return static_cast<uint8_t>(1u << static_cast<uint8_t>(b));
    }
};

} // namespace pyxift

#endif
