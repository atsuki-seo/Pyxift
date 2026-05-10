import Testing
import Foundation
import CPyxiftCore

private final class Canvas {
    let h: OpaquePointer
    init(width: Int = 16, height: Int = 16) {
        h = pyxift_canvas_test_create(Int32(width), Int32(height))!
    }
    deinit { pyxift_canvas_test_destroy(h) }
    var raw: OpaquePointer { h }

    func pget(_ x: Int, _ y: Int) -> UInt8 {
        pyxift_canvas_test_pget(h, Int32(x), Int32(y))
    }
}

private final class Image {
    let h: OpaquePointer
    init() { h = pyxift_image_test_create()! }
    deinit { pyxift_image_test_destroy(h) }
    var raw: OpaquePointer { h }

    func pset(_ x: Int, _ y: Int, _ c: UInt8) {
        pyxift_image_test_pset(h, Int32(x), Int32(y), c)
    }
}

private final class Tilemap {
    let h: OpaquePointer
    init() { h = pyxift_tilemap_test_create()! }
    deinit { pyxift_tilemap_test_destroy(h) }
    var raw: OpaquePointer { h }

    func setCell(_ cx: Int, _ cy: Int, _ tx: UInt8, _ ty: UInt8) {
        pyxift_tilemap_test_set_cell(h, Int32(cx), Int32(cy), tx, ty)
    }
}

@Test func elliFillsBoundingBoxCorners() {
    let c = Canvas(width: 16, height: 16)
    pyxift_canvas_test_elli(c.raw, 2, 3, 7, 5, 9)
    #expect(c.pget(2 + 7 / 2, 3) == 9)
    #expect(c.pget(2, 3 + 5 / 2) == 9)
    #expect(c.pget(2, 3) == 0)
    #expect(c.pget(2 + 6, 3) == 0)
    #expect(c.pget(2, 3 + 4) == 0)
    #expect(c.pget(2 + 6, 3 + 4) == 0)
    #expect(c.pget(2 + 3, 3 + 2) == 9)
}

@Test func elliWithZeroSizeIsNoop() {
    let c = Canvas()
    pyxift_canvas_test_elli(c.raw, 5, 5, 0, 5, 7)
    pyxift_canvas_test_elli(c.raw, 5, 5, 5, 0, 7)
    for y in 0..<16 {
        for x in 0..<16 {
            #expect(c.pget(x, y) == 0)
        }
    }
}

@Test func ellibDrawsBorderOnly() {
    let c = Canvas(width: 16, height: 16)
    pyxift_canvas_test_ellib(c.raw, 2, 3, 7, 5, 11)
    #expect(c.pget(2 + 7 / 2, 3) == 11)
    #expect(c.pget(2, 3 + 5 / 2) == 11)
    #expect(c.pget(2 + 3, 3 + 2) == 0)
}

@Test func fillReplacesContiguousRegion() {
    let c = Canvas(width: 8, height: 8)
    pyxift_canvas_test_rect(c.raw, 2, 2, 4, 4, 5)
    pyxift_canvas_test_fill(c.raw, 3, 3, 9)
    for y in 2..<6 {
        for x in 2..<6 {
            #expect(c.pget(x, y) == 9)
        }
    }
    #expect(c.pget(0, 0) == 0)
    #expect(c.pget(7, 7) == 0)
}

@Test func fillStopsAtClipBoundary() {
    let c = Canvas(width: 8, height: 8)
    pyxift_canvas_test_clip(c.raw, 1, 1, 4, 4)
    pyxift_canvas_test_fill(c.raw, 2, 2, 7)
    for y in 1..<5 {
        for x in 1..<5 {
            #expect(c.pget(x, y) == 7)
        }
    }
    #expect(c.pget(0, 0) == 0)
    #expect(c.pget(5, 5) == 0)
    #expect(c.pget(7, 7) == 0)
}

@Test func fillSameColorIsNoop() {
    let c = Canvas(width: 4, height: 4)
    pyxift_canvas_test_pset(c.raw, 1, 1, 5)
    pyxift_canvas_test_fill(c.raw, 1, 1, 5)
    #expect(c.pget(1, 1) == 5)
    #expect(c.pget(0, 0) == 0)
}

