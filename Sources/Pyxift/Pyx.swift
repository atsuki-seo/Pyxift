import CPyxiftCore

public enum Pyx {}

@MainActor
enum Runtime {
    static var engine: OpaquePointer?
    static var appBox: AnyObject?
    static var updateThunk: (() -> Void)?
    static var drawThunk: (() -> Void)?
}

@MainActor
private final class AppBox<A: App> {
    var app: A
    init(_ app: A) { self.app = app }
}

extension Pyx {
    @MainActor
    public static func run<A: App>(_ app: A,
                                   width: Int = 160,
                                   height: Int = 120,
                                   title: String = "Pyxift",
                                   fps: Int = 30) {
        guard Runtime.engine == nil else {
            fatalError("Pyx.run is already running")
        }

        let engine = title.withCString { c in
            pyxift_engine_create(Int32(width), Int32(height), c, Int32(fps))
        }
        guard let engine else {
            fatalError("pyxift_engine_create failed")
        }
        Runtime.engine = engine

        let box = AppBox(app)
        Runtime.appBox = box
        Runtime.updateThunk = { box.app.update() }
        Runtime.drawThunk = { box.app.draw() }

        let updateFn: @convention(c) (UnsafeMutableRawPointer?) -> Void = { _ in
            MainActor.assumeIsolated {
                Runtime.updateThunk?()
            }
        }
        let drawFn: @convention(c) (UnsafeMutableRawPointer?) -> Void = { _ in
            MainActor.assumeIsolated {
                Runtime.drawThunk?()
            }
        }

        pyxift_engine_run(engine, updateFn, drawFn, nil)

        pyxift_engine_destroy(engine)
        Runtime.engine = nil
        Runtime.appBox = nil
        Runtime.updateThunk = nil
        Runtime.drawThunk = nil
    }

    @MainActor
    public static func quit() {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_quit(engine)
    }

    @MainActor
    public static func title(_ s: String) {
        guard let engine = Runtime.engine else { return }
        s.withCString { c in
            pyxift_engine_set_title(engine, c)
        }
    }

    @MainActor
    public static var width: Int {
        guard let engine = Runtime.engine else { return 0 }
        return Int(pyxift_engine_width(engine))
    }

    @MainActor
    public static var height: Int {
        guard let engine = Runtime.engine else { return 0 }
        return Int(pyxift_engine_height(engine))
    }

    @MainActor
    public static var frameCount: Int {
        guard let engine = Runtime.engine else { return 0 }
        return Int(pyxift_engine_frame_count(engine))
    }

    @MainActor
    public static func cls(color: Color) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_cls(engine, color.index)
    }

    @MainActor
    public static func pset(x: Int, y: Int, color: Color) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_pset(engine, Int32(x), Int32(y), color.index)
    }

    @MainActor
    public static func pget(x: Int, y: Int) -> Color {
        guard let engine = Runtime.engine else { return .black }
        return Color(Int(pyxift_engine_pget(engine, Int32(x), Int32(y))))
    }

    @MainActor
    public static func line(x1: Int, y1: Int, x2: Int, y2: Int, color: Color) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_line(engine, Int32(x1), Int32(y1), Int32(x2), Int32(y2), color.index)
    }

    @MainActor
    public static func rect(x: Int, y: Int, w: Int, h: Int, color: Color) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_rect(engine, Int32(x), Int32(y), Int32(w), Int32(h), color.index)
    }

    @MainActor
    public static func rectb(x: Int, y: Int, w: Int, h: Int, color: Color) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_rectb(engine, Int32(x), Int32(y), Int32(w), Int32(h), color.index)
    }

    @MainActor
    public static func circ(x: Int, y: Int, r: Int, color: Color) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_circ(engine, Int32(x), Int32(y), Int32(r), color.index)
    }

    @MainActor
    public static func circb(x: Int, y: Int, r: Int, color: Color) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_circb(engine, Int32(x), Int32(y), Int32(r), color.index)
    }

    @MainActor
    public static func tri(x1: Int, y1: Int, x2: Int, y2: Int, x3: Int, y3: Int, color: Color) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_tri(engine,
                          Int32(x1), Int32(y1),
                          Int32(x2), Int32(y2),
                          Int32(x3), Int32(y3),
                          color.index)
    }

    @MainActor
    public static func trib(x1: Int, y1: Int, x2: Int, y2: Int, x3: Int, y3: Int, color: Color) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_trib(engine,
                           Int32(x1), Int32(y1),
                           Int32(x2), Int32(y2),
                           Int32(x3), Int32(y3),
                           color.index)
    }

    @MainActor
    public static func clip(x: Int, y: Int, w: Int, h: Int) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_clip(engine, Int32(x), Int32(y), Int32(w), Int32(h))
    }

    @MainActor
    public static func clip() {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_clip_reset(engine)
    }

    @MainActor
    public static func camera(x: Int, y: Int) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_camera(engine, Int32(x), Int32(y))
    }

    @MainActor
    public static func camera() {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_camera_reset(engine)
    }

    @MainActor
    public static func pal(from: Color, to: Color) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_pal(engine, from.index, to.index)
    }

    @MainActor
    public static func pal() {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_pal_reset(engine)
    }

    @MainActor
    public static func blt(x: Int, y: Int,
                           image: Int,
                           u: Int, v: Int, w: Int, h: Int,
                           transparent: Color? = nil) {
        guard let engine = Runtime.engine else { return }
        let t = transparent.map { Int32($0.index) } ?? Int32(-1)
        pyxift_engine_blt(engine,
                          Int32(x), Int32(y),
                          Int32(image),
                          Int32(u), Int32(v), Int32(w), Int32(h),
                          t)
    }

    @MainActor
    public static func bltm(x: Int, y: Int,
                            tilemap: Int,
                            u: Int, v: Int, w: Int, h: Int,
                            transparent: Color? = nil) {
        guard let engine = Runtime.engine else { return }
        let t = transparent.map { Int32($0.index) } ?? Int32(-1)
        pyxift_engine_bltm(engine,
                           Int32(x), Int32(y),
                           Int32(tilemap),
                           Int32(u), Int32(v), Int32(w), Int32(h),
                           t)
    }

    @MainActor
    public static func text(x: Int, y: Int, _ s: String, color: Color) {
        guard let engine = Runtime.engine else { return }
        s.withCString { c in
            pyxift_engine_text(engine, Int32(x), Int32(y), c, color.index)
        }
    }
}

