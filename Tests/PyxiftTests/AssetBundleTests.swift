import Testing
import CPyxiftCore
import Foundation

private struct RoundTripResult {
    var ok: Int32
    var color: UInt8
    var tx: UInt8
    var ty: UInt8
    var imgsrc: Int32
    var note: Int8
    var speed: Int32
    var musicValue: Int32
}

private func roundTrip(imgX: Int32 = 7, imgY: Int32 = 11, imgColor: UInt8 = 5,
                       cx: Int32 = 3, cy: Int32 = 4,
                       tx: UInt8 = 12, ty: UInt8 = 9, imgsrc: Int32 = 2,
                       note: Int8 = 33, speed: Int32 = 24,
                       musicValue: Int32 = 17,
                       excludeImages: Bool = false,
                       excludeTilemaps: Bool = false,
                       excludeSounds: Bool = false,
                       excludeMusics: Bool = false) -> RoundTripResult {
    let path = NSTemporaryDirectory() + "pyxift-bundle-test-\(UUID().uuidString).pyxift"
    defer { try? FileManager.default.removeItem(atPath: path) }
    var color: UInt8 = 0
    var oTx: UInt8 = 0
    var oTy: UInt8 = 0
    var oImgsrc: Int32 = 0
    var oNote: Int8 = 0
    var oSpeed: Int32 = 0
    var oMusic: Int32 = 0
    let ok = path.withCString { p -> Int32 in
        pyxift_test_bundle_roundtrip(p,
                                     imgX, imgY, imgColor,
                                     cx, cy, tx, ty, imgsrc,
                                     note, speed, musicValue,
                                     excludeImages, excludeTilemaps,
                                     excludeSounds, excludeMusics,
                                     &color, &oTx, &oTy, &oImgsrc,
                                     &oNote, &oSpeed, &oMusic)
    }
    return RoundTripResult(ok: ok, color: color, tx: oTx, ty: oTy, imgsrc: oImgsrc,
                           note: oNote, speed: oSpeed, musicValue: oMusic)
}

@Test func assetBundleRoundTripPreservesAllBanks() {
    let r = roundTrip()
    #expect(r.ok == 1)
    #expect(r.color == 5)
    #expect(r.tx == 12)
    #expect(r.ty == 9)
    #expect(r.imgsrc == 2)
    #expect(r.note == 33)
    #expect(r.speed == 24)
    #expect(r.musicValue == 17)
}

@Test func assetBundleZeroInitializedRoundTripStaysZero() {
    let r = roundTrip(imgX: 0, imgY: 0, imgColor: 0,
                      cx: 0, cy: 0, tx: 0, ty: 0, imgsrc: 0,
                      note: -1, speed: 30, musicValue: 0)
    #expect(r.ok == 1)
    #expect(r.color == 0)
    #expect(r.tx == 0)
    #expect(r.ty == 0)
    #expect(r.imgsrc == 0)
    #expect(r.speed == 30)
}

// Sentinel values inside pyxift_test_bundle_roundtrip:
// kImageSentinel=14, kTileSentinelX=200, kTileSentinelY=201,
// kImgsrcSentinel=-42, kNoteSentinel=-100, kSpeedSentinel=-7, kMusicSentinel=-55.

@Test func assetBundleExcludeImagesPreservesDestinationImage() {
    let r = roundTrip(excludeImages: true)
    #expect(r.ok == 1)
    #expect(r.color == 14)
    #expect(r.tx == 12)
    #expect(r.note == 33)
    #expect(r.musicValue == 17)
}

@Test func assetBundleExcludeTilemapsPreservesDestinationTilemap() {
    let r = roundTrip(excludeTilemaps: true)
    #expect(r.ok == 1)
    #expect(r.color == 5)
    #expect(r.tx == 200)
    #expect(r.ty == 201)
    #expect(r.imgsrc == -42)
    #expect(r.note == 33)
}

@Test func assetBundleExcludeSoundsPreservesDestinationSound() {
    let r = roundTrip(excludeSounds: true)
    #expect(r.ok == 1)
    #expect(r.color == 5)
    #expect(r.note == -100)
    #expect(r.speed == -7)
    #expect(r.musicValue == 17)
}

@Test func assetBundleExcludeMusicsPreservesDestinationMusic() {
    let r = roundTrip(excludeMusics: true)
    #expect(r.ok == 1)
    #expect(r.color == 5)
    #expect(r.note == 33)
    #expect(r.musicValue == -55)
}

private func saveExcludeRoundTrip(imgX: Int32 = 7, imgY: Int32 = 11, imgColor: UInt8 = 5,
                                  cx: Int32 = 3, cy: Int32 = 4,
                                  tx: UInt8 = 12, ty: UInt8 = 9, imgsrc: Int32 = 2,
                                  note: Int8 = 33, speed: Int32 = 24,
                                  musicValue: Int32 = 17,
                                  excludeImages: Bool = false,
                                  excludeTilemaps: Bool = false,
                                  excludeSounds: Bool = false,
                                  excludeMusics: Bool = false) -> RoundTripResult {
    let path = NSTemporaryDirectory() + "pyxift-bundle-test-\(UUID().uuidString).pyxift"
    defer { try? FileManager.default.removeItem(atPath: path) }
    var color: UInt8 = 0
    var oTx: UInt8 = 0
    var oTy: UInt8 = 0
    var oImgsrc: Int32 = 0
    var oNote: Int8 = 0
    var oSpeed: Int32 = 0
    var oMusic: Int32 = 0
    let ok = path.withCString { p -> Int32 in
        pyxift_test_bundle_save_exclude_roundtrip(p,
                                                  imgX, imgY, imgColor,
                                                  cx, cy, tx, ty, imgsrc,
                                                  note, speed, musicValue,
                                                  excludeImages, excludeTilemaps,
                                                  excludeSounds, excludeMusics,
                                                  &color, &oTx, &oTy, &oImgsrc,
                                                  &oNote, &oSpeed, &oMusic)
    }
    return RoundTripResult(ok: ok, color: color, tx: oTx, ty: oTy, imgsrc: oImgsrc,
                           note: oNote, speed: oSpeed, musicValue: oMusic)
}

@Test func saveExcludeImagesPreservesDestinationImage() {
    let r = saveExcludeRoundTrip(excludeImages: true)
    #expect(r.ok == 1)
    #expect(r.color == 14)
    #expect(r.tx == 12)
    #expect(r.note == 33)
    #expect(r.musicValue == 17)
}

@Test func saveExcludeTilemapsPreservesDestinationTilemap() {
    let r = saveExcludeRoundTrip(excludeTilemaps: true)
    #expect(r.ok == 1)
    #expect(r.color == 5)
    #expect(r.tx == 200)
    #expect(r.ty == 201)
    #expect(r.imgsrc == -42)
    #expect(r.note == 33)
}

@Test func saveExcludeSoundsPreservesDestinationSound() {
    let r = saveExcludeRoundTrip(excludeSounds: true)
    #expect(r.ok == 1)
    #expect(r.color == 5)
    #expect(r.note == -100)
    #expect(r.speed == -7)
    #expect(r.musicValue == 17)
}

@Test func saveExcludeMusicsPreservesDestinationMusic() {
    let r = saveExcludeRoundTrip(excludeMusics: true)
    #expect(r.ok == 1)
    #expect(r.color == 5)
    #expect(r.note == 33)
    #expect(r.musicValue == -55)
}
