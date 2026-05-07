import Testing
import CPyxiftCore

// pyxift::font_pixel と本家由来 FONT_DATA の意味論テスト。
// 完全一致は目指さないが、本家から流用したビット配列のデコード規則
// （上位ビット 0x0080_0000 から行優先で 24bit）と、範囲外文字の扱いを確認する。
//
// 期待値は本家 FONT_DATA のうち代表的なグリフを、Pyxel 同様
// 「上位 8bit 未使用、その後 4×6=24bit を行優先」で展開したもの。

private func glyphMatrix(_ ch: Character) -> [[Bool]] {
    let scalar = ch.unicodeScalars.first!
    let c = CChar(scalar.value)
    let h = Int(pyxift_font_glyph_height())
    let w = Int(pyxift_font_glyph_width())
    var rows: [[Bool]] = []
    for r in 0..<h {
        var row: [Bool] = []
        for c2 in 0..<w {
            row.append(pyxift_font_pixel(c, Int32(c2), Int32(r)))
        }
        rows.append(row)
    }
    return rows
}

@Test func glyphSizeIsFourBySix() {
    #expect(pyxift_font_glyph_width() == 4)
    #expect(pyxift_font_glyph_height() == 6)
}

@Test func spaceGlyphIsAllBlank() {
    // FONT_DATA[0] = 0x000000 → 全 0、6 行とも空白。
    let m = glyphMatrix(" ")
    for row in m {
        for px in row {
            #expect(!px)
        }
    }
}

@Test func capitalAGlyphMatchesPyxelData() {
    // 'A' = 0x41, FONT_DATA index = 0x41 - 0x20 = 0x21 = 33 → 0x4aeaa0
    // 24bit を行優先 4列ずつ: 0100 1010 1110 1010 1010 0000
    // すなわち:
    //   . X . .
    //   X . X .
    //   X X X .
    //   X . X .
    //   X . X .
    //   . . . .
    let m = glyphMatrix("A")
    let expected: [[Bool]] = [
        [false, true,  false, false],
        [true,  false, true,  false],
        [true,  true,  true,  false],
        [true,  false, true,  false],
        [true,  false, true,  false],
        [false, false, false, false],
    ]
    #expect(m == expected)
}

@Test func filledGlyphIsAllOn() {
    // '\x7f' (DEL) = FONT_DATA 末尾 0xeeeee0 → 5 行 X X X . が並び 6 行目空白。
    let m = glyphMatrix("\u{7f}")
    for r in 0..<5 {
        #expect(m[r] == [true, true, true, false])
    }
    #expect(m[5] == [false, false, false, false])
}

@Test func outOfRangeCharsAreInvisible() {
    // 制御文字（0x1f 以下）と 0x80 以上は非表示。
    #expect(!pyxift_font_pixel(CChar(0x00), 0, 0))
    #expect(!pyxift_font_pixel(CChar(0x1f), 0, 0))
    #expect(!pyxift_font_pixel(CChar(bitPattern: 0xff), 0, 0))
}

@Test func outOfRangePixelCoordsReturnFalse() {
    // 範囲外の col/row は描画しない（Canvas からの誤呼び出し対策）。
    #expect(!pyxift_font_pixel(Character("A").asciiValue.map(CChar.init)!, -1, 0))
    #expect(!pyxift_font_pixel(Character("A").asciiValue.map(CChar.init)!, 4, 0))
    #expect(!pyxift_font_pixel(Character("A").asciiValue.map(CChar.init)!, 0, 6))
}
