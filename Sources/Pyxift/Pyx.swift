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
                           transparent: Color? = nil,
                           rotate: Double = 0,
                           scale: Double = 1) {
        guard let engine = Runtime.engine else { return }
        let t = transparent.map { Int32($0.index) } ?? Int32(-1)
        pyxift_engine_blt(engine,
                          Int32(x), Int32(y),
                          Int32(image),
                          Int32(u), Int32(v), Int32(w), Int32(h),
                          t, rotate, scale)
    }

    @MainActor
    public static func bltm(x: Int, y: Int,
                            tilemap: Int,
                            u: Int, v: Int, w: Int, h: Int,
                            transparent: Color? = nil,
                            rotate: Double = 0,
                            scale: Double = 1) {
        guard let engine = Runtime.engine else { return }
        let t = transparent.map { Int32($0.index) } ?? Int32(-1)
        pyxift_engine_bltm(engine,
                           Int32(x), Int32(y),
                           Int32(tilemap),
                           Int32(u), Int32(v), Int32(w), Int32(h),
                           t, rotate, scale)
    }

    @MainActor
    public static func elli(x: Int, y: Int, w: Int, h: Int, color: Color) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_elli(engine, Int32(x), Int32(y), Int32(w), Int32(h), color.index)
    }

    @MainActor
    public static func ellib(x: Int, y: Int, w: Int, h: Int, color: Color) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_ellib(engine, Int32(x), Int32(y), Int32(w), Int32(h), color.index)
    }

    @MainActor
    public static func fill(x: Int, y: Int, color: Color) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_fill(engine, Int32(x), Int32(y), color.index)
    }

    @MainActor
    public static func dither(_ alpha: Double) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_dither(engine, alpha)
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

public enum ScreenMode: Int32 {
    case crisp = 0
    case smooth = 1
    case retro = 2
}

extension Pyx {
    public static let VERSION: String = "0.7.0"

    @MainActor
    public static func fullscreen(_ enabled: Bool) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_set_fullscreen(engine, enabled)
    }

    @MainActor
    public static var isFullscreen: Bool {
        guard let engine = Runtime.engine else { return false }
        return pyxift_engine_fullscreen(engine)
    }

    @MainActor
    public static func resize(width: Int, height: Int) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_resize(engine, Int32(width), Int32(height))
    }

    @MainActor
    public static func screenMode(_ mode: ScreenMode) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_set_screen_mode(engine, mode.rawValue)
    }

    @MainActor
    public static var currentScreenMode: ScreenMode {
        guard let engine = Runtime.engine else { return .crisp }
        return ScreenMode(rawValue: pyxift_engine_screen_mode(engine)) ?? .crisp
    }

    @MainActor
    public static func integerScale(_ enabled: Bool) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_set_integer_scale(engine, enabled)
    }

    @MainActor
    public static var isIntegerScale: Bool {
        guard let engine = Runtime.engine else { return false }
        return pyxift_engine_integer_scale(engine)
    }

    @MainActor
    public static func perfMonitor(_ enabled: Bool) {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_set_perf_monitor(engine, enabled)
    }

    @MainActor
    public static var isPerfMonitor: Bool {
        guard let engine = Runtime.engine else { return false }
        return pyxift_engine_perf_monitor(engine)
    }

    @MainActor
    public static func icon(data: [String], scale: Int = 1, colorKey: Color? = nil) {
        guard let engine = Runtime.engine else { return }
        let h = data.count
        guard h > 0, let firstRow = data.first else { return }
        let w = firstRow.count
        guard w > 0, scale >= 1 else { return }
        let outW = w * scale
        let outH = h * scale
        var rgba = [UInt8](repeating: 0, count: outW * outH * 4)
        for (y, row) in data.enumerated() {
            let chars = Array(row)
            guard chars.count == w else { return }
            for x in 0..<w {
                let cidx = hexDigitToColorIndex(chars[x])
                let isTransparent = (colorKey?.index ?? 0xff) == cidx
                let rgb = paletteRGB(at: Int(cidx))
                let r = UInt8((rgb >> 16) & 0xff)
                let g = UInt8((rgb >> 8) & 0xff)
                let b = UInt8(rgb & 0xff)
                let a: UInt8 = isTransparent ? 0 : 0xff
                for dy in 0..<scale {
                    let py = y * scale + dy
                    for dx in 0..<scale {
                        let px = x * scale + dx
                        let off = (py * outW + px) * 4
                        rgba[off + 0] = r
                        rgba[off + 1] = g
                        rgba[off + 2] = b
                        rgba[off + 3] = a
                    }
                }
            }
        }
        rgba.withUnsafeBufferPointer { buf in
            pyxift_engine_set_icon(engine, buf.baseAddress, Int32(outW), Int32(outH))
        }
    }

    @MainActor
    public static func screenshot(scale: Int = 2, filename: String? = nil) {
        guard let engine = Runtime.engine else { return }
        let path = filename ?? defaultCapturePath(extension: "png")
        _ = path.withCString { c in
            pyxift_engine_screenshot(engine, c, Int32(scale))
        }
    }

    @MainActor
    public static func show() {
        guard let engine = Runtime.engine else { return }
        pyxift_engine_show(engine)
    }
}

@MainActor
private func hexDigitToColorIndex(_ ch: Character) -> UInt8 {
    if let d = ch.hexDigitValue {
        return UInt8(d & 0x0f)
    }
    return 0
}

@MainActor
private func paletteRGB(at index: Int) -> UInt32 {
    return pyxift_default_palette(UInt8(index & 0x0f))
}

#if canImport(Foundation)
import Foundation

@MainActor
private func defaultCapturePath(extension ext: String) -> String {
    let now = Date()
    let formatter = DateFormatter()
    formatter.dateFormat = "yyyyMMdd-HHmmss"
    let stamp = formatter.string(from: now)
    let basename = "pyxift-\(stamp).\(ext)"
    let dir = FileManager.default.urls(for: .desktopDirectory, in: .userDomainMask).first
        ?? URL(fileURLWithPath: NSHomeDirectory())
    return dir.appendingPathComponent(basename).path
}
#else
@MainActor
private func defaultCapturePath(extension ext: String) -> String {
    return "pyxift.\(ext)"
}
#endif

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
