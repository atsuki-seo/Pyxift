import Testing
import CPyxiftCore

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
    let m = glyphMatrix(" ")
    for row in m {
        for px in row {
            #expect(!px)
        }
    }
}

@Test func capitalAGlyphMatchesPyxelData() {
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
    let m = glyphMatrix("\u{7f}")
    for r in 0..<5 {
        #expect(m[r] == [true, true, true, false])
    }
    #expect(m[5] == [false, false, false, false])
}

@Test func outOfRangeCharsAreInvisible() {
    #expect(!pyxift_font_pixel(CChar(0x00), 0, 0))
    #expect(!pyxift_font_pixel(CChar(0x1f), 0, 0))
    #expect(!pyxift_font_pixel(CChar(bitPattern: 0xff), 0, 0))
}

@Test func outOfRangePixelCoordsReturnFalse() {
    #expect(!pyxift_font_pixel(Character("A").asciiValue.map(CChar.init)!, -1, 0))
    #expect(!pyxift_font_pixel(Character("A").asciiValue.map(CChar.init)!, 4, 0))
    #expect(!pyxift_font_pixel(Character("A").asciiValue.map(CChar.init)!, 0, 6))
}
