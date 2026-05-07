# Pyxift

Pyxel風のレトロ2Dゲームエンジン（Swift製・C++コア・SDL3バックエンド）。Linux と macOS が対象。

> ⚠️ **構想段階**: コードはまだありません。設計判断と未解決事項が `docs/` に置いてあります。

## ねらい

- **Pyxel互換の制約感**: 16色固定パレット・最大256×256・4チャンネル音声など、Pyxelの数値仕様に揃える
- **Swiftらしい書き味**: 名前空間 `Pyx` 配下の静的関数 ＋ 列挙体・ラベル付き引数で型安全化、`App` プロトコルでライフサイクル
- **SPM単体で完結**: Swift Package Manager のパッケージとして配布。CMake は使わない
- **数十関数で完結**: API表面を絞り、覚えるべき関数を意図的に少なく保つ

## イメージ

```swift
import Pyxift

struct MyGame: App {
    var x = 80, y = 60

    mutating func update() {
        if Pyx.button(.left)  { x -= 1 }
        if Pyx.button(.right) { x += 1 }
    }

    func draw() {
        Pyx.cls(color: 0)
        Pyx.pset(x: x, y: y, color: 7)
        Pyx.text(x: 4, y: 4, "hello", color: 7)
    }
}

@main
struct Main { static func main() { Pyx.run(MyGame(), width: 160, height: 120) } }
```

## ステータス

| バージョン | 内容 |
| --- | --- |
| v0.1.0（未着手） | 描画・入力・固定タイムステップループ・内蔵フォント・PNGロード |
| v0.2.0（未着手） | 音声合成（4ch・矩形/三角/パルス/ノイズ） |
| v0.3.0（未着手） | アセットバンドル `.pyxift` 形式 |

## ライセンス

MIT License を予定（コード追加時に LICENSE ファイルを設置）。

Pyxel（MIT License, Copyright (c) 2018-2026 Takashi Kitao）の数値仕様・既定パレット・内蔵フォントデータを参考または流用する予定。流用箇所は該当ファイル内で明示する。

## ドキュメント

- [docs/decisions.md](docs/decisions.md) — 設計判断のサマリ
- [docs/open-questions.md](docs/open-questions.md) — 未解決事項とその解決状況
- [docs/api-sketch.md](docs/api-sketch.md) — Swift API のラフスケッチ
- [docs/pyxel-reference.md](docs/pyxel-reference.md) — Pyxel本家から流用する数値・データの一覧
