# 実装計画

`grill-me` セッション（2026-05-07）で確定した、Pyxift の実装ロードマップ。
設計判断（`decisions.md`）が「**何を作るか**」、本ドキュメントは「**どの順で作るか**」を扱う。

## 全体方針

- **垂直スライス先行**: SPM↔C++↔SDL3 の連結という最大の不確実性を M1 で潰してから、各機能を肉付けする
- **リリース間隔は緩い**: v0.1.0 タグ後すぐ v0.2.0 に着手してよい。明示的な安定化期間は設けない（hotfix が必要になったら都度ブランチを切る）
- **ピクセル完全一致は目指さない**: 描画は教科書アルゴリズムで実装、本家サンプル移植時に 1〜2px 違ってよい（`decisions.md` 既出）

## 言語・ツールチェーン

- **Swift**: tools-version 6.0、swift-language-mode 6（strict concurrency 前提）
  - ゲームループは `@MainActor` 単一実行で押さえる
  - C関数橋渡しは `nonisolated(unsafe)` で明示
- **C++**: C++17（`cxxLanguageStandard: .cxx17`）。コア層の実装言語
- **C境界**: 手書きヘッダ `pyxift_c.h` + opaque pointer (`PyxiftEngine*`) の1個方式
  - Swift C++ interop（`.interoperabilityMode(.Cxx)`）は採用しない。理由:
    - 公式ドキュメント（swift.org/documentation/cxx-interop/status）が「actively evolving feature」と明記。Swift 6.2 で safe interop モードが新設されたばかりで設計が固まりきっていない
    - Linux 側で既知のリグレッションが現役（例: Glibc import 経由の library evolution エラー、Foundation との組み合わせ問題）。Pyxift は Linux 公式サポートなので踏み抜きリスクが許容できない
    - SwiftPM 側にも未解決の連動バグ（実行ターゲットの clang++ リンク自動切替が効かないなど）
    - 利得が薄い: Pyxift の境界は描画プリミティブのC関数50個程度で、std::vector / std::string / テンプレートなどを跨いで渡す必要がない単純な境界。手書きCヘッダのコストが interop バグ調査のコストより明確に安い
  - 描画プリミティブごとに薄いC関数を並べる（50個前後を許容）
- **SDL3**: `SystemLibrary` ターゲット（mac=Homebrew、Linux=apt or ソースビルド）

## SPM パッケージ構成（予定）

```
Package.swift              // tools-version: 6.0
Sources/
├── CSDL3/                 // systemLibrary, pkgConfig: "sdl3"
├── CPyxiftCore/           // .target, C++17, depends on CSDL3
│   ├── include/pyxift_c.h // 公開Cヘッダ（opaque pointer）
│   └── src/*.{cpp,hpp}    // Engine / Canvas / Input / Audio など
└── Pyxift/                // .target, Swift, depends on CPyxiftCore
    ├── App.swift          // App プロトコル
    ├── Pyx.swift          // 名前空間 + run/quit/title/...
    ├── Color.swift        // Color 列挙互換
    ├── Button.swift       // Button / Key / MouseButton
    └── Internals/         // C関数呼び出しラッパー
Tests/
└── PyxiftTests/           // ユニットテスト（コア層は SwiftTesting）
Examples/
└── PyxiftDemo/            // executableTarget（手動動作確認用）
```

---

## v0.1.0 マイルストーン

### M1 — 垂直スライス（"hello pixel"）

最大の不確実性（SPM↔C++↔SDL3 連結）を潰す回。

**完了条件:**
- [x] `swift run PyxiftDemo` で 160×120 のウィンドウが開く
- [x] 画面中央に1ピクセル（`Pyx.pset` で打った白）が表示される
- [x] 30FPS で `frameCount` がインクリメントしている（タイトルバーに数値出して目視確認）
- [x] ESC または ウィンドウ閉じるボタンで `Pyx.run` から正常 return する
- [x] CI（macOS + Ubuntu の `swift build`）が緑

含まれるもの: SPM 骨格、`PyxiftEngine` の最小実装、固定タイムステップループ、フレームバッファ→ウィンドウ転送（SDL_Texture, nearest neighbor で整数倍拡大）、最小Swift API（`App` / `Pyx.run` / `Pyx.cls` / `Pyx.pset`）。

### M2 — 描画プリミティブ全部

3 段階に細分化。

**M2a — 単純塗り系（state 依存なし）**
- [x] `line`（Bresenham）
- [x] `rect` / `rectb`
- [x] `circ` / `circb`（midpoint circle）
- [x] `tri` / `trib`（線のみ、塗りは M2c で）

**M2b — 状態系**
- [x] `clip` / `camera` をエンジン状態として保持し、M2a の全関数を state 経由に通すリファクタ
- [x] `pal` 設定の保持。`blt` の透明色判定はパレット適用前のソース色で行う（`decisions.md` 7 番）

