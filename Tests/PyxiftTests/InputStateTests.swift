import Testing
import CPyxiftCore

// pyxift::InputState（pure C++、SDL3 非依存）の意味論テスト。
// VirtualEvent を直接注入できる pyxift_input_state_* C ブリッジを使う。
//
// Button raw 値: left=0, right=1, up=2, down=3, a=4, b=5, start=6 (Pyxift.Button と一致)
// MouseButton raw 値: left=0, right=1, middle=2

private final class State {
    let h: OpaquePointer
    init() { h = pyxift_input_state_create()! }
    deinit { pyxift_input_state_destroy(h) }

    var raw: OpaquePointer { h }
}

// MARK: - 仮想ボタン: 押下→pressed が 1 フレームだけ立ち、button は維持

@Test func buttonPressedFiresOnceThenStaysHeld() {
    let s = State()
    pyxift_input_state_push_button(s.raw, 4 /* a */, 0, true)

    #expect(pyxift_input_state_button(s.raw, 4, 0))
    #expect(pyxift_input_state_button_pressed(s.raw, 4, 0))
    #expect(!pyxift_input_state_button_released(s.raw, 4, 0))

    pyxift_input_state_end_frame(s.raw)

    // pressed/released は消えるが、button（最終状態）は維持
    #expect(pyxift_input_state_button(s.raw, 4, 0))
    #expect(!pyxift_input_state_button_pressed(s.raw, 4, 0))
    #expect(!pyxift_input_state_button_released(s.raw, 4, 0))
}

@Test func buttonReleaseTransition() {
    let s = State()
    pyxift_input_state_push_button(s.raw, 0 /* left */, 0, true)
    pyxift_input_state_end_frame(s.raw)
    pyxift_input_state_push_button(s.raw, 0, 0, false)

    #expect(!pyxift_input_state_button(s.raw, 0, 0))
    #expect(pyxift_input_state_button_released(s.raw, 0, 0))
}

// 1 フレーム内で press → release → press が起きると、最終状態は true、pressed/released ともに true
@Test func buttonChatteringWithinFrame() {
    let s = State()
    pyxift_input_state_push_button(s.raw, 4, 0, true)
    pyxift_input_state_push_button(s.raw, 4, 0, false)
    pyxift_input_state_push_button(s.raw, 4, 0, true)

    #expect(pyxift_input_state_button(s.raw, 4, 0))
    #expect(pyxift_input_state_button_pressed(s.raw, 4, 0))
    #expect(pyxift_input_state_button_released(s.raw, 4, 0))
}

// 1 フレーム内で press → release で終わった場合、最終状態は false、両方 true
@Test func buttonChatteringEndsReleased() {
    let s = State()
    pyxift_input_state_push_button(s.raw, 4, 0, true)
    pyxift_input_state_push_button(s.raw, 4, 0, false)

    #expect(!pyxift_input_state_button(s.raw, 4, 0))
    #expect(pyxift_input_state_button_pressed(s.raw, 4, 0))
    #expect(pyxift_input_state_button_released(s.raw, 4, 0))
}

// プレイヤーごとに状態が独立している
@Test func buttonsArePerPlayer() {
    let s = State()
    pyxift_input_state_push_button(s.raw, 4, 0, true)
    pyxift_input_state_push_button(s.raw, 4, 1, false) // 既に false なので無効
    pyxift_input_state_push_button(s.raw, 5 /* b */, 1, true)

    #expect(pyxift_input_state_button(s.raw, 4, 0))
    #expect(!pyxift_input_state_button(s.raw, 4, 1))
    #expect(pyxift_input_state_button(s.raw, 5, 1))
    #expect(!pyxift_input_state_button(s.raw, 5, 0))
}

// MARK: - キー

@Test func keyPressedReleasedTransitions() {
    let s = State()
    let keycode: Int32 = 0x61 // 'a'
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

// MARK: - マウス

@Test func mousePositionAndWheel() {
    let s = State()
    pyxift_input_state_push_mouse_move(s.raw, 42, 13)
    pyxift_input_state_push_mouse_wheel(s.raw, 2)
    pyxift_input_state_push_mouse_wheel(s.raw, -1)

    #expect(pyxift_input_state_mouse_x(s.raw) == 42)
    #expect(pyxift_input_state_mouse_y(s.raw) == 13)
    #expect(pyxift_input_state_mouse_wheel(s.raw) == 1) // 加算

    pyxift_input_state_end_frame(s.raw)
    // 位置は維持、wheel はリセット
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
