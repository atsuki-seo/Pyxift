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
    #expect(pyxift_input_state_button_pressed(s.raw, 4, 0))
    #expect(!pyxift_input_state_button_released(s.raw, 4, 0))

    pyxift_input_state_end_frame(s.raw)

    #expect(pyxift_input_state_button(s.raw, 4, 0))
    #expect(!pyxift_input_state_button_pressed(s.raw, 4, 0))
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
    #expect(pyxift_input_state_button_pressed(s.raw, 4, 0))
    #expect(pyxift_input_state_button_released(s.raw, 4, 0))
}

@Test func buttonChatteringEndsReleased() {
    let s = State()
    pyxift_input_state_push_button(s.raw, 4, 0, true)
    pyxift_input_state_push_button(s.raw, 4, 0, false)

    #expect(!pyxift_input_state_button(s.raw, 4, 0))
    #expect(pyxift_input_state_button_pressed(s.raw, 4, 0))
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
    #expect(pyxift_input_state_key_pressed(s.raw, keycode))

    pyxift_input_state_end_frame(s.raw)
    #expect(pyxift_input_state_key(s.raw, keycode))
    #expect(!pyxift_input_state_key_pressed(s.raw, keycode))

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