@Test func fillUnderDitherTerminates() {
    let c = Canvas(width: 8, height: 8)
    pyxift_canvas_test_dither(c.raw, 0.5)
    pyxift_canvas_test_fill(c.raw, 0, 0, 9)
    var hits = 0
    for y in 0..<8 {
        for x in 0..<8 {
            if c.pget(x, y) == 9 { hits += 1 }
        }
    }
    #expect(hits > 0)
    #expect(hits < 64)
}

@Test func ditherFullyOpaqueWritesEverywhere() {
    let c = Canvas(width: 8, height: 8)
    pyxift_canvas_test_dither(c.raw, 1.0)
    pyxift_canvas_test_rect(c.raw, 0, 0, 8, 8, 6)
    for y in 0..<8 {
        for x in 0..<8 {
            #expect(c.pget(x, y) == 6)
        }
    }
}

@Test func ditherFullyTransparentWritesNothing() {
    let c = Canvas(width: 8, height: 8)
    pyxift_canvas_test_dither(c.raw, 0.0)
    pyxift_canvas_test_rect(c.raw, 0, 0, 8, 8, 6)
    for y in 0..<8 {
        for x in 0..<8 {
            #expect(c.pget(x, y) == 0)
        }
    }
}

@Test func ditherHalfMatchesBayerPattern() {
    let c = Canvas(width: 4, height: 4)
    pyxift_canvas_test_dither(c.raw, 0.5)
    pyxift_canvas_test_rect(c.raw, 0, 0, 4, 4, 8)
    let expected: [[UInt8]] = [
        [8, 0, 8, 0],
        [0, 8, 0, 8],
        [8, 0, 8, 0],
        [0, 8, 0, 8],
    ]
    for y in 0..<4 {
        for x in 0..<4 {
            #expect(c.pget(x, y) == expected[y][x])
        }
    }
}

@Test func ditherIsClampedToUnit() {
    let c = Canvas(width: 4, height: 4)
    pyxift_canvas_test_dither(c.raw, 2.0)
    pyxift_canvas_test_rect(c.raw, 0, 0, 4, 4, 3)
    for y in 0..<4 {
        for x in 0..<4 {
            #expect(c.pget(x, y) == 3)
        }
    }
    pyxift_canvas_test_dither(c.raw, -1.0)
    pyxift_canvas_test_rect(c.raw, 0, 0, 4, 4, 5)
    for y in 0..<4 {
        for x in 0..<4 {
            #expect(c.pget(x, y) == 3)
        }
    }
}

@Test func bltUnrotatedUnscaledMatchesOldPath() {
    let c = Canvas(width: 16, height: 16)
    let img = Image()
    img.pset(0, 0, 4)
    img.pset(1, 0, 5)
    img.pset(0, 1, 6)
    img.pset(1, 1, 7)
    pyxift_canvas_test_blt(c.raw, 5, 5, img.raw, 0, 0, 2, 2, -1, 0, 1)
    #expect(c.pget(5, 5) == 4)
    #expect(c.pget(6, 5) == 5)
    #expect(c.pget(5, 6) == 6)
    #expect(c.pget(6, 6) == 7)
}

@Test func bltScale2EnlargesPixels() {
    let c = Canvas(width: 16, height: 16)
    let img = Image()
    img.pset(0, 0, 4)
    img.pset(1, 0, 4)
    img.pset(0, 1, 4)
    img.pset(1, 1, 4)
    pyxift_canvas_test_blt(c.raw, 2, 2, img.raw, 0, 0, 2, 2, -1, 0, 2)
    var hits = 0
    for y in 0..<10 {
        for x in 0..<10 {
            if c.pget(x, y) == 4 { hits += 1 }
        }
    }
    #expect(hits >= 9)
}

@Test func bltScaleZeroIsNoop() {
    let c = Canvas(width: 8, height: 8)
    let img = Image()
    img.pset(0, 0, 5)
    pyxift_canvas_test_blt(c.raw, 0, 0, img.raw, 0, 0, 1, 1, -1, 0, 0)
    for y in 0..<8 {
        for x in 0..<8 {
            #expect(c.pget(x, y) == 0)
        }
    }
}

@Test func bltRotate180SwapsSourcePixels() {
    let c = Canvas(width: 16, height: 16)
    let img = Image()
    img.pset(0, 0, 1)
    img.pset(1, 0, 2)
    img.pset(0, 1, 3)
    img.pset(1, 1, 4)
    pyxift_canvas_test_blt(c.raw, 5, 5, img.raw, 0, 0, 2, 2, -1, 180, 1)
    #expect(c.pget(5, 5) == 4)
    #expect(c.pget(6, 5) == 3)
    #expect(c.pget(5, 6) == 2)
    #expect(c.pget(6, 6) == 1)
}

