# Pyxift

**Pyxift** /pɪkswɪft/ — Pyxel + Swift.

A Pyxel-style retro 2D game engine (written in Swift, with a C++ core and SDL3 backend). Targets Linux and macOS.

## Getting Started

Install SDL3 first, then run `swift build`.

**macOS:**

```sh
brew install sdl3
```

**Linux (Debian/Ubuntu):**

```sh
sudo apt install libsdl3-dev
```

If `libsdl3-dev` is not available, build from source via the [official SDL releases](https://github.com/libsdl-org/SDL/releases):

```sh
git clone --depth 1 --branch release-3.2.x https://github.com/libsdl-org/SDL.git
cmake -S SDL -B SDL/build -DCMAKE_BUILD_TYPE=Release
cmake --build SDL/build -j
sudo cmake --install SDL/build
sudo ldconfig
```

### Try the bundled sample

Clone the repository and run the included demo:

```sh
git clone https://github.com/atsuki-seo/Pyxift.git
cd Pyxift
swift run PyxiftDemo
```

### Use as an SPM dependency

To use Pyxift from your own Swift package, add it as a dependency in `Package.swift`:

```swift
// swift-tools-version: 6.0
import PackageDescription

let package = Package(
    name: "MyGame",
    platforms: [.macOS(.v13)],
    dependencies: [
        .package(url: "https://github.com/atsuki-seo/Pyxift.git", from: "0.1.0"),
    ],
    targets: [
        .executableTarget(
            name: "MyGame",
            dependencies: [.product(name: "Pyxift", package: "Pyxift")],
            swiftSettings: [.swiftLanguageMode(.v6)]
        ),
    ]
)
```

A minimal game:

```swift
import Pyxift

struct MyApp: App {
    mutating func update() {}

    func draw() {
        Pyx.cls(color: .black)
        Pyx.text(x: 40, y: 56, "Hello, Pyxift!", color: .white)
    }
}

@main
struct Main {
    static func main() {
        Pyx.run(MyApp(), width: 160, height: 120, title: "My Game")
    }
}
```

Launch it with `swift run`.

## License

MIT License. See [LICENSE](LICENSE) for details.

Pyxift adopts numeric specifications, the default palette, and built-in font data from Pyxel (MIT License, Copyright (c) 2018-2026 Takashi Kitao). Each file containing such adopted material carries an attribution comment at its top.

## Documentation

- [docs/decisions.md](docs/decisions.md) — design decisions
- [docs/status.md](docs/status.md) — open tasks, open questions, and roadmap
- [docs/pyxel-reference.md](docs/pyxel-reference.md) — list of numeric values and data adopted from upstream Pyxel
