# 設計判断サマリ

`grill-me` セッション（2026-05-07）で確定した内容。
2026-05-07 の本家（kitao/pyxel）コード突き合わせセッションで追記あり（末尾「Pyxel本家との突き合わせ」セクション）。

## 言語・配布

- **ユーザー言語**: Swift のみ。C++ はエンジン内部実装で、純Cヘッダ越しに Swift から呼ぶ
- **配布形式**: Swift Package Manager (SPM) パッケージ単体で完結。CMake は使わない
- **依存**: SDL3（mac=Homebrew、Linux=apt または手動ビルド）。`SystemLibrary` ターゲットで参照
- **ライセンス**: MIT
- **公開**: GitHub Public、SPM Git URL 直接参照、Swift Package Index 登録は v0.1.0 タグ後
- **対象OS**: Linux と macOS（Windows・iOS・Android は対象外）

## 制約（Pyxel完全準拠）

数値はすべて Pyxel 既定値に揃える。具体値（解像度・色数・画像バンク数・タイルマップ・FPS・音声チャンネル数など）の一覧は `pyxel-reference.md` の「制約数値」表を参照。

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

- 内蔵ビットマップフォント1種類（仕様詳細は `pyxel-reference.md` の「内蔵フォント」節を参照）
- 日本語・カスタムフォント・TTFは v0.1/v0.2 では非対応

## ホットリロード

- 非対応。`swift run` で都度ビルド＆起動

## Pyxel本家との突き合わせ

本家リポジトリの場所・主要ファイル・スコープ外ディレクトリは `CLAUDE.md` の「Pyxel 本家リポジトリの参照」節を参照（一次台帳）。突き合わせ対象は `crates/pyxel-core/`（エンジン本体）と `python/pyxel/__init__.pyi`（公開API表面）の2点に限定。

### エンジン実装の取り込み方針

- **本家コードはコピーしない。仕様参照して C++ で書き直す**。
  - 理由: 本家は Rust、Pyxift は C++ コア。依存（`glow`/`blip_buf`/`sdl2-sys`）も違う。本家には editor / screencast / tmx_parser / bgm_generator / shaders 等 v0.1 不要なものが大量に同梱されている。
- **数値データはバイト単位で流用**: `DEFAULT_COLORS` / `FONT_DATA` / `DEFAULT_TONE_*`（v0.2）は配列リテラルを Swift/C++ に機械変換するだけ。
- **代替案として検討した「Rustコアを直接FFIで使う」は不採用**: C++コア・SPM単体配布・CMake不使用という根本方針を覆すため。

### 描画アルゴリズムの再現精度

- **ピクセル完全一致は目指さない**。線/円/三角形は教科書アルゴリズム（Bresenham / midpoint circle / scanline）で実装。
- **意味論は本家準拠**: `clip` / `camera` / `pal` / 透明色 / `blt` のソース矩形指定の合成順序は本家と一致させる。
- 結果として、本家のサンプルゲームを移植した際にラスタライズで 1〜2 ピクセル違うことは許容する。

### 座標・整数型・範囲外アクセス

- 描画 API は **`Int` 受け**（本家は `float` 受け、内部 floor）。Pyxift では小数座標 API は提供しない。
- **画面外座標は黙ってクリップ**、**負のサイズは no-op**。`throws` 不使用方針と一致。
- デバッグビルドでの warn ログは v0.1 では実装しない（過剰最適化を避ける）。

### 入力フレーム境界

- **update 呼び出しの直前に SDL3 イベントキューを 1 回ドレイン**し、フレーム中の押下/離上イベントを bit フラグで蓄積。
- `button(_:)` は最終状態、`buttonPressed(_:)`/`buttonReleased(_:)` はそのフレームに発生したかどうか。
- 1 フレーム内で押→離→押が起きた場合、`Pressed` / `Released` は両方 true、`button` は最終状態（false→true→false なら最終 false）。
- これは Pyxel本家と同じエッジ蓄積方式。30FPS 固定下でのチャタリング対策として最終状態優先で割り切る。

### 公開API表面の Pyxel本家との対応

Pyxel本家にあって Pyxift では関数名が異なるもの:

- `Pyx.mouseCursor(visible: Bool)` ← 本家 `mouse(visible:)`（関数名衝突回避のためリネーム）

editor 連携機能は永久不採用。

### 数学・乱数 API：Pyxel仕様より Swift 慣用を優先

Pyxel 公開 API の `ceil` / `floor` / `clamp` / `sgn` / `sqrt` / `sin` / `cos` / `atan2` / `rseed` / `rndi` / `rndf` / `nseed` / `noise` は **Pyxift では提供しない**。

理由: Swift には Foundation の `sin`/`cos`/`sqrt`、`Int.random(in:)` / `Double.random(in:)`、`SystemRandomNumberGenerator` 等が標準で揃っており、Pyxel 互換の薄いラッパーを足すと「Swift らしさ」を毀損する。Perlin noise だけは標準にないが、必要になった時点で別パッケージか v0.1.x で再検討する。

これは「ユーザー向けの書き味は Swift らしさを優先、他の仕様は Pyxel に合わせる」という基本方針の、明確な「Swift側」分岐点。

### キーコード定数の網羅

本家 `KEY_*` 定数は 215 個。Pyxift `enum Key` は SDL3 の `SDL_Keycode`（`SDLK_*`）と 1:1 で網羅する。命名は Swift 慣用に従い `case escape` / `case leftArrow` のように lowerCamelCase。生の SDL3 キーコードへのアクセスは `Key(rawValue:)` で可能にする。
