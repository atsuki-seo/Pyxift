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
}
