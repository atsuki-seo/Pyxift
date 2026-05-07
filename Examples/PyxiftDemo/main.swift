import Pyxift

struct Demo: App {
    var initialized = false

    mutating func update() {
        if !initialized {
            buildAssets()
            initialized = true
        }
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

        // M2c: tri が塗り三角形になる
        Pyx.trib(x1: 80, y1: 50, x2: 110, y2: 50, x3: 95, y3: 80, color: .cyan)
        Pyx.tri(x1: 120, y1: 50, x2: 150, y2: 50, x3: 135, y3: 80, color: .peach)

        // M2c: blt（画像バンク0の (0,0)-(16x16) を画面に転送、Color.black を透明扱い）
        Pyx.blt(x: 4, y: 90, image: 0, u: 0, v: 0, w: 16, h: 16, transparent: .black)
        // pal を効かせた blt: red → yellow に置換、透明判定はソース色で
        Pyx.pal(from: .red, to: .yellow)
        Pyx.blt(x: 24, y: 90, image: 0, u: 0, v: 0, w: 16, h: 16, transparent: .black)
        Pyx.pal()

        // M2c: bltm（タイルマップ0、3×2 タイル領域＝24×16 px、Color.black を透明）
        Pyx.bltm(x: 50, y: 90, tilemap: 0, u: 0, v: 0, w: 3, h: 2, transparent: .black)

        // M2c: text（4×6 ダミーグリフ）
        Pyx.text(x: 100, y: 95, "PYXIFT", color: .white)
        Pyx.text(x: 100, y: 105, "M2C OK", color: .yellow)
    }

    private func buildAssets() {
        // 画像バンク0: 16×16 のスプライトを描く（外周＝赤、中身＝白、左上隅＝黒で透明テスト）
        for x in 0..<16 {
            for y in 0..<16 {
                let isEdge = (x == 0 || y == 0 || x == 15 || y == 15)
                let color: Color = isEdge ? .red : .white
                Pyx.imagePset(bank: 0, x: x, y: y, color: color)
            }
        }
        // 透明色テスト用に左上 4×4 を黒（Color.black = 0）にしておく
        for x in 0..<4 {
            for y in 0..<4 {
                Pyx.imagePset(bank: 0, x: x, y: y, color: .black)
            }
        }
        // タイルマップ用に 8×8 タイルを 3 種類画像バンク0の (0,16) 以降に並べる
        // tile (0, 2) = 8×8 緑塗り、(1, 2) = 8×8 赤塗り、(2, 2) = 8×8 黒塗り（透明扱い）
        for ty in 0..<8 {
            for tx in 0..<8 {
                Pyx.imagePset(bank: 0, x: 0 * 8 + tx, y: 2 * 8 + ty, color: .green)
                Pyx.imagePset(bank: 0, x: 1 * 8 + tx, y: 2 * 8 + ty, color: .red)
                Pyx.imagePset(bank: 0, x: 2 * 8 + tx, y: 2 * 8 + ty, color: .black)
            }
        }
        // タイルマップ0: 3×2 セル領域に [緑 赤 黒 / 赤 黒 緑] のパターン
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

Pyx.run(Demo(), width: 160, height: 120, title: "Pyxift")