**M2c — 転送系**
- [x] `tri`/`trib` の塗り完成（scanline fill）
- [x] `blt` / `bltm`（画像バンク・タイルマップへのアクセス、`pal` と透明色の合成）
- [x] `text`（API シグネチャと clip/camera 連携。フォントデータは M4 で流し込み、M2c 時点はダミーフォントでテスト）

### M3 — 入力系

順序: キーボード → マウス → ゲームパッド。

- [ ] `update` 直前にイベントキューを 1 回ドレイン、bit フラグで蓄積
- [ ] `button(_:)` / `buttonPressed(_:)` / `buttonReleased(_:)`: 仮想ボタン抽象（`decisions.md` 4 番のマッピング）
- [ ] `key(_:)`: SDL3 SDLK_* と 1:1 の `Key` 列挙（命名規則は `decisions.md` 10 番）。`SDL_keycode.h` から自動生成スクリプトで吐く
- [ ] `mouse()` / `mouseWheel` / `mouseButton(_:)`
- [ ] ゲームパッド: SDL3 `SDL_JoystickID` をプラットフォーム層で接続順 0..3 に正規化、`Pyx.button(.a, player: 0..3)`（`decisions.md` 9 番）

**テスト戦略**: コア層 `InputState`（pure C++、SDL3 非依存）は `VirtualEvent` 列を注入してユニットテスト。SDL3 アダプタ層は `SDL_Event` → `VirtualEvent` 変換のみを担い、テストはせず手動・実機確認に委ねる。これで CI から SDL3 初期化を完全に切り離せる。

### M4 — アセット & 内蔵フォント

- 内蔵フォント: 本家 `FONT_DATA`（u32 ビットパッキング、4×6、ASCII 0x20〜0x7F）を C++ ヘッダに配列リテラルとして埋め込み。Pyxift のフォントモジュール冒頭に出典コメント
- `Pyx.loadImage(_:into:)`: PNG 復号で `stb_image.h` を C++ コアに同梱（単一ヘッダ・MIT）。RGB→16色パレット最近傍マッピングで画像バンクへ書き込み
  - 挙動は **Pyxel本家 `Image::from_image`（`crates/pyxel-core/src/image.rs:57`）に揃える**:
    - PNG を RGB に展開（α チャンネルは捨てる、Pyxel 同様）
    - 各 RGB ピクセルを Pyxel既定パレット16色とユークリッド距離（二乗）で最近傍マッピング
    - ユニーク RGB → Color のマップを `unordered_map` でキャッシュし、同じ色の再計算を回避
    - パレット外の色でも警告は出さない（毎ピクセル発火しがちで邪魔）
    - 本家の `include_colors: true` モード（PNG からパレットを動的取り込み）は v0.1 では非対応。Pyxift は「16色固定パレット = Pyxel既定」が方針なので機能自体が矛盾
- 画像バンク 3 枚 × 256×256 の領域確保、`blt` から参照

### M5 — 仕上げと v0.1.0 タグ

- 残 API: `pget` / `quit` / `title` / `frameCount` / `width` / `height` / `mouseWheel` / `mouseCursor`
- `Pyx.quit()` の挙動: 内部フラグを立てるだけ、現フレームの update/draw は完了してから break（`decisions.md` 8 番）

**v0.1.0 タグ前のチェックリスト:**
- [ ] サンプル「ボール跳ね返し」（`Examples/PyxiftDemo`）が Linux と macOS の両方で30秒連続動作・コマ落ちなし
- [ ] README の Getting Started 手順を空ディレクトリから再実行して通る（自分の開発環境ではなく、ホーム直下の素のフォルダで `swift package init` から）
- [ ] CI の build job が両 OS で緑
- [ ] 本家流用箇所（FONT_DATA / DEFAULT_COLORS）の出典コメントと LICENSE ファイル設置を grep で確認
- [ ] Linux 側の SDL3 取得手順を README に明記（apt が古い場合のソースビルド手順含む）
- [ ] `swift build -Xswiftc -warnings-as-errors` で警告ゼロ
- [ ] v0.1.0 タグ → Swift Package Index 登録申請

---

## v0.2.0 マイルストーン — 音声系

### M6 — 音声基盤

- [ ] C++ コアに `AudioMixer`（48kHz・モノラル・S16・4ch 加算ミックス）を仮実装
- [ ] SDL3 `SDL_AudioStream` 接続、無音 PCM を流して再生確認
- [ ] Swift API: `Pyx.play(channel:sound:loop:)` / `Pyx.stop(channel:)` のスタブ

**スレッディングモデル**: 音声合成はオーディオコールバックスレッドで同期生成。メインスレッドからの操作は SPSC ロックフリーリングバッファ経由で `AudioCommand`（POD）を push し、コールバックが pop して合成。SDL3 ロックは使わない（バッファアンダーラン回避）。Swift 側は `@MainActor` のまま、内部で C 関数 → リングバッファ push なので concurrency 違反なし。

