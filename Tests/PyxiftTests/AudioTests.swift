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

@Test func playPosNilWhenIdle() {
    let m = Mixer()
    var idx: Int32 = -1
    var sec: Float = -1
    #expect(!pyxift_audio_mixer_play_pos(m.h, 0, &idx, &sec))
}

@Test func playPosReturnsCurrentSoundIndex() {
    let m = Mixer()
    m.setSound(7, notes: "a2", tones: "t", volumes: "7", effects: "n", speed: 30)
    pyxift_audio_mixer_play(m.h, 0, 7, true)
    var idx: Int32 = -1
    var sec: Float = -1
    #expect(pyxift_audio_mixer_play_pos(m.h, 0, &idx, &sec))
    #expect(idx == 7)
    #expect(sec >= 0)
}

@Test func playPosNilAfterStop() {
    let m = Mixer()
    m.setSound(0, notes: "a2", tones: "t", volumes: "7", effects: "n", speed: 30)
    pyxift_audio_mixer_play(m.h, 0, 0, false)
    pyxift_audio_mixer_stop(m.h, 0)
    var idx: Int32 = 0
    var sec: Float = 0
    #expect(!pyxift_audio_mixer_play_pos(m.h, 0, &idx, &sec))
}

@Test func musicPlaysOnAllAssignedChannels() {
    let m = Mixer()
    m.setSound(0, notes: "a2", tones: "t", volumes: "7", effects: "n", speed: 30)
    m.setSound(1, notes: "c2", tones: "t", volumes: "7", effects: "n", speed: 30)

    let ch0: [Int32] = [0]
    let ch1: [Int32] = [1]
    let ch2: [Int32] = []
    let ch3: [Int32] = []
    ch0.withUnsafeBufferPointer { p0 in
        ch1.withUnsafeBufferPointer { p1 in
            ch2.withUnsafeBufferPointer { p2 in
                ch3.withUnsafeBufferPointer { p3 in
                    pyxift_audio_mixer_music_set(m.h, 0,
                        p0.baseAddress, Int32(p0.count),
                        p1.baseAddress, Int32(p1.count),
                        p2.baseAddress, Int32(p2.count),
                        p3.baseAddress, Int32(p3.count))
                }
            }
        }
    }
    pyxift_audio_mixer_play_music(m.h, 0, false)
    #expect(pyxift_audio_mixer_is_playing(m.h, 0))
    #expect(pyxift_audio_mixer_is_playing(m.h, 1))
    #expect(!pyxift_audio_mixer_is_playing(m.h, 2))
    #expect(!pyxift_audio_mixer_is_playing(m.h, 3))

    var buf = [Int16](repeating: 0, count: 1024)
    pyxift_audio_mixer_render(m.h, &buf, Int32(buf.count))
    let energy = buf.reduce(0) { $0 + Int($1) * Int($1) }
    #expect(energy > 0)
}
