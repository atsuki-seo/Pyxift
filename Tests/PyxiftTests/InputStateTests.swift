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

private func readInputText(_ s: State) -> String {
    let len = Int(pyxift_input_state_input_text(s.raw, nil, 0))
    if len == 0 { return "" }
    var buf = [UInt8](repeating: 0, count: len + 1)
    buf.withUnsafeMutableBufferPointer { p in
        _ = p.baseAddress?.withMemoryRebound(to: CChar.self, capacity: len + 1) { cp in
            pyxift_input_state_input_text(s.raw, cp, Int32(len + 1))
        }
    }
    buf.removeLast()
    return String(decoding: buf, as: UTF8.self)
}

private func readDroppedFile(_ s: State, _ i: Int32) -> String? {
    let len = Int(pyxift_input_state_dropped_files_at(s.raw, i, nil, 0))
    if len < 0 { return nil }
    if len == 0 { return "" }
    var buf = [UInt8](repeating: 0, count: len + 1)
    buf.withUnsafeMutableBufferPointer { p in
        _ = p.baseAddress?.withMemoryRebound(to: CChar.self, capacity: len + 1) { cp in
            pyxift_input_state_dropped_files_at(s.raw, i, cp, Int32(len + 1))
        }
    }
    buf.removeLast()
    return String(decoding: buf, as: UTF8.self)
}

@Test func inputTextAccumulatesAndClearsOnFrame() {
    let s = State()
    pyxift_input_state_push_text(s.raw, "Hi")
    pyxift_input_state_push_text(s.raw, " 日本語")
    #expect(readInputText(s) == "Hi 日本語")

    pyxift_input_state_end_frame(s.raw)
    #expect(readInputText(s) == "")
}

@Test func inputTextTruncatesAndReportsFullLength() {
    let s = State()
    pyxift_input_state_push_text(s.raw, "abcdef")

    var buf = [CChar](repeating: 99, count: 4)
    let reported = pyxift_input_state_input_text(s.raw, &buf, 4)
    #expect(reported == 6)
    #expect(buf[0] == 0x61)
    #expect(buf[1] == 0x62)
    #expect(buf[2] == 0x63)
    #expect(buf[3] == 0)
}

@Test func inputKeysAccumulatesAndClearsOnFrame() {
    let s = State()
    pyxift_input_state_push_input_key(s.raw, 0x61)
    pyxift_input_state_push_input_key(s.raw, 0x62)
    pyxift_input_state_push_input_key(s.raw, 0x63)

    #expect(pyxift_input_state_input_keys_count(s.raw) == 3)
    #expect(pyxift_input_state_input_keys_at(s.raw, 0) == 0x61)
    #expect(pyxift_input_state_input_keys_at(s.raw, 1) == 0x62)
    #expect(pyxift_input_state_input_keys_at(s.raw, 2) == 0x63)

    pyxift_input_state_end_frame(s.raw)
    #expect(pyxift_input_state_input_keys_count(s.raw) == 0)
}

@Test func droppedFilesAccumulatesAndClearsOnFrame() {
    let s = State()
    pyxift_input_state_push_dropped_file(s.raw, "/tmp/a.png")
    pyxift_input_state_push_dropped_file(s.raw, "/tmp/別.txt")

    #expect(pyxift_input_state_dropped_files_count(s.raw) == 2)
    #expect(readDroppedFile(s, 0) == "/tmp/a.png")
    #expect(readDroppedFile(s, 1) == "/tmp/別.txt")
    #expect(readDroppedFile(s, 5) == nil)

    pyxift_input_state_end_frame(s.raw)
    #expect(pyxift_input_state_dropped_files_count(s.raw) == 0)
}

@Test func setMousePosUpdatesPositionImmediately() {
    let s = State()
    pyxift_input_state_set_mouse_pos(s.raw, 17, 42)

    #expect(pyxift_input_state_mouse_x(s.raw) == 17)
    #expect(pyxift_input_state_mouse_y(s.raw) == 42)

    pyxift_input_state_end_frame(s.raw)
    #expect(pyxift_input_state_mouse_x(s.raw) == 17)
    #expect(pyxift_input_state_mouse_y(s.raw) == 42)
}
