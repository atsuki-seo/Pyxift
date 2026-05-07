import Pyxift

// PyxiftDemo
// Tab キーでシーン切り替え：
//   0: M2 描画プリミティブ（line/rect/circ/tri/blt/bltm/text）
//   1: M3 入力可視化（仮想ボタン・キー・マウス）

enum Scene: Int {
    case drawing = 0
    case input = 1

    var next: Scene {
        switch self {
        case .drawing: return .input
        case .input: return .drawing
        }
    }
}

struct Demo: App {
    var initialized = false
    var scene: Scene = .input

    // pressed / released フラッシュ用：イベントが発火したフレーム番号を記録。
    var lastButtonPressedFrame: [Button: Int] = [:]
    var lastButtonReleasedFrame: [Button: Int] = [:]
    var lastMouseWheelFrame: Int = -100
    var lastMouseWheel: Int = 0

    mutating func update() {
        if !initialized {
            buildAssets()
            initialized = true
        }
        if Pyx.keyPressed(.tab) {
            scene = scene.next
        }
        // フラッシュ表示用に「直近で pressed/released した時刻」を記録
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

        // 画面右下にシーン切り替えのヒントを常時表示
        Pyx.text(x: Pyx.width - 80, y: Pyx.height - 8, "TAB:SWITCH", color: .gray)

        switch scene {
        case .drawing: drawDrawingScene()
        case .input:   drawInputScene()
        }
    }

    // MARK: - Scene 0: 描画プリミティブ
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

    // MARK: - Scene 1: 入力可視化
    private func drawInputScene() {
        Pyx.text(x: 4, y: 4, "M3 INPUT", color: .yellow)
        Pyx.text(x: 4, y: 14, "P0 KEYBOARD+PAD", color: .gray)

        // ---- ゲームパッド風レイアウト（左に十字キー・右に A/B/START） ----
        let padX = 20, padY = 40
        drawDpad(centerX: padX, centerY: padY)

        let abX = 100, abY = 40
        drawFaceButton(x: abX,      y: abY + 10, button: .a, label: "A")
        drawFaceButton(x: abX + 24, y: abY,      button: .b, label: "B")

        // START は中央に少し小さめに
        drawStartButton(x: 60, y: padY + 4)

        // ---- マウス可視化（右側のミニマップ風枠） ----
        drawMouseArea(x: 150, y: 32, w: Pyx.width - 154, h: 60)

        // ---- 押下中キーの一覧（下部） ----
        drawKeyDisplay(x: 4, y: Pyx.height - 30)
    }

    private func drawDpad(centerX cx: Int, centerY cy: Int) {
        // 1 マス 12px の十字。各方向ボタンを矩形で表示。
        let s = 12
        // 中心（飾り）
        Pyx.rectb(x: cx - s/2, y: cy - s/2, w: s, h: s, color: .gray)
        // 上
        drawArrow(x: cx - s/2, y: cy - s - s/2, w: s, h: s, button: .up, glyph: "^")
        // 下
        drawArrow(x: cx - s/2, y: cy + s/2,     w: s, h: s, button: .down, glyph: "v")
        // 左
        drawArrow(x: cx - s - s/2, y: cy - s/2, w: s, h: s, button: .left, glyph: "<")
        // 右
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
        // 円形ボタン。押下中は赤塗り、通常は枠のみ。
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

    // pressed / released を 6 フレームの色付き枠で表示。
    private func drawEdgeFlash(x: Int, y: Int, w: Int, h: Int, button: Button) {
        let now = Pyx.frameCount
        let kFlashFrames = 6
        if let f = lastButtonPressedFrame[button], now - f < kFlashFrames {
            // 押された瞬間 = 緑のリング（外側に膨らむ）
            let pad = 2 + (now - f)
            Pyx.rectb(x: x - pad, y: y - pad, w: w + pad * 2, h: h + pad * 2, color: .lime)
        }
        if let f = lastButtonReleasedFrame[button], now - f < kFlashFrames {
            // 離された瞬間 = 黄色のリング
            let pad = 2 + (now - f)
            Pyx.rectb(x: x - pad, y: y - pad, w: w + pad * 2, h: h + pad * 2, color: .yellow)
        }
    }

    private func drawMouseArea(x: Int, y: Int, w: Int, h: Int) {
        Pyx.rectb(x: x, y: y, w: w, h: h, color: .gray)
        Pyx.text(x: x + 2, y: y - 8, "MOUSE", color: .gray)

        let m = Pyx.mouse()
        // 枠内ならクロスヘア、枠外でも座標は表示
        let inside = m.x >= x && m.x < x + w && m.y >= y && m.y < y + h
        if inside {
            Pyx.line(x1: x, y1: m.y, x2: x + w - 1, y2: m.y, color: .darkBlue)
            Pyx.line(x1: m.x, y1: y, x2: m.x, y2: y + h - 1, color: .darkBlue)
            // 中心点を強調
            Pyx.rect(x: m.x - 1, y: m.y - 1, w: 3, h: 3, color: .yellow)
        }

        // ボタン状態
        let l = Pyx.mouseButton(.left)   ? "L" : "."
        let r = Pyx.mouseButton(.right)  ? "R" : "."
        let mb = Pyx.mouseButton(.middle) ? "M" : "."
        Pyx.text(x: x + 2, y: y + h + 2, "BTN \(l)\(r)\(mb)", color: .white)
        // 座標数値
        Pyx.text(x: x + 2, y: y + h + 10, "X\(m.x) Y\(m.y)", color: .lightBlue)

        // wheel: 直近の値を 12 フレーム表示
        let now = Pyx.frameCount
        if now - lastMouseWheelFrame < 12 {
            Pyx.text(x: x + 2, y: y + h + 18, "WHEEL \(lastMouseWheel)", color: .lime)
        }
    }

    private func drawKeyDisplay(x: Int, y: Int) {
        Pyx.text(x: x, y: y, "KEYS HELD:", color: .gray)
        // 監視するキーの一覧（押下中のみ表示）
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
