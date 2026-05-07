import CPyxiftCore

public enum Pyx {}

// ゲームループは @MainActor 単一実行（decisions.md: Swift 6 strict concurrency）。
@MainActor
private enum Runtime {
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
}
