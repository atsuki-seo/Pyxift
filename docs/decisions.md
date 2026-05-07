# 設計判断サマリ

`grill-me` セッション（2026-05-07）で確定した内容。

## 言語・配布

- **ユーザー言語**: Swift のみ。C++ はエンジン内部実装で、純Cヘッダ越しに Swift から呼ぶ
- **配布形式**: Swift Package Manager (SPM) パッケージ単体で完結。CMake は使わない
- **依存**: SDL3（mac=Homebrew、Linux=apt または手動ビルド）。`SystemLibrary` ターゲットで参照
- **ライセンス**: MIT
- **公開**: GitHub Public、SPM Git URL 直接参照、Swift Package Index 登録は v0.1.0 タグ後
- **対象OS**: Linux と macOS（Windows・iOS・Android は対象外）

## 制約（Pyxel完全準拠）

- 解像度: 最大 256×256（コンストラクタ可変）
- 色数: 16色固定パレット（Pyxel既定パレット）
- 画像バンク: 3枚（各 256×256）
- タイルマップ: 8枚（各 256×256、タイル 8×8）
- サウンド: 4チャンネル、波形は 矩形/三角/パルス/ノイズ
- FPS: 30 既定（可変）

## API形状

- 名前空間 `Pyx` 配下の静的関数（caseless enum）で衝突回避しつつ短い関数名を維持
- ボタン・色などは Int リテラルではなく `Button.left` `Color.red` 等の列挙で型安全
- ライフサイクルは `App` プロトコル（`mutating func update()` / `func draw()`）＋ `Pyx.run(_:width:height:)`
- エラー処理: `throws` 不使用。回復不能エラーは `fatalError`、回復可能は Optional
- ゲームループ: 固定タイムステップ（30FPS）、最大2回までキャッチアップ、描画は VSync 同期

## 描画プリミティブ（Pyxel互換セット）

`cls` / `pset` / `line` / `rect` / `rectb` / `circ` / `circb` / `tri` / `trib` / `blt` / `bltm` / `text` / `clip` / `pal` / `camera`

## 入力

- キーボード ＋ マウス ＋ ゲームパッド（SDL3 GameController サブシステム経由）
- 仮想ボタン抽象 `.left/.right/.up/.down/.a/.b/.start` でキー＆パッド統合
- タッチ・加速度センサーは対象外

## 音声（v0.2 以降）

- C++ コア層で自前ソフトウェア合成（矩形/三角/パルス/ノイズ）
- SDL3 `SDL_AudioStream` に PCM を流し込む
- MML的な短いテキスト記法で曲を書く（Pyxel `sound.set("c3e3g3", ...)` 風）

## アセット形式（v0.2 以降）

- 単一バンドルファイル `.pyxift`（zip + JSON + PNG 構造）
- ロードAPI: `Pyx.load("mygame.pyxift")` 1関数
- v0.1 では素のPNGを `Pyx.loadImage("hero.png")` で読む簡易APIのみ

## テキスト

- 内蔵ビットマップフォント1種類（ASCII範囲のみ・固定サイズ 4×6 ピクセル）
- 日本語・カスタムフォント・TTFは v0.1/v0.2 では非対応

## ホットリロード

- 非対応。`swift run` で都度ビルド＆起動

## v0.1.0 スコープ

- ウィンドウ生成・固定タイムステップループ
- 描画プリミティブ全部
- キーボード・マウス・ゲームパッド入力
- 内蔵ビットマップフォント
- 16色固定パレット・最大 256×256
- `App` プロトコル ＋ `Pyx.run()` の Swift API
- SDL3 SystemLibrary 経由の SPM パッケージ
- `Pyx.loadImage("hero.png")` で素のPNGを読む簡易API

v0.2 以降に: 音声・`.pyxift` バンドル形式
