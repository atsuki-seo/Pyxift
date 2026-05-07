import Pyxift

struct Demo: App {
    mutating func update() {
        Pyx.title("Pyxift  frame=\(Pyx.frameCount)")
    }

    func draw() {
        Pyx.cls(color: .navy)

        // pset: 中央に1ドット
        Pyx.pset(x: Pyx.width / 2, y: Pyx.height / 2, color: .white)

        // line: 4色で四方
        Pyx.line(x1: 4, y1: 4, x2: 60, y2: 30, color: .red)
        Pyx.line(x1: 60, y1: 4, x2: 4, y2: 30, color: .yellow)

        // rect / rectb
        Pyx.rect(x: 70, y: 4, w: 24, h: 16, color: .green)
        Pyx.rectb(x: 100, y: 4, w: 24, h: 16, color: .lime)

        // circ / circb
        Pyx.circ(x: 20, y: 60, r: 10, color: .orange)
        Pyx.circb(x: 50, y: 60, r: 10, color: .pink)

        // tri / trib（M2a 段階では tri は線のみ）
        Pyx.trib(x1: 80, y1: 50, x2: 110, y2: 50, x3: 95, y3: 80, color: .cyan)
        Pyx.tri(x1: 120, y1: 50, x2: 150, y2: 50, x3: 135, y3: 80, color: .peach)

        // 画面端クリップの確認: はみ出る矩形
        Pyx.rectb(x: -4, y: 95, w: 30, h: 30, color: .white)
        Pyx.circb(x: Pyx.width + 5, y: 105, r: 12, color: .white)
    }
}

Pyx.run(Demo(), width: 160, height: 120, title: "Pyxift")
