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
}
