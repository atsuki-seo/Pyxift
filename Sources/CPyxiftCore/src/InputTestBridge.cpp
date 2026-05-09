#include "pyxift_input_test_c.h"

#include "core/Input.hpp"

#include <cstring>

struct PyxiftInputStateHandle {
    pyxift::InputState state;
};

extern "C" {

PyxiftInputStateHandle *pyxift_input_state_create(void) {
    return new PyxiftInputStateHandle{};
}

void pyxift_input_state_destroy(PyxiftInputStateHandle *h) {
    delete h;
}

void pyxift_input_state_end_frame(PyxiftInputStateHandle *h) {
    if (h != nullptr) h->state.end_frame();
}

void pyxift_input_state_push_button(PyxiftInputStateHandle *h,
                                    uint8_t button, int32_t player, bool down) {
    if (h == nullptr) return;
    pyxift::VirtualEvent ev{};
    ev.type = down ? pyxift::VirtualEvent::Type::ButtonDown
                   : pyxift::VirtualEvent::Type::ButtonUp;
    ev.a = button;
    ev.b = player;
    h->state.push(ev);
}

void pyxift_input_state_push_key(PyxiftInputStateHandle *h, int32_t keycode, bool down) {
    if (h == nullptr) return;
    pyxift::VirtualEvent ev{};
    ev.type = down ? pyxift::VirtualEvent::Type::KeyDown
                   : pyxift::VirtualEvent::Type::KeyUp;
    ev.a = keycode;
    h->state.push(ev);
}

void pyxift_input_state_push_mouse_button(PyxiftInputStateHandle *h, uint8_t button, bool down) {
    if (h == nullptr) return;
    pyxift::VirtualEvent ev{};
    ev.type = down ? pyxift::VirtualEvent::Type::MouseButtonDown
                   : pyxift::VirtualEvent::Type::MouseButtonUp;
    ev.a = button;
    h->state.push(ev);
}

void pyxift_input_state_push_mouse_move(PyxiftInputStateHandle *h, int32_t x, int32_t y) {
    if (h == nullptr) return;
    pyxift::VirtualEvent ev{};
    ev.type = pyxift::VirtualEvent::Type::MouseMove;
    ev.a = x;
    ev.b = y;
    h->state.push(ev);
}

void pyxift_input_state_push_mouse_wheel(PyxiftInputStateHandle *h, int32_t delta) {
    if (h == nullptr) return;
    pyxift::VirtualEvent ev{};
    ev.type = pyxift::VirtualEvent::Type::MouseWheel;
    ev.a = delta;
    h->state.push(ev);
}

void pyxift_input_state_push_gamepad_axis(PyxiftInputStateHandle *h,
                                          int32_t player, uint8_t axis, int32_t raw_value) {
    if (h == nullptr) return;
    pyxift::VirtualEvent ev{};
    ev.type = pyxift::VirtualEvent::Type::GamepadAxis;
    ev.a = player;
    ev.b = axis;
    ev.c = raw_value;
    h->state.push(ev);
}

bool pyxift_input_state_button(const PyxiftInputStateHandle *h, uint8_t button, int32_t player) {
    return h != nullptr && h->state.button(static_cast<pyxift::Button>(button), player);
}

bool pyxift_input_state_button_pressed(const PyxiftInputStateHandle *h,
                                       uint8_t button, int32_t player,
                                       int32_t hold, int32_t repeat) {
    return h != nullptr && h->state.button_pressed(
        static_cast<pyxift::Button>(button), player, hold, repeat);
}

bool pyxift_input_state_button_released(const PyxiftInputStateHandle *h, uint8_t button, int32_t player) {
    return h != nullptr && h->state.button_released(static_cast<pyxift::Button>(button), player);
}

bool pyxift_input_state_key(const PyxiftInputStateHandle *h, int32_t keycode) {
    return h != nullptr && h->state.key(keycode);
}

bool pyxift_input_state_key_pressed(const PyxiftInputStateHandle *h, int32_t keycode,
                                    int32_t hold, int32_t repeat) {
    return h != nullptr && h->state.key_pressed(keycode, hold, repeat);
}

bool pyxift_input_state_key_released(const PyxiftInputStateHandle *h, int32_t keycode) {
    return h != nullptr && h->state.key_released(keycode);
}

bool pyxift_input_state_mouse_button(const PyxiftInputStateHandle *h, uint8_t button) {
    return h != nullptr && h->state.mouse_button(static_cast<pyxift::MouseButton>(button));
}

bool pyxift_input_state_mouse_button_pressed(const PyxiftInputStateHandle *h, uint8_t button) {
    return h != nullptr && h->state.mouse_button_pressed(static_cast<pyxift::MouseButton>(button));
}

int32_t pyxift_input_state_mouse_x(const PyxiftInputStateHandle *h) {
    return h != nullptr ? h->state.mouse_x() : 0;
}

int32_t pyxift_input_state_mouse_y(const PyxiftInputStateHandle *h) {
    return h != nullptr ? h->state.mouse_y() : 0;
}

int32_t pyxift_input_state_mouse_wheel(const PyxiftInputStateHandle *h) {
    return h != nullptr ? h->state.mouse_wheel() : 0;
}

float pyxift_input_state_gamepad_axis(const PyxiftInputStateHandle *h, int32_t player, uint8_t axis) {
    if (h == nullptr) return 0.0f;
    return h->state.gamepad_axis(player, static_cast<pyxift::GamepadAxis>(axis));
}

void pyxift_input_state_push_text(PyxiftInputStateHandle *h, const char *utf8) {
    if (h != nullptr) h->state.push_text(utf8);
}

void pyxift_input_state_push_input_key(PyxiftInputStateHandle *h, int32_t keycode) {
    if (h != nullptr) h->state.push_input_key(keycode);
}

void pyxift_input_state_push_dropped_file(PyxiftInputStateHandle *h, const char *path) {
    if (h != nullptr) h->state.push_dropped_file(path);
}

void pyxift_input_state_set_mouse_pos(PyxiftInputStateHandle *h, int32_t x, int32_t y) {
    if (h != nullptr) h->state.set_mouse_pos(x, y);
}

static int32_t copy_string_to_buf(const std::string &s, char *buf, int32_t buf_size) {
    const int32_t len = static_cast<int32_t>(s.size());
    if (buf != nullptr && buf_size > 0) {
        const int32_t copy = len < buf_size - 1 ? len : buf_size - 1;
        std::memcpy(buf, s.data(), static_cast<size_t>(copy));
        buf[copy] = '\0';
    }
    return len;
}

int32_t pyxift_input_state_input_text(const PyxiftInputStateHandle *h, char *buf, int32_t buf_size) {
    if (h == nullptr) {
        if (buf != nullptr && buf_size > 0) buf[0] = '\0';
        return 0;
    }
    return copy_string_to_buf(h->state.input_text(), buf, buf_size);
}

int32_t pyxift_input_state_input_keys_count(const PyxiftInputStateHandle *h) {
    return h != nullptr ? static_cast<int32_t>(h->state.input_keys().size()) : 0;
}

int32_t pyxift_input_state_input_keys_at(const PyxiftInputStateHandle *h, int32_t index) {
    if (h == nullptr) return 0;
    const auto &keys = h->state.input_keys();
    if (index < 0 || static_cast<size_t>(index) >= keys.size()) return 0;
    return keys[static_cast<size_t>(index)];
}

int32_t pyxift_input_state_dropped_files_count(const PyxiftInputStateHandle *h) {
    return h != nullptr ? static_cast<int32_t>(h->state.dropped_files().size()) : 0;
}

int32_t pyxift_input_state_dropped_files_at(const PyxiftInputStateHandle *h, int32_t index,
                                            char *buf, int32_t buf_size) {
    if (h == nullptr) {
        if (buf != nullptr && buf_size > 0) buf[0] = '\0';
        return -1;
    }
    const auto &files = h->state.dropped_files();
    if (index < 0 || static_cast<size_t>(index) >= files.size()) {
        if (buf != nullptr && buf_size > 0) buf[0] = '\0';
        return -1;
    }
    return copy_string_to_buf(files[static_cast<size_t>(index)], buf, buf_size);
}

} // extern "C"
