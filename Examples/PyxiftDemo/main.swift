import Pyxift

struct Demo: App {
    mutating func update() {
        Pyx.title("Pyxift  frame=\(Pyx.frameCount)")
    }

    func draw() {
        Pyx.cls(color: .navy)

        // ---- M2a プリミティブ（左上ブロック） ----
        Pyx.pset(x: Pyx.width / 2, y: Pyx.height / 2, color: .white)

        Pyx.line(x1: 4, y1: 4, x2: 60, y2: 30, color: .red)
        Pyx.line(x1: 60, y1: 4, x2: 4, y2: 30, color: .yellow)

        Pyx.rect(x: 70, y: 4, w: 24, h: 16, color: .green)
        Pyx.rectb(x: 100, y: 4, w: 24, h: 16, color: .lime)

        Pyx.circ(x: 20, y: 60, r: 10, color: .orange)
        Pyx.circb(x: 50, y: 60, r: 10, color: .pink)

        Pyx.trib(x1: 80, y1: 50, x2: 110, y2: 50, x3: 95, y3: 80, color: .cyan)
        Pyx.tri(x1: 120, y1: 50, x2: 150, y2: 50, x3: 135, y3: 80, color: .peach)

        Pyx.rectb(x: -4, y: 95, w: 30, h: 30, color: .white)
        Pyx.circb(x: Pyx.width + 5, y: 105, r: 12, color: .white)

        // ---- M2b: clip ----
        // 帯状の clip 矩形を設置し、はみ出す円が縦帯にだけ見える状態を作る。
        Pyx.clip(x: 30, y: 90, w: 30, h: 25)
        Pyx.circ(x: 45, y: 102, r: 20, color: .red)        // clip 範囲外は描かれない
        Pyx.rectb(x: 30, y: 90, w: 30, h: 25, color: .white)  // 枠は clip 自体とぴったり
        Pyx.clip()  // reset

        // ---- M2b: camera ----
        // camera(-90, -90) で右下に矩形が動く（API 座標 0,0 → フレームバッファ 90,90）。
        Pyx.camera(x: -90, y: -90)
        Pyx.rect(x: 0, y: 0, w: 12, h: 12, color: .yellow)
        Pyx.line(x1: 0, y1: 0, x2: 12, y2: 12, color: .black)
        Pyx.camera()  // reset

        // ---- M2b: pal ----
        // navy(1) を red(8) に差し替えた状態で矩形を描き、すぐ reset。
        Pyx.pal(from: .navy, to: .red)
        Pyx.rect(x: 130, y: 95, w: 24, h: 20, color: .navy)  // 実際は赤で塗られる
        Pyx.pal()
    }
}

Pyx.run(Demo(), width: 160, height: 120, title: "Pyxift")
