# Swift API ラフスケッチ

実装前のメンタルモデル合わせ用。シグネチャは仮で、実装時に微調整する。

## ライフサイクル

```swift
public protocol App {
    mutating func update()
    func draw()
}

public enum Pyx {
    public static func run<A: App>(_ app: A, width: Int = 160, height: Int = 120,
                                    title: String = "Pyxift", fps: Int = 30)
}
```

## 描画

```swift
extension Pyx {
    public static func cls(color: Color)
    public static func pset(x: Int, y: Int, color: Color)
    public static func line(x1: Int, y1: Int, x2: Int, y2: Int, color: Color)
    public static func rect(x: Int, y: Int, w: Int, h: Int, color: Color)
    public static func rectb(x: Int, y: Int, w: Int, h: Int, color: Color)
    public static func circ(x: Int, y: Int, r: Int, color: Color)
    public static func circb(x: Int, y: Int, r: Int, color: Color)
    public static func tri(x1: Int, y1: Int, x2: Int, y2: Int, x3: Int, y3: Int, color: Color)
    public static func trib(x1: Int, y1: Int, x2: Int, y2: Int, x3: Int, y3: Int, color: Color)
    public static func text(x: Int, y: Int, _ s: String, color: Color)

    public static func blt(x: Int, y: Int, image: Int, u: Int, v: Int, w: Int, h: Int,
                           transparent: Color? = nil)
    public static func bltm(x: Int, y: Int, tilemap: Int, u: Int, v: Int, w: Int, h: Int,
                            transparent: Color? = nil)

    public static func clip(x: Int, y: Int, w: Int, h: Int)
    public static func clip()  // reset
    public static func camera(x: Int, y: Int)
    public static func camera()  // reset
    public static func pal(from: Color, to: Color)
    public static func pal()  // reset
}
```

## 入力

```swift
public enum Button {
    case left, right, up, down, a, b, start
}

public enum Key { /* SDL3 keycode と1:1 */ }

public enum MouseButton { case left, right, middle }

extension Pyx {
    public static func button(_ b: Button, player: Int = 0) -> Bool
    public static func buttonPressed(_ b: Button, player: Int = 0) -> Bool   // このフレームだけ true
    public static func buttonReleased(_ b: Button, player: Int = 0) -> Bool
    public static func key(_ k: Key) -> Bool
    public static func mouse() -> (x: Int, y: Int)
    public static func mouseButton(_ b: MouseButton) -> Bool
}
```

## 色

```swift
public struct Color: Equatable, ExpressibleByIntegerLiteral {
    public let index: UInt8  // 0..15
    public init(integerLiteral value: Int) { self.index = UInt8(value) }
}

extension Color {
    public static let black: Color = 0
    public static let navy: Color = 1
    public static let purple: Color = 2
    public static let green: Color = 3
    public static let brown: Color = 4
    public static let darkBlue: Color = 5
    public static let lightBlue: Color = 6
    public static let white: Color = 7
    public static let red: Color = 8
    public static let orange: Color = 9
    public static let yellow: Color = 10
    public static let lime: Color = 11
    public static let cyan: Color = 12
    public static let gray: Color = 13
    public static let pink: Color = 14
    public static let peach: Color = 15
}
```

`Pyx.cls(color: 0)` も `Pyx.cls(color: .black)` も書けるよう `ExpressibleByIntegerLiteral` で両対応。

## アセット

```swift
extension Pyx {
    public static func loadImage(_ path: String, into bank: Int = 0)  // v0.1
    public static func load(_ path: String)                            // v0.2 以降（.pyxift）
}
```

## サウンド（v0.2 以降）

```swift
extension Pyx {
    public static func sound(_ index: Int, notes: String, tones: String,
                             volumes: String, effects: String, speed: Int = 30)
    public static func play(channel: Int, sound: Int, loop: Bool = false)
    public static func stop(channel: Int)
}
```
