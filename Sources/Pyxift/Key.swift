// SDL3 SDL_Keycode と 1:1 対応する物理キー識別子。
// 命名は Swift 慣用の lowerCamelCase（decisions.md 末尾「キーコード定数の網羅」）。
// 生の SDL3 値を直接渡したい場合は `Key(rawValue:)` を使う。
//
// v0.1 では「ゲームを書く最低限」の主要キーを網羅。SDL3 の全 215 個は
// v0.1.x で `SDL_keycode.h` からの自動生成スクリプトに置き換える予定
// （docs/implementation-plan.md M3 参照）。
public enum Key: Int32, Sendable {
    // 制御
    case escape = 0x0000001b       // SDLK_ESCAPE
    case tab = 0x00000009
    case backspace = 0x00000008
    case `return` = 0x0000000d
    case space = 0x00000020

    // 矢印
    case rightArrow = 0x4000004f
    case leftArrow  = 0x40000050
    case downArrow  = 0x40000051
    case upArrow    = 0x40000052

    // 編集系
    case insert   = 0x40000049
    case home     = 0x4000004a
    case pageUp   = 0x4000004b
    case delete   = 0x0000007f
    case end      = 0x4000004d
    case pageDown = 0x4000004e

    // 文字
    case digit0 = 0x00000030
    case digit1 = 0x00000031
    case digit2 = 0x00000032
    case digit3 = 0x00000033
    case digit4 = 0x00000034
    case digit5 = 0x00000035
    case digit6 = 0x00000036
    case digit7 = 0x00000037
    case digit8 = 0x00000038
    case digit9 = 0x00000039

    case a = 0x00000061
    case b = 0x00000062
    case c = 0x00000063
    case d = 0x00000064
    case e = 0x00000065
    case f = 0x00000066
    case g = 0x00000067
    case h = 0x00000068
    case i = 0x00000069
    case j = 0x0000006a
    case k = 0x0000006b
    case l = 0x0000006c
    case m = 0x0000006d
    case n = 0x0000006e
    case o = 0x0000006f
    case p = 0x00000070
    case q = 0x00000071
    case r = 0x00000072
    case s = 0x00000073
    case t = 0x00000074
    case u = 0x00000075
    case v = 0x00000076
    case w = 0x00000077
    case x = 0x00000078
    case y = 0x00000079
    case z = 0x0000007a

    // ファンクション
    case f1  = 0x4000003a
    case f2  = 0x4000003b
    case f3  = 0x4000003c
    case f4  = 0x4000003d
    case f5  = 0x4000003e
    case f6  = 0x4000003f
    case f7  = 0x40000040
    case f8  = 0x40000041
    case f9  = 0x40000042
    case f10 = 0x40000043
    case f11 = 0x40000044
    case f12 = 0x40000045

    // 修飾キー
    case leftCtrl   = 0x400000e0
    case leftShift  = 0x400000e1
    case leftAlt    = 0x400000e2
    case leftGui    = 0x400000e3
    case rightCtrl  = 0x400000e4
    case rightShift = 0x400000e5
    case rightAlt   = 0x400000e6
    case rightGui   = 0x400000e7
}
