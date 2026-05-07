import Pyxift
import Foundation

enum Scene: Int {
    case drawing = 0
    case input = 1
    case assets = 2
    case bouncing = 3

    var next: Scene {
        switch self {
        case .drawing:  return .input
        case .input:    return .assets
        case .assets:   return .bouncing
        case .bouncing: return .drawing
        }
    }
}

struct Ball {
    var x: Double
    var y: Double
    var vx: Double
    var vy: Double
    var radius: Int
    var color: Color
}

struct Demo: App {
    var initialized = false
    var scene: Scene = .bouncing

    var lastButtonPressedFrame: [Button: Int] = [:]
    var lastButtonReleasedFrame: [Button: Int] = [:]
    var lastMouseWheelFrame: Int = -100
    var lastMouseWheel: Int = 0

    var balls: [Ball] = []

    mutating func update() {
        if !initialized {
            buildAssets()
            spawnBalls()
            initialized = true
        }
        if Pyx.keyPressed(.tab) {
            scene = scene.next
        }
        if scene == .bouncing {
            updateBalls()
        }
        for b in Button.all {
            if Pyx.buttonPressed(b) {
                lastButtonPressedFrame[b] = Pyx.frameCount
            }
            if Pyx.buttonReleased(b) {
                lastButtonReleasedFrame[b] = Pyx.frameCount
            }
        }
        if Pyx.mouseWheel != 0 {
            lastMouseWheelFrame = Pyx.frameCount
            lastMouseWheel = Pyx.mouseWheel
        }
        Pyx.title("Pyxift  scene=\(scene)  frame=\(Pyx.frameCount)")
    }

    func draw() {
        Pyx.cls(color: .black)

        Pyx.text(x: Pyx.width - 80, y: Pyx.height - 8, "TAB:SWITCH", color: .gray)

        switch scene {
        case .drawing:  drawDrawingScene()
        case .input:    drawInputScene()
        case .assets:   drawAssetsScene()
        case .bouncing: drawBouncingScene()
        }
    }

    private mutating func spawnBalls() {
        let palette: [Color] = [.red, .yellow, .lime, .cyan, .pink, .orange, .lightBlue, .white]
        // 起動ごとの再現性を担保するため、Foundation の乱数ではなく固定シードの線形合同法を使う。
        var seed: UInt32 = 0x9E37_79B9
        func next() -> Double {
            seed = seed &* 1_664_525 &+ 1_013_904_223
            return Double(seed) / Double(UInt32.max)
        }
        balls.removeAll()
        for i in 0..<8 {
            let r = 3 + Int(next() * 3)
            let x = Double(r) + next() * Double(Pyx.width  - 2 * r)
            let y = Double(r) + next() * Double(Pyx.height - 2 * r)
            let vx = (next() * 2 - 1) * 1.8 + 0.4
            let vy = (next() * 2 - 1) * 1.4 + 0.3
            balls.append(Ball(x: x, y: y, vx: vx, vy: vy, radius: r, color: palette[i % palette.count]))
        }
    }

    private mutating func updateBalls() {
        let w = Double(Pyx.width)
        let h = Double(Pyx.height)
        for i in balls.indices {
            balls[i].x += balls[i].vx
            balls[i].y += balls[i].vy
            let r = Double(balls[i].radius)
            if balls[i].x < r {
                balls[i].x = r
                balls[i].vx = -balls[i].vx
            } else if balls[i].x > w - 1 - r {
                balls[i].x = w - 1 - r
                balls[i].vx = -balls[i].vx
            }
            if balls[i].y < r {
                balls[i].y = r
                balls[i].vy = -balls[i].vy
            } else if balls[i].y > h - 1 - r {
                balls[i].y = h - 1 - r
                balls[i].vy = -balls[i].vy
            }
        }
    }

