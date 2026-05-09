#ifndef PYXIFT_INPUT_TEST_C_H
#define PYXIFT_INPUT_TEST_C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct PyxiftInputStateHandle PyxiftInputStateHandle;

PyxiftInputStateHandle *pyxift_input_state_create(void);
void pyxift_input_state_destroy(PyxiftInputStateHandle *h);
void pyxift_input_state_end_frame(PyxiftInputStateHandle *h);

void pyxift_input_state_push_button(PyxiftInputStateHandle *h,
                                    uint8_t button, int32_t player, bool down);
void pyxift_input_state_push_key(PyxiftInputStateHandle *h, int32_t keycode, bool down);
void pyxift_input_state_push_mouse_button(PyxiftInputStateHandle *h, uint8_t button, bool down);
void pyxift_input_state_push_mouse_move(PyxiftInputStateHandle *h, int32_t x, int32_t y);
void pyxift_input_state_push_mouse_wheel(PyxiftInputStateHandle *h, int32_t delta);
void pyxift_input_state_push_gamepad_axis(PyxiftInputStateHandle *h,
                                          int32_t player, uint8_t axis, int32_t raw_value);

bool pyxift_input_state_button(const PyxiftInputStateHandle *h, uint8_t button, int32_t player);
bool pyxift_input_state_button_pressed(const PyxiftInputStateHandle *h,
                                       uint8_t button, int32_t player,
                                       int32_t hold, int32_t repeat);
bool pyxift_input_state_button_released(const PyxiftInputStateHandle *h, uint8_t button, int32_t player);
bool pyxift_input_state_key(const PyxiftInputStateHandle *h, int32_t keycode);
bool pyxift_input_state_key_pressed(const PyxiftInputStateHandle *h, int32_t keycode,
                                    int32_t hold, int32_t repeat);
bool pyxift_input_state_key_released(const PyxiftInputStateHandle *h, int32_t keycode);
bool pyxift_input_state_mouse_button(const PyxiftInputStateHandle *h, uint8_t button);
bool pyxift_input_state_mouse_button_pressed(const PyxiftInputStateHandle *h, uint8_t button);
int32_t pyxift_input_state_mouse_x(const PyxiftInputStateHandle *h);
int32_t pyxift_input_state_mouse_y(const PyxiftInputStateHandle *h);
int32_t pyxift_input_state_mouse_wheel(const PyxiftInputStateHandle *h);
float pyxift_input_state_gamepad_axis(const PyxiftInputStateHandle *h, int32_t player, uint8_t axis);

#ifdef __cplusplus
}
#endif

#endif