@Test func bltTransparentColorIsSkipped() {
    let c = Canvas(width: 8, height: 8)
    pyxift_canvas_test_cls(c.raw, 7)
    let img = Image()
    img.pset(0, 0, 0)
    img.pset(1, 0, 5)
    pyxift_canvas_test_blt(c.raw, 2, 2, img.raw, 0, 0, 2, 1, 0, 0, 1)
    #expect(c.pget(2, 2) == 7)
    #expect(c.pget(3, 2) == 5)
}

@Test func bltmStitchesTilesUnrotated() {
    let c = Canvas(width: 32, height: 32)
    let img = Image()
    img.pset(0, 0, 5)
    img.pset(8, 0, 9)
    let tm = Tilemap()
    tm.setCell(0, 0, 0, 0)
    tm.setCell(1, 0, 1, 0)
    pyxift_canvas_test_bltm(c.raw, 0, 0, tm.raw, img.raw, 0, 0, 2, 1, -1, 0, 1)
    #expect(c.pget(0, 0) == 5)
    #expect(c.pget(8, 0) == 9)
}

@Test func cameraOffsetsDrawing() {
    let c = Canvas(width: 16, height: 16)
    pyxift_canvas_test_camera(c.raw, 5, 0)
    pyxift_canvas_test_pset(c.raw, 7, 3, 11)
    #expect(c.pget(7, 3) == 11)
    pyxift_canvas_test_camera_reset(c.raw)
    #expect(c.pget(2, 3) == 11)
}

@Test func resizeChangesDimensionsAndClearsPixels() {
    let c = Canvas(width: 16, height: 16)
    pyxift_canvas_test_pset(c.raw, 5, 5, 9)
    pyxift_canvas_test_resize(c.raw, 32, 24)
    #expect(pyxift_canvas_test_width(c.raw) == 32)
    #expect(pyxift_canvas_test_height(c.raw) == 24)
    #expect(c.pget(5, 5) == 0)
    #expect(c.pget(31, 23) == 0)
}

@Test func resizeRejectsNonPositiveDimensions() {
    let c = Canvas(width: 16, height: 16)
    pyxift_canvas_test_resize(c.raw, 0, 10)
    #expect(pyxift_canvas_test_width(c.raw) == 16)
    #expect(pyxift_canvas_test_height(c.raw) == 16)
    pyxift_canvas_test_resize(c.raw, -3, -3)
    #expect(pyxift_canvas_test_width(c.raw) == 16)
}

@Test func resizeResetsClipAndCamera() {
    let c = Canvas(width: 16, height: 16)
    pyxift_canvas_test_clip(c.raw, 2, 2, 4, 4)
    pyxift_canvas_test_camera(c.raw, 3, 3)
    pyxift_canvas_test_resize(c.raw, 24, 24)
    pyxift_canvas_test_pset(c.raw, 0, 0, 7)
    pyxift_canvas_test_pset(c.raw, 23, 23, 8)
    #expect(c.pget(0, 0) == 7)
    #expect(c.pget(23, 23) == 8)
}

@Test func savePngWritesValidFile() {
    let c = Canvas(width: 4, height: 3)
    pyxift_canvas_test_cls(c.raw, 5)
    pyxift_canvas_test_pset(c.raw, 0, 0, 8)
    let dir = FileManager.default.temporaryDirectory
    let path = dir.appendingPathComponent("pyxift-test-\(UUID().uuidString).png").path
    defer { try? FileManager.default.removeItem(atPath: path) }
    let ok = path.withCString { pyxift_canvas_test_save_png(c.raw, $0, 2) }
    #expect(ok == 1)
    let data = try? Data(contentsOf: URL(fileURLWithPath: path))
    #expect(data != nil)
    if let d = data {
        #expect(d.count > 8)
        let sig: [UInt8] = [0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A]
        #expect(Array(d.prefix(8)) == sig)
    }
}

@Test func savePngRejectsNullPath() {
    let c = Canvas(width: 4, height: 4)
    let ok = pyxift_canvas_test_save_png(c.raw, nil, 1)
    #expect(ok == 0)
}