    private func drawBouncingScene() {
        Pyx.text(x: 4, y: 4, "M5 BOUNCING BALLS", color: .yellow)
        Pyx.text(x: 4, y: 12, "FRAME=\(Pyx.frameCount)", color: .gray)
        Pyx.rectb(x: 0, y: 0, w: Pyx.width, h: Pyx.height, color: .darkBlue)
        for b in balls {
            Pyx.circ(x: Int(b.x), y: Int(b.y), r: b.radius, color: b.color)
        }
    }

    private func drawAssetsScene() {
        Pyx.text(x: 4, y: 4, "ASSETS", color: .yellow)

        Pyx.text(x: 4, y: 16, "BUILTIN FONT", color: .gray)
        let originX = 4, originY = 26
        let cellW = 5, cellH = 7
        for code in 0x20...0x7e {
            let i = code - 0x20
            let cx = originX + (i % 16) * cellW
            let cy = originY + (i / 16) * cellH
            let s = String(UnicodeScalar(code)!)
            Pyx.text(x: cx, y: cy, s, color: .white)
        }

        Pyx.text(x: 4, y: 78, "LOADED PNG (BANK 1)", color: .gray)
        Pyx.blt(x: 4, y: 88, image: 1, u: 0, v: 0, w: 32, h: 32, transparent: nil)
        Pyx.pal(from: .red, to: .lime)
        Pyx.blt(x: 40, y: 88, image: 1, u: 0, v: 0, w: 32, h: 32, transparent: .black)
        Pyx.pal()

        Pyx.text(x: 80, y: 90,  "FONT+PNG", color: .yellow)
        Pyx.text(x: 80, y: 100, "M4 OK!",   color: .lime)
        Pyx.text(x: 80, y: 110, "FRAME=\(Pyx.frameCount)", color: .lightBlue)
    }

    private func drawDrawingScene() {
        Pyx.text(x: 4, y: 4, "M2 PRIMITIVES", color: .yellow)

        Pyx.line(x1: 4, y1: 16, x2: 60, y2: 42, color: .red)
        Pyx.line(x1: 60, y1: 16, x2: 4, y2: 42, color: .yellow)

        Pyx.rect(x: 70, y: 16, w: 24, h: 16, color: .green)
        Pyx.rectb(x: 100, y: 16, w: 24, h: 16, color: .lime)

        Pyx.circ(x: 20, y: 70, r: 10, color: .orange)
        Pyx.circb(x: 50, y: 70, r: 10, color: .pink)

        Pyx.trib(x1: 80, y1: 60, x2: 110, y2: 60, x3: 95, y3: 90, color: .cyan)
        Pyx.tri(x1: 120, y1: 60, x2: 150, y2: 60, x3: 135, y3: 90, color: .peach)

        Pyx.blt(x: 4, y: 100, image: 0, u: 0, v: 0, w: 16, h: 16, transparent: .black)
        Pyx.pal(from: .red, to: .yellow)
        Pyx.blt(x: 24, y: 100, image: 0, u: 0, v: 0, w: 16, h: 16, transparent: .black)
        Pyx.pal()

        Pyx.bltm(x: 50, y: 100, tilemap: 0, u: 0, v: 0, w: 3, h: 2, transparent: .black)

        Pyx.text(x: 100, y: 105, "PYXIFT", color: .white)
        Pyx.text(x: 100, y: 115, "M2 OK",  color: .yellow)
    }

    private func drawInputScene() {
        Pyx.text(x: 4, y: 4, "M3 INPUT", color: .yellow)
        Pyx.text(x: 4, y: 14, "P0 KEYBOARD+PAD", color: .gray)

        let padX = 20, padY = 40
        drawDpad(centerX: padX, centerY: padY)

        let abX = 100, abY = 40
        drawFaceButton(x: abX,      y: abY + 10, button: .a, label: "A")
        drawFaceButton(x: abX + 24, y: abY,      button: .b, label: "B")

        drawStartButton(x: 60, y: padY + 4)

        drawMouseArea(x: 150, y: 32, w: Pyx.width - 154, h: 60)

        drawKeyDisplay(x: 4, y: Pyx.height - 30)
    }

