import Testing
import CPyxiftCore

private final class State {
    let h: OpaquePointer
    init() { h = pyxift_input_state_create()! }
    deinit { pyxift_input_state_destroy(h) }

    var raw: OpaquePointer { h }
}

@Test func buttonPressedFiresOnceThenStaysHeld() {
    let s = State()
    pyxift_input_state_push_button(s.raw, 4, 0, true)

    #expect(pyxift_input_state_button(s.raw, 4, 0))
    #expect(pyxift_input_state_button_pressed(s.raw, 4, 0, 0, 0))
    #expect(!pyxift_input_state_button_released(s.raw, 4, 0))

    pyxift_input_state_end_frame(s.raw)

    #expect(pyxift_input_state_button(s.raw, 4, 0))
    #expect(!pyxift_input_state_button_pressed(s.raw, 4, 0, 0, 0))
    #expect(!pyxift_input_state_button_released(s.raw, 4, 0))
}

@Test func buttonReleaseTransition() {
    let s = State()
    pyxift_input_state_push_button(s.raw, 0, 0, true)
    pyxift_input_state_end_frame(s.raw)
    pyxift_input_state_push_button(s.raw, 0, 0, false)

    #expect(!pyxift_input_state_button(s.raw, 0, 0))
    #expect(pyxift_input_state_button_released(s.raw, 0, 0))
}

@Test func buttonChatteringWithinFrame() {
    let s = State()
    pyxift_input_state_push_button(s.raw, 4, 0, true)
    pyxift_input_state_push_button(s.raw, 4, 0, false)
    pyxift_input_state_push_button(s.raw, 4, 0, true)

    #expect(pyxift_input_state_button(s.raw, 4, 0))
    #expect(pyxift_input_state_button_pressed(s.raw, 4, 0, 0, 0))
    #expect(pyxift_input_state_button_released(s.raw, 4, 0))
}

@Test func buttonChatteringEndsReleased() {
    let s = State()
    pyxift_input_state_push_button(s.raw, 4, 0, true)
    pyxift_input_state_push_button(s.raw, 4, 0, false)

    #expect(!pyxift_input_state_button(s.raw, 4, 0))
    #expect(pyxift_input_state_button_pressed(s.raw, 4, 0, 0, 0))
    #expect(pyxift_input_state_button_released(s.raw, 4, 0))
}

@Test func buttonsArePerPlayer() {
    let s = State()
    pyxift_input_state_push_button(s.raw, 4, 0, true)
    pyxift_input_state_push_button(s.raw, 4, 1, false)
    pyxift_input_state_push_button(s.raw, 5, 1, true)

    #expect(pyxift_input_state_button(s.raw, 4, 0))
    #expect(!pyxift_input_state_button(s.raw, 4, 1))
    #expect(pyxift_input_state_button(s.raw, 5, 1))
    #expect(!pyxift_input_state_button(s.raw, 5, 0))
}

@Test func keyPressedReleasedTransitions() {
    let s = State()
    let keycode: Int32 = 0x61
    pyxift_input_state_push_key(s.raw, keycode, true)
    #expect(pyxift_input_state_key(s.raw, keycode))
    #expect(pyxift_input_state_key_pressed(s.raw, keycode, 0, 0))

    pyxift_input_state_end_frame(s.raw)
    #expect(pyxift_input_state_key(s.raw, keycode))
    #expect(!pyxift_input_state_key_pressed(s.raw, keycode, 0, 0))

    pyxift_input_state_push_key(s.raw, keycode, false)
    #expect(!pyxift_input_state_key(s.raw, keycode))
    #expect(pyxift_input_state_key_released(s.raw, keycode))
}

@Test func mousePositionAndWheel() {
    let s = State()
    pyxift_input_state_push_mouse_move(s.raw, 42, 13)
    pyxift_input_state_push_mouse_wheel(s.raw, 2)
    pyxift_input_state_push_mouse_wheel(s.raw, -1)

    #expect(pyxift_input_state_mouse_x(s.raw) == 42)
    #expect(pyxift_input_state_mouse_y(s.raw) == 13)
    #expect(pyxift_input_state_mouse_wheel(s.raw) == 1)

    pyxift_input_state_end_frame(s.raw)
    #expect(pyxift_input_state_mouse_x(s.raw) == 42)
    #expect(pyxift_input_state_mouse_wheel(s.raw) == 0)
}

@Test func mouseButtonPressed() {
    let s = State()
    pyxift_input_state_push_mouse_button(s.raw, 0, true)
    #expect(pyxift_input_state_mouse_button(s.raw, 0))
    #expect(pyxift_input_state_mouse_button_pressed(s.raw, 0))
    pyxift_input_state_end_frame(s.raw)
    #expect(pyxift_input_state_mouse_button(s.raw, 0))
    #expect(!pyxift_input_state_mouse_button_pressed(s.raw, 0))
}

@Test func btnpRepeatZeroFiresOnlyOnInitialFrame() {
    let s = State()
    pyxift_input_state_push_button(s.raw, 4, 0, true)

    #expect(pyxift_input_state_button_pressed(s.raw, 4, 0, 0, 0))
    pyxift_input_state_end_frame(s.raw)
    #expect(!pyxift_input_state_button_pressed(s.raw, 4, 0, 0, 0))
    pyxift_input_state_end_frame(s.raw)
    #expect(!pyxift_input_state_button_pressed(s.raw, 4, 0, 0, 0))
}