### M7 — 波形ジェネレータ

- [ ] 矩形 / 三角 / パルス / ノイズ の 4 オシレータ
- [ ] 本家 `DEFAULT_TONE_*` 定数を C++ ヘッダにバイト単位で流用（**出典コメント必須**）
- [ ] 単音（1音×1ch）が鳴る所まで

### M8 — MML パーサ & Sound オブジェクト

- [ ] `notes` / `tones` / `volumes` / `effects` / `speed` 文字列のパース
- [ ] `Pyx.sound(_:notes:tones:volumes:effects:speed:)` で定義 → `play` で再生
- [ ] エフェクト: slide / vibrato / fadeout / none

### M9 — v0.2.0 仕上げ

- [ ] 4ch 同時再生検証
- [ ] サンプル音源を `PyxiftDemo` に追加
- [ ] v0.2.0 タグ

---

## v0.3.0 マイルストーン — `.pyxift` バンドル形式

### M10 — フォーマット確定

- [ ] `open-questions.md` 5 番の暫定仕様を実装ベースで詰める
- [ ] manifest.json スキーマ・images/tilemaps/sounds の入れ子構造を固定
- [ ] バリデータを書く（壊れた `.pyxift` で fatalError しない）

### M11 — ローダ

- [ ] zip 展開ライブラリ **miniz**（単一ヘッダ・public domain / MIT-0）を `Sources/CPyxiftCore/vendor/` に同梱
- [ ] `Pyx.load(_:)`: zip 展開 → manifest 読み → images/tilemaps/sounds をバンク・スロットへ流し込み
- [ ] 既存 `Pyx.loadImage` との共存ルール: `load` 後に `loadImage` で部分上書きOK

### M12 — パッカ CLI

- [ ] SPM `executableTarget` として `pyxift-pack` コマンド
- [ ] ディレクトリ → `.pyxift` 生成
- [ ] エディタは作らない（外部ツール + 手書き JSON 想定）

### M13 — v0.3.0 仕上げ

- [ ] サンプルゲームを `.pyxift` 化
- [ ] v0.3.0 タグ

---

## v0.4.0 以降（粒度を粗く・順不同）

- 描画追補: `elli` / `ellib` / `fill` / `dither` / `blt3d` / `bltm3d`
- システム追補: `fullscreen` / `resize` / `screenshot` / `integer_scale` / `icon` / `load_pal` / `save_pal`
- 入力追補: `btnv`（アナログ軸）/ `input_text` / `dropped_files`
- サンプル集を別リポで 5 本程度
- Swift Package Index のメンテ

---

## 開発運用

- **ブランチ運用**: 1人開発・速度優先のため、main 直接コミット。PR は使わない
- **マイルストーン区切り**: 完了時に `git commit --allow-empty -m "M<番号>: ..."` でプレフィックス付き empty コミットを打つ。後から `git log --grep '^M[0-9]'` で節目を抽出可能
- **タグ**: v0.1.0 / v0.2.0 / v0.3.0 のリリース版のみ。中間マイルストーンにはタグを打たない
- **CI 導入タイミング**: M1 完了直後に GitHub Actions（macos-latest + ubuntu-latest、`swift build` のみ）。`swift test` は M2 以降テストが書かれてから追加。public リポなので両 OS とも runner は無料
- **Examples**: 同一 SPM パッケージ内に `executableTarget` として `Examples/PyxiftDemo` を含める。別リポにはしない
- **SDL3 取得手順**: M1〜M4 期間中は開発者環境に sdl3 が pkg-config で見える前提で進める。Linux/mac それぞれの取得手順を README に書くのは M5 でまとめる

## 横断的なテスト方針

- **コア層（C++）**: ピクセル完全一致を諦めるので、ユニットテストは「クリップ矩形を超えた pset が落ちない」「`pal` 設定下で `blt` の出力色が期待通り」など**意味論レベル**で書く
- **Swift API**: `@MainActor` 化された `Pyx` の挙動はゴールデンファイル比較が難しいため、`PyxiftDemo` での手動動作確認 + 各マイルストーン末尾のチェックリストで担保
- **CI**: GitHub Actions で macOS と Ubuntu の両方で `swift build` と `swift test` を走らせる（M1 完了時点で導入）

## ライセンス・出典コメント運用

- `FONT_DATA` / `DEFAULT_COLORS` / `DEFAULT_TONE_*` 流用箇所のヘッダコメントに必ず出典明記:
  ```
  // Source: kitao/pyxel crates/pyxel-core/src/settings.rs
  // License: MIT (Copyright (c) 2018-2026 Takashi Kitao)
  ```
- `LICENSE` ファイル設置は M1 で実施（v0.1.0 タグ前に必ず）
