public struct Color: Equatable, Hashable, ExpressibleByIntegerLiteral, Sendable {
    public let index: UInt8

    public init(integerLiteral value: Int) {
        self.index = UInt8(value & 0x0f)
    }

    public init(_ index: Int) {
        self.index = UInt8(index & 0x0f)
    }
}

extension Color {
    public static let black: Color     = 0
    public static let navy: Color      = 1
    public static let purple: Color    = 2
    public static let green: Color     = 3
    public static let brown: Color     = 4
    public static let darkBlue: Color  = 5
    public static let lightBlue: Color = 6
    public static let white: Color     = 7
    public static let red: Color       = 8
    public static let orange: Color    = 9
    public static let yellow: Color    = 10
    public static let lime: Color      = 11
    public static let cyan: Color      = 12
    public static let gray: Color      = 13
    public static let pink: Color      = 14
    public static let peach: Color     = 15
}
