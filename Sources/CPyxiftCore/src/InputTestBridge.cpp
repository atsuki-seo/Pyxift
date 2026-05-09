#include "pyxift_input_test_c.h"

#include "core/Input.hpp"

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

} // extern "C"
