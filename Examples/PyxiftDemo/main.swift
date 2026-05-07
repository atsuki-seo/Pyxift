import Pyxift

struct Demo: App {
    mutating func update() {
        Pyx.title("Pyxift  frame=\(Pyx.frameCount)")
    }

    func draw() {
        Pyx.cls(color: .black)
        Pyx.pset(x: Pyx.width / 2, y: Pyx.height / 2, color: .white)
    }
}

Pyx.run(Demo(), width: 160, height: 120, title: "Pyxift")