@Test func btnpRepeatTriggersAtHoldThenInterval() {
    let s = State()
    pyxift_input_state_push_button(s.raw, 4, 0, true)

    #expect(pyxift_input_state_button_pressed(s.raw, 4, 0, 10, 5))

    for _ in 1...9 {
        pyxift_input_state_end_frame(s.raw)
        #expect(!pyxift_input_state_button_pressed(s.raw, 4, 0, 10, 5))
    }

    pyxift_input_state_end_frame(s.raw)
    #expect(pyxift_input_state_button_pressed(s.raw, 4, 0, 10, 5))

    for _ in 1...4 {
        pyxift_input_state_end_frame(s.raw)
        #expect(!pyxift_input_state_button_pressed(s.raw, 4, 0, 10, 5))
    }

    pyxift_input_state_end_frame(s.raw)
    #expect(pyxift_input_state_button_pressed(s.raw, 4, 0, 10, 5))
}

@Test func btnpReleaseStopsRepeat() {
    let s = State()
    pyxift_input_state_push_button(s.raw, 4, 0, true)
    for _ in 0..<5 { pyxift_input_state_end_frame(s.raw) }
    pyxift_input_state_push_button(s.raw, 4, 0, false)

    pyxift_input_state_end_frame(s.raw)
    for _ in 0..<20 {
        #expect(!pyxift_input_state_button_pressed(s.raw, 4, 0, 0, 1))
        pyxift_input_state_end_frame(s.raw)
    }
}

@Test func btnpRepressRestartsClock() {
    let s = State()
    pyxift_input_state_push_button(s.raw, 4, 0, true)
    pyxift_input_state_end_frame(s.raw)
    pyxift_input_state_end_frame(s.raw)
    pyxift_input_state_push_button(s.raw, 4, 0, false)
    pyxift_input_state_end_frame(s.raw)
    pyxift_input_state_push_button(s.raw, 4, 0, true)

    #expect(pyxift_input_state_button_pressed(s.raw, 4, 0, 5, 3))
    for _ in 0..<4 {
        pyxift_input_state_end_frame(s.raw)
        #expect(!pyxift_input_state_button_pressed(s.raw, 4, 0, 5, 3))
    }
    pyxift_input_state_end_frame(s.raw)
    #expect(pyxift_input_state_button_pressed(s.raw, 4, 0, 5, 3))
}

@Test func keyPressedRepeatMatchesButton() {
    let s = State()
    let keycode: Int32 = 0x61
    pyxift_input_state_push_key(s.raw, keycode, true)

    #expect(pyxift_input_state_key_pressed(s.raw, keycode, 3, 2))
    pyxift_input_state_end_frame(s.raw)
    #expect(!pyxift_input_state_key_pressed(s.raw, keycode, 3, 2))
    pyxift_input_state_end_frame(s.raw)
    #expect(!pyxift_input_state_key_pressed(s.raw, keycode, 3, 2))
    pyxift_input_state_end_frame(s.raw)
    #expect(pyxift_input_state_key_pressed(s.raw, keycode, 3, 2))
    pyxift_input_state_end_frame(s.raw)
    #expect(!pyxift_input_state_key_pressed(s.raw, keycode, 3, 2))
    pyxift_input_state_end_frame(s.raw)
    #expect(pyxift_input_state_key_pressed(s.raw, keycode, 3, 2))
}

@Test func gamepadAxisRoundTripNormalization() {
    let s = State()
    pyxift_input_state_push_gamepad_axis(s.raw, 0, 0, 32767)
    pyxift_input_state_push_gamepad_axis(s.raw, 0, 1, -32768)
    pyxift_input_state_push_gamepad_axis(s.raw, 0, 2, 0)

    #expect(pyxift_input_state_gamepad_axis(s.raw, 0, 0) == 1.0)
    #expect(pyxift_input_state_gamepad_axis(s.raw, 0, 1) == -1.0)
    #expect(pyxift_input_state_gamepad_axis(s.raw, 0, 2) == 0.0)
}

@Test func gamepadTriggerClampsToZeroOne() {
    let s = State()
    pyxift_input_state_push_gamepad_axis(s.raw, 0, 4, -100)
    pyxift_input_state_push_gamepad_axis(s.raw, 0, 5, 32767)

    #expect(pyxift_input_state_gamepad_axis(s.raw, 0, 4) == 0.0)
    #expect(pyxift_input_state_gamepad_axis(s.raw, 0, 5) == 1.0)
}

@Test func gamepadAxisPersistsAcrossFrames() {
    let s = State()
    pyxift_input_state_push_gamepad_axis(s.raw, 0, 0, 16384)
    let v0 = pyxift_input_state_gamepad_axis(s.raw, 0, 0)

    pyxift_input_state_end_frame(s.raw)
    pyxift_input_state_end_frame(s.raw)
    #expect(pyxift_input_state_gamepad_axis(s.raw, 0, 0) == v0)
}

@Test func gamepadAxisIsPerPlayer() {
    let s = State()
    pyxift_input_state_push_gamepad_axis(s.raw, 0, 0, 32767)
    pyxift_input_state_push_gamepad_axis(s.raw, 1, 0, -32768)

    #expect(pyxift_input_state_gamepad_axis(s.raw, 0, 0) == 1.0)
    #expect(pyxift_input_state_gamepad_axis(s.raw, 1, 0) == -1.0)
    #expect(pyxift_input_state_gamepad_axis(s.raw, 2, 0) == 0.0)
}