extension Pyx {
    @MainActor
    public static func button(_ b: Button, player: Int = 0) -> Bool {
        guard let engine = Runtime.engine else { return false }
        return pyxift_engine_button(engine, b.rawValue, Int32(player))
    }

    @MainActor
    public static func buttonPressed(_ b: Button, player: Int = 0,
                                     hold: Int = 0, `repeat`: Int = 0) -> Bool {
        guard let engine = Runtime.engine else { return false }
        return pyxift_engine_button_pressed(engine, b.rawValue, Int32(player),
                                            Int32(hold), Int32(`repeat`))
    }

    @MainActor
    public static func buttonReleased(_ b: Button, player: Int = 0) -> Bool {
        guard let engine = Runtime.engine else { return false }
        return pyxift_engine_button_released(engine, b.rawValue, Int32(player))
    }

    @MainActor
    public static func gamepadAxis(_ axis: GamepadAxis, player: Int = 0) -> Float {
        guard let engine = Runtime.engine else { return 0 }
        return pyxift_engine_gamepad_axis(engine, Int32(player), axis.rawValue)
    }

    @MainActor
    public static func key(_ k: Key) -> Bool {
        guard let engine = Runtime.engine else { return false }
        return pyxift_engine_key(engine, k.rawValue)
    }

    @MainActor
    public static func keyPressed(_ k: Key, hold: Int = 0, `repeat`: Int = 0) -> Bool {
        guard let engine = Runtime.engine else { return false }
        return pyxift_engine_key_pressed(engine, k.rawValue,
                                         Int32(hold), Int32(`repeat`))
    }

    @MainActor
    public static func keyReleased(_ k: Key) -> Bool {
        guard let engine = Runtime.engine else { return false }
        return pyxift_engine_key_released(engine, k.rawValue)
    }

    @MainActor
    public static func mouse() -> (x: Int, y: Int) {
        guard let engine = Runtime.engine else { return (0, 0) }
        var mx: Int32 = 0
        var my: Int32 = 0
        pyxift_engine_mouse(engine, &mx, &my)
        return (Int(mx), Int(my))
    }

    @MainActor
    public static var mouseWheel: Int {
        guard let engine = Runtime.engine else { return 0 }
        return Int(pyxift_engine_mouse_wheel(engine))
    }

    @MainActor
    public static func mouseButton(_ b: MouseButton) -> Bool {
        guard let engine = Runtime.engine else { return false }
        return pyxift_engine_mouse_button(engine, b.rawValue)
    }

    @MainActor
    public static func mouseButtonPressed(_ b: MouseButton) -> Bool {
        guard let engine = Runtime.engine else { return false }
        return pyxift_engine_mouse_button_pressed(engine, b.rawValue)
    }

    @MainActor
    public static func mouseButtonReleased(_ b: MouseButton) -> Bool {
        guard let engine = Runtime.engine else { return false }
        return pyxift_engine_mouse_button_released(engine, b.rawValue)
    }

