import Testing
import CPyxiftCore
import Foundation

@Test func assetBundleRoundTripPreservesAllBanks() {
    let tmp = NSTemporaryDirectory() + "pyxift-bundle-test-\(UUID().uuidString).pyxift"
    defer { try? FileManager.default.removeItem(atPath: tmp) }

    var outColor: UInt8 = 0
    var outTx: UInt8 = 0
    var outTy: UInt8 = 0
    var outImgsrc: Int32 = -1
    var outNote: Int8 = 0
    var outSpeed: Int32 = 0
    var outMusicValue: Int32 = -1

    let ok = tmp.withCString { path -> Int32 in
        pyxift_test_bundle_roundtrip(path,
                                     7, 11, 5,
                                     3, 4, 12, 9, 2,
                                     33, 24,
                                     17,
                                     &outColor,
                                     &outTx, &outTy,
                                     &outImgsrc,
                                     &outNote, &outSpeed,
                                     &outMusicValue)
    }
    #expect(ok == 1)
    #expect(outColor == 5)
    #expect(outTx == 12)
    #expect(outTy == 9)
    #expect(outImgsrc == 2)
    #expect(outNote == 33)
    #expect(outSpeed == 24)
    #expect(outMusicValue == 17)
}

@Test func assetBundleZeroInitializedRoundTripStaysZero() {
    let tmp = NSTemporaryDirectory() + "pyxift-bundle-test-\(UUID().uuidString).pyxift"
    defer { try? FileManager.default.removeItem(atPath: tmp) }

    var outColor: UInt8 = 99
    var outTx: UInt8 = 99
    var outTy: UInt8 = 99
    var outImgsrc: Int32 = -1
    var outNote: Int8 = 0
    var outSpeed: Int32 = 0
    var outMusicValue: Int32 = -1

    // Sample at (0,0) with color 0 means the bundle is effectively empty —
    // every encoder should produce a minimal JSON, and decoding must yield
    // the same zero-initialized state.
    let ok = tmp.withCString { path -> Int32 in
        pyxift_test_bundle_roundtrip(path,
                                     0, 0, 0,
                                     0, 0, 0, 0, 0,
                                     -1, 30,
                                     0,
                                     &outColor,
                                     &outTx, &outTy,
                                     &outImgsrc,
                                     &outNote, &outSpeed,
                                     &outMusicValue)
    }
    #expect(ok == 1)
    #expect(outColor == 0)
    #expect(outTx == 0)
    #expect(outTy == 0)
    #expect(outImgsrc == 0)
    #expect(outSpeed == 30)
}
