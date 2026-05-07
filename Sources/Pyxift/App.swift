@MainActor
public protocol App {
    mutating func update()
    func draw()
}
