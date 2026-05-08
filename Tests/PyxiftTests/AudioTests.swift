import Testing
import CPyxiftCore

private func parseNotes(_ s: String) -> [Int8] {
    let n = s.withCString { pyxift_audio_sound_parse_notes($0, nil, 0) }
    var buf = [Int8](repeating: 0, count: Int(n))
    _ = s.withCString { pyxift_audio_sound_parse_notes($0, &buf, Int32(buf.count)) }
    return buf
}

private func parseTones(_ s: String) -> [UInt8] {
    let n = s.withCString { pyxift_audio_sound_parse_tones($0, nil, 0) }
    var buf = [UInt8](repeating: 0, count: Int(n))
    _ = s.withCString { pyxift_audio_sound_parse_tones($0, &buf, Int32(buf.count)) }
    return buf
}

private func parseVolumes(_ s: String) -> [UInt8] {
    let n = s.withCString { pyxift_audio_sound_parse_volumes($0, nil, 0) }
    var buf = [UInt8](repeating: 0, count: Int(n))
    _ = s.withCString { pyxift_audio_sound_parse_volumes($0, &buf, Int32(buf.count)) }
    return buf
}

private func parseEffects(_ s: String) -> [UInt8] {
    let n = s.withCString { pyxift_audio_sound_parse_effects($0, nil, 0) }
    var buf = [UInt8](repeating: 0, count: Int(n))
    _ = s.withCString { pyxift_audio_sound_parse_effects($0, &buf, Int32(buf.count)) }
    return buf
}

@Test func notesParserParsesOctaveAndAccidentals() {
    // C0=0, D0=2, E0=4, ... ; A2 = 2*12+9 = 33
    #expect(parseNotes("c0d0e0") == [0, 2, 4])
    #expect(parseNotes("a2") == [33])
    #expect(parseNotes("c#1") == [13])
    #expect(parseNotes("d-1") == [13])
    #expect(parseNotes("R") == [-1])
    #expect(parseNotes("") == [])
}

@Test func notesParserIgnoresWhitespaceAndIsCaseInsensitive() {
    #expect(parseNotes("  C0 D0\tE0 ") == [0, 2, 4])
    #expect(parseNotes("g2B-2D3R") == [31, 34, 38, -1])
}

@Test func tonesParserMapsLetters() {
    #expect(parseTones("tspn") == [0, 1, 2, 3])
    #expect(parseTones("TTSS PPPN") == [0, 0, 1, 1, 2, 2, 2, 3])
}

@Test func volumesParser() {
    #expect(parseVolumes("01234567") == [0, 1, 2, 3, 4, 5, 6, 7])
    #expect(parseVolumes("7777 7531") == [7, 7, 7, 7, 7, 5, 3, 1])
}

@Test func effectsParser() {
    #expect(parseEffects("nsvfhq") == [0, 1, 2, 3, 4, 5])
}

private final class Mixer {
    let h: OpaquePointer
    init() { h = pyxift_audio_mixer_create()! }
    deinit { pyxift_audio_mixer_destroy(h) }

    func setSound(_ index: Int32, notes: String, tones: String, volumes: String, effects: String, speed: Int32) {
        notes.withCString { n in
            tones.withCString { t in
                volumes.withCString { v in
                    effects.withCString { e in
                        pyxift_audio_mixer_set_sound(h, index, n, t, v, e, speed)
                    }
                }
            }
        }
    }
}

@Test func mixerProducesNonZeroPCMWhenPlaying() {
    let m = Mixer()
    m.setSound(0, notes: "a2", tones: "t", volumes: "7", effects: "n", speed: 30)
    pyxift_audio_mixer_play(m.h, 0, 0, false)
    #expect(pyxift_audio_mixer_is_playing(m.h, 0))

    var buf = [Int16](repeating: 0, count: 1024)
    pyxift_audio_mixer_render(m.h, &buf, Int32(buf.count))

    let energy = buf.reduce(0) { $0 + Int($1) * Int($1) }
    #expect(energy > 0)
}

@Test func mixerSilentWhenIdle() {
    let m = Mixer()
    var buf = [Int16](repeating: 0, count: 256)
    pyxift_audio_mixer_render(m.h, &buf, Int32(buf.count))
    #expect(buf.allSatisfy { $0 == 0 })
}

@Test func mixerStopSilencesChannel() {
    let m = Mixer()
    m.setSound(0, notes: "a2", tones: "t", volumes: "7", effects: "n", speed: 30)
    pyxift_audio_mixer_play(m.h, 0, 0, true)
    pyxift_audio_mixer_stop(m.h, 0)
    #expect(!pyxift_audio_mixer_is_playing(m.h, 0))
}
