import CPyxiftCore

public enum Button: UInt8, Sendable {
    case left = 0
    case right = 1
    case up = 2
    case down = 3
    case a = 4
    case b = 5
    case start = 6
}

public enum MouseButton: UInt8, Sendable {
    case left = 0
    case right = 1
    case middle = 2
}
