// swift-tools-version: 6.0
import PackageDescription

let package = Package(
    name: "Pyxift",
    platforms: [
        .macOS(.v13),
    ],
    products: [
        .library(name: "Pyxift", targets: ["Pyxift"]),
        .executable(name: "PyxiftDemo", targets: ["PyxiftDemo"]),
    ],
    targets: [
        .systemLibrary(
            name: "CSDL3",
            pkgConfig: "sdl3",
            providers: [
                .brew(["sdl3"]),
                .apt(["libsdl3-dev"]),
            ]
        ),
        .target(
            name: "CPyxiftCore",
            dependencies: ["CSDL3"],
            path: "Sources/CPyxiftCore",
            publicHeadersPath: "include",
            cxxSettings: [
                .headerSearchPath("src"),
            ],
            linkerSettings: [
                .linkedLibrary("m", .when(platforms: [.linux])),
            ]
        ),
        .target(
            name: "Pyxift",
            dependencies: ["CPyxiftCore"],
            path: "Sources/Pyxift",
            swiftSettings: [
                .swiftLanguageMode(.v6),
            ]
        ),
        .executableTarget(
            name: "PyxiftDemo",
            dependencies: ["Pyxift"],
            path: "Examples/PyxiftDemo",
            resources: [
                .copy("assets"),
            ],
            swiftSettings: [
                .swiftLanguageMode(.v6),
            ]
        ),
        .testTarget(
            name: "PyxiftTests",
            dependencies: ["Pyxift", "CPyxiftCore"],
            path: "Tests/PyxiftTests",
            swiftSettings: [
                .swiftLanguageMode(.v6),
            ]
        ),
    ],
    cxxLanguageStandard: .cxx17
)