    private func drawDpad(centerX cx: Int, centerY cy: Int) {
        let s = 12
        Pyx.rectb(x: cx - s/2, y: cy - s/2, w: s, h: s, color: .gray)
        drawArrow(x: cx - s/2, y: cy - s - s/2, w: s, h: s, button: .up, glyph: "^")
        drawArrow(x: cx - s/2, y: cy + s/2,     w: s, h: s, button: .down, glyph: "v")
        drawArrow(x: cx - s - s/2, y: cy - s/2, w: s, h: s, button: .left, glyph: "<")
        drawArrow(x: cx + s/2,     y: cy - s/2, w: s, h: s, button: .right, glyph: ">")
    }

    private func drawArrow(x: Int, y: Int, w: Int, h: Int, button: Button, glyph: String) {
        let pressed = Pyx.button(button)
        let fill: Color = pressed ? .red : .navy
        Pyx.rect(x: x, y: y, w: w, h: h, color: fill)
        Pyx.rectb(x: x, y: y, w: w, h: h, color: .white)
        Pyx.text(x: x + (w - 4) / 2, y: y + (h - 6) / 2, glyph, color: .white)
        drawEdgeFlash(x: x, y: y, w: w, h: h, button: button)
    }

    private func drawFaceButton(x: Int, y: Int, button: Button, label: String) {
        let r = 8
        let pressed = Pyx.button(button)
        if pressed {
            Pyx.circ(x: x + r, y: y + r, r: r, color: .red)
        } else {
            Pyx.circ(x: x + r, y: y + r, r: r, color: .darkBlue)
        }
        Pyx.circb(x: x + r, y: y + r, r: r, color: .white)
        Pyx.text(x: x + r - 2, y: y + r - 3, label, color: .white)
        drawEdgeFlash(x: x, y: y, w: r * 2 + 1, h: r * 2 + 1, button: button)
    }

    private func drawStartButton(x: Int, y: Int) {
        let w = 30, h = 8
        let pressed = Pyx.button(.start)
        Pyx.rect(x: x, y: y, w: w, h: h, color: pressed ? .red : .darkBlue)
        Pyx.rectb(x: x, y: y, w: w, h: h, color: .white)
        Pyx.text(x: x + 6, y: y + 1, "START", color: .white)
        drawEdgeFlash(x: x, y: y, w: w, h: h, button: .start)
    }

    private func drawEdgeFlash(x: Int, y: Int, w: Int, h: Int, button: Button) {
        let now = Pyx.frameCount
        let kFlashFrames = 6
        if let f = lastButtonPressedFrame[button], now - f < kFlashFrames {
            let pad = 2 + (now - f)
            Pyx.rectb(x: x - pad, y: y - pad, w: w + pad * 2, h: h + pad * 2, color: .lime)
        }
        if let f = lastButtonReleasedFrame[button], now - f < kFlashFrames {
            let pad = 2 + (now - f)
            Pyx.rectb(x: x - pad, y: y - pad, w: w + pad * 2, h: h + pad * 2, color: .yellow)
        }
    }