    @MainActor
    public static func mouseCursor(visible: Bool) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_mouse_cursor(engine, visible)
    }

    @MainActor
    public static func setMousePos(x: Int, y: Int) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_set_mouse_pos(engine, Int32(x), Int32(y))
    }

    @MainActor
    public static func inputText() -> String {
        guard let engine = Runtime.engine else { return "" }
        let len = Int(pyxift_engine_input_text(engine, nil, 0))
        if len == 0 { return "" }
        var buf = [UInt8](repeating: 0, count: len + 1)
        buf.withUnsafeMutableBufferPointer { p in
            _ = p.baseAddress?.withMemoryRebound(to: CChar.self, capacity: len + 1) { cp in
                pyxift_engine_input_text(engine, cp, Int32(len + 1))
            }
        }
        buf.removeLast()
        return String(decoding: buf, as: UTF8.self)
    }

    @MainActor
    public static func inputKeys() -> [Key] {
        guard let engine = Runtime.engine else { return [] }
        let count = Int(pyxift_engine_input_keys_count(engine))
        var keys: [Key] = []
        keys.reserveCapacity(count)
        for i in 0..<count {
            let raw = pyxift_engine_input_keys_at(engine, Int32(i))
            if let k = Key(rawValue: raw) {
                keys.append(k)
            }
        }
        return keys
    }

    @MainActor
    public static func droppedFiles() -> [String] {
        guard let engine = Runtime.engine else { return [] }
        let count = Int(pyxift_engine_dropped_files_count(engine))
        var paths: [String] = []
        paths.reserveCapacity(count)
        for i in 0..<count {
            let len = Int(pyxift_engine_dropped_files_at(engine, Int32(i), nil, 0))
            if len < 0 { continue }
            if len == 0 {
                paths.append("")
                continue
            }
            var buf = [UInt8](repeating: 0, count: len + 1)
            buf.withUnsafeMutableBufferPointer { p in
                _ = p.baseAddress?.withMemoryRebound(to: CChar.self, capacity: len + 1) { cp in
                    pyxift_engine_dropped_files_at(engine, Int32(i), cp, Int32(len + 1))
                }
            }
            buf.removeLast()
            paths.append(String(decoding: buf, as: UTF8.self))
        }
        return paths
    }
}

extension Pyx {
    @MainActor
    public static func load(_ path: String,
                            excludeImages: Bool = false,
                            excludeTilemaps: Bool = false,
                            excludeSounds: Bool = false,
                            excludeMusics: Bool = false) {
        guard let engine = Runtime.engine else {
            fatalError("Pyx.load called before Pyx.run")
        }
        let ok = path.withCString { c in
            pyxift_engine_load_bundle(engine, c,
                                      excludeImages, excludeTilemaps,
                                      excludeSounds, excludeMusics)
        }
        if !ok {
            fatalError("Pyx.load failed: \(path)")
        }
    }

    @MainActor
    public static func save(_ path: String,
                            excludeImages: Bool = false,
                            excludeTilemaps: Bool = false,
                            excludeSounds: Bool = false,
                            excludeMusics: Bool = false) {
        guard let engine = Runtime.engine else {
            fatalError("Pyx.save called before Pyx.run")
        }
        let ok = path.withCString { c in
            pyxift_engine_save_bundle(engine, c,
                                      excludeImages, excludeTilemaps,
                                      excludeSounds, excludeMusics)
        }
        if !ok {
            fatalError("Pyx.save failed: \(path)")
        }
    }
}

extension Pyx {
    @MainActor
    public static func loadImage(_ path: String, into bank: Int = 0) {
        guard let engine = Runtime.engine else {
            fatalError("Pyx.loadImage called before Pyx.run")
        }
        let ok = path.withCString { c in
            pyxift_engine_load_image(engine, Int32(bank), c)
        }
        if !ok {
            fatalError("Pyx.loadImage failed: \(path) (bank: \(bank))")
        }
    }
}

extension Pyx {
    @MainActor
    public static func imagePset(bank: Int, x: Int, y: Int, color: Color) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_image_pset(engine, Int32(bank), Int32(x), Int32(y), color.index)
    }

    @MainActor
    public static func tilemapSetCell(tilemap: Int, cx: Int, cy: Int,
                                      tileX: Int, tileY: Int) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_tilemap_set(engine, Int32(tilemap), Int32(cx), Int32(cy),
                                  UInt8(tileX & 0xff), UInt8(tileY & 0xff))
    }

    @MainActor
    public static func tilemapSetImageBank(tilemap: Int, bank: Int) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_tilemap_set_image_bank(engine, Int32(tilemap), Int32(bank))
    }
}
