# Pyxift

**Pyxift** /pɪkswɪft/（ピクスイフト） — Pyxel + Swift。

Pyxel風のレトロ2Dゲームエンジン（Swift製・C++コア・SDL3バックエンド）。Linux と macOS が対象。

## Getting Started

事前に SDL3 をインストールしてから `swift build` する。

**macOS:**

```sh
brew install sdl3
```

**Linux (Debian/Ubuntu):**

```sh
sudo apt install libsdl3-dev
```

`libsdl3-dev` が見つからない場合は [SDL 公式リリース](https://github.com/libsdl-org/SDL/releases) からソースビルドする:

```sh
git clone --depth 1 --branch release-3.2.x https://github.com/libsdl-org/SDL.git
cmake -S SDL -B SDL/build -DCMAKE_BUILD_TYPE=Release
cmake --build SDL/build -j
sudo cmake --install SDL/build
sudo ldconfig
```

### 同梱サンプルを試す

リポジトリを clone して同梱サンプルを実行する:

```sh
git clone https://github.com/atsuki-seo/Pyxift.git
cd Pyxift
swift run PyxiftDemo
```

### SPM 依存として使う

自分の Swift パッケージから Pyxift を使うには、`Package.swift` に依存を追加する:

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

最小のゲームコード:

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

`swift run` で起動する。

## ライセンス

MIT License。詳細は [LICENSE](LICENSE) 参照。

Pyxel（MIT License, Copyright (c) 2018-2026 Takashi Kitao）の数値仕様・既定パレット・内蔵フォントデータを流用している。流用箇所は該当ファイル冒頭に出典コメントを明記。

## ドキュメント

- [docs/decisions.md](docs/decisions.md) — 設計判断
- [docs/status.md](docs/status.md) — 未達タスク・未解決事項・ロードマップ
- [docs/pyxel-reference.md](docs/pyxel-reference.md) — Pyxel本家から流用する数値・データの一覧