    private func drawMouseArea(x: Int, y: Int, w: Int, h: Int) {
        Pyx.rectb(x: x, y: y, w: w, h: h, color: .gray)
        Pyx.text(x: x + 2, y: y - 8, "MOUSE", color: .gray)

        let m = Pyx.mouse()
        let inside = m.x >= x && m.x < x + w && m.y >= y && m.y < y + h
        if inside {
            Pyx.line(x1: x, y1: m.y, x2: x + w - 1, y2: m.y, color: .darkBlue)
            Pyx.line(x1: m.x, y1: y, x2: m.x, y2: y + h - 1, color: .darkBlue)
            Pyx.rect(x: m.x - 1, y: m.y - 1, w: 3, h: 3, color: .yellow)
        }

        let l = Pyx.mouseButton(.left)   ? "L" : "."
        let r = Pyx.mouseButton(.right)  ? "R" : "."
        let mb = Pyx.mouseButton(.middle) ? "M" : "."
        Pyx.text(x: x + 2, y: y + h + 2, "BTN \(l)\(r)\(mb)", color: .white)
        Pyx.text(x: x + 2, y: y + h + 10, "X\(m.x) Y\(m.y)", color: .lightBlue)

        let now = Pyx.frameCount
        if now - lastMouseWheelFrame < 12 {
            Pyx.text(x: x + 2, y: y + h + 18, "WHEEL \(lastMouseWheel)", color: .lime)
        }
    }

    private func drawKeyDisplay(x: Int, y: Int) {
        Pyx.text(x: x, y: y, "KEYS HELD:", color: .gray)
        let watch: [(Key, String)] = [
            (.space, "SPC"), (.return, "ENT"), (.tab, "TAB"),
            (.leftShift, "LSH"), (.leftCtrl, "LCT"), (.leftAlt, "LAL"),
            (.a, "A"), (.s, "S"), (.d, "D"), (.w, "W"),
            (.z, "Z"), (.x, "X"), (.c, "C"),
            (.upArrow, "UP"), (.downArrow, "DN"),
            (.leftArrow, "LF"), (.rightArrow, "RG"),
            (.f1, "F1"), (.f2, "F2"),
        ]
        var col = 0
        let cellW = 16
        let cellH = 8
        let perRow = (Pyx.width - x) / cellW
        for (k, label) in watch where Pyx.key(k) {
            let cx = x + (col % perRow) * cellW
            let cy = y + 8 + (col / perRow) * cellH
            Pyx.rect(x: cx, y: cy, w: cellW - 2, h: cellH - 1, color: .darkBlue)
            Pyx.text(x: cx + 1, y: cy + 1, label, color: .white)
            col += 1
        }
        if col == 0 {
            Pyx.text(x: x, y: y + 8, "(press any key)", color: .gray)
        }
    }

    private func buildAssets() {
        for x in 0..<16 {
            for y in 0..<16 {
                let isEdge = (x == 0 || y == 0 || x == 15 || y == 15)
                let color: Color = isEdge ? .red : .white
                Pyx.imagePset(bank: 0, x: x, y: y, color: color)
            }
        }
        for x in 0..<4 {
            for y in 0..<4 {
                Pyx.imagePset(bank: 0, x: x, y: y, color: .black)
            }
        }
        for ty in 0..<8 {
            for tx in 0..<8 {
                Pyx.imagePset(bank: 0, x: 0 * 8 + tx, y: 2 * 8 + ty, color: .green)
                Pyx.imagePset(bank: 0, x: 1 * 8 + tx, y: 2 * 8 + ty, color: .red)
                Pyx.imagePset(bank: 0, x: 2 * 8 + tx, y: 2 * 8 + ty, color: .black)
            }
        }
        if let url = Bundle.module.url(forResource: "sample", withExtension: "png", subdirectory: "assets") {
            Pyx.loadImage(url.path, into: 1)
        }

        Pyx.tilemapSetImageBank(tilemap: 0, bank: 0)
        let layout: [[(Int, Int)]] = [
            [(0, 2), (1, 2), (2, 2)],
            [(1, 2), (2, 2), (0, 2)],
        ]
        for (cy, row) in layout.enumerated() {
            for (cx, tile) in row.enumerated() {
                Pyx.tilemapSetCell(tilemap: 0, cx: cx, cy: cy, tileX: tile.0, tileY: tile.1)
            }
        }
    }
}

private extension Button {
    static let all: [Button] = [.left, .right, .up, .down, .a, .b, .start]
}

Pyx.run(Demo(), width: 220, height: 160, title: "Pyxift")
