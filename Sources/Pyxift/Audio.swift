import CPyxiftCore

extension Pyx {
    @MainActor
    public static func sound(_ index: Int,
                             notes: String,
                             tones: String = "t",
                             volumes: String = "7",
                             effects: String = "n",
                             speed: Int = 30) {
        guard let engine = Runtime.engine else { return }
        notes.withCString { n in
            tones.withCString { t in
                volumes.withCString { v in
                    effects.withCString { e in
                        pyxift_engine_sound_set(engine,
                                                Int32(index),
                                                n, t, v, e,
                                                Int32(speed))
                    }
                }
            }
        }
    }

    @MainActor
    public static func play(channel: Int, sound: Int, loop: Bool = false) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_play(engine, Int32(channel), Int32(sound), loop)
    }

    @MainActor
    public static func stop(channel: Int) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_stop(engine, Int32(channel))
    }

    @MainActor
    public static func stop() {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_stop_all(engine)
    }

    @MainActor
    public static func isPlaying(channel: Int) -> Bool {
        guard let engine = Runtime.engine else { return false }
        return pyxift_engine_is_playing(engine, Int32(channel))
    }

    @MainActor
    public static func music(_ index: Int,
                             ch0: [Int] = [],
                             ch1: [Int] = [],
                             ch2: [Int] = [],
                             ch3: [Int] = []) {
        guard let engine = Runtime.engine else { return }
        let s0 = ch0.map { Int32($0) }
        let s1 = ch1.map { Int32($0) }
        let s2 = ch2.map { Int32($0) }
        let s3 = ch3.map { Int32($0) }
        s0.withUnsafeBufferPointer { p0 in
            s1.withUnsafeBufferPointer { p1 in
                s2.withUnsafeBufferPointer { p2 in
                    s3.withUnsafeBufferPointer { p3 in
                        pyxift_engine_music_set(engine,
                                                Int32(index),
                                                p0.baseAddress, Int32(p0.count),
                                                p1.baseAddress, Int32(p1.count),
                                                p2.baseAddress, Int32(p2.count),
                                                p3.baseAddress, Int32(p3.count))
                    }
                }
            }
        }
    }

    @MainActor
    public static func playMusic(_ msc: Int, loop: Bool = false) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_play_music(engine, Int32(msc), loop)
    }

    @MainActor
    public static func playPos(channel: Int) -> (sound: Int, sec: Double)? {
        guard let engine = Runtime.engine else { return nil }
        var idx: Int32 = 0
        var sec: Float = 0
        let ok = pyxift_engine_play_pos(engine, Int32(channel), &idx, &sec)
        if !ok { return nil }
        return (Int(idx), Double(sec))
    }
}
