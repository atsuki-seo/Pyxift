# 未解決事項と解決状況

`grill-me` セッション（2026-05-07）で残った6項目を解決した記録。

## 1. Pyxel 既定パレット16色のRGB値 — 解決

Pyxel本家リポジトリ（MIT License, kitao/pyxel）の `crates/pyxel-core/src/settings.rs` の `DEFAULT_COLORS` 定数から取得。詳細は [pyxel-reference.md](pyxel-reference.md) 参照。

ライセンス上、MIT クレジット表記で流用可能。Pyxift もMITで配布するので互換。

## 2. SDL3 配布状況（2026年5月時点） — 解決

- **macOS / Homebrew**: `formulae.brew.sh/formula/sdl3` で正式に formula 提供されている。`brew install sdl3` で即導入可能
- **Linux**:
  - SDL本家は SDL 3.4.x を安定リリース済み
  - 主要ディストリの apt パッケージとしては地域差があり、Debian/Ubuntu の公式リポジトリでは `libsdl3-dev` が利用できるバージョンと、まだソースビルドが必要なバージョンが混在
  - Arch Linux は AUR/community 経由で取得可能
  - Pyxift README には「Linux で `apt install libsdl3-dev` が失敗する場合はソースビルド手順を案内」と書く方針

**判断**: SystemLibrary 方式は維持。Linux で apt が古い場合のフォールバック手順を README に明記する。Pyxift v0.1.0 リリース時点で再確認が必要。

## 3. 内蔵フォントの出典 — 解決

Pyxel本家の `FONT_DATA` 定数（`crates/pyxel-core/src/settings.rs`）を流用する。4×6 ピクセル・ASCII 0x20〜0x7F の 96 グリフ。各グリフは `u32`（24ビット使用）でビットパッキングされている。

MIT License で流用可能。Pyxift のフォントモジュール冒頭に出典コメントを記載する。

## 4. 仮想ボタンの物理マッピング — 解決

```
.left  → KEY_LEFT  / DPAD_LEFT  / Left Stick X (-)
.right → KEY_RIGHT / DPAD_RIGHT / Left Stick X (+)
.up    → KEY_UP    / DPAD_UP    / Left Stick Y (-)
.down  → KEY_DOWN  / DPAD_DOWN  / Left Stick Y (+)
.a     → KEY_Z     / GAMEPAD A
.b     → KEY_X     / GAMEPAD B
.start → KEY_RETURN / GAMEPAD START
```

理由: Z/X 配置は Pyxel・PICO-8・往年のレトロエンジンの定番。スティック軸のしきい値は SDL3 既定（およそ 50%）を使う。v0.1 ではキー再マップAPIは提供しない。

## 5. `.pyxift` フォーマット仕様（v0.2 以降の素案） — 解決（暫定）

zip 圧縮された以下構造:

```
mygame.pyxift (zip)
├── manifest.json
│     {
│       "version": 1,
│       "title": "My Game",
│       "screen": { "width": 160, "height": 120 },
│       "fps": 30
│     }
├── images/0.png    // 256×256, インデックスカラー（パレット16色）
├── images/1.png
├── images/2.png
├── tilemaps/0.json // { "imageBank": 0, "tiles": [[...256x256 タイルID...]] }
├── ...
└── sounds/0.json   // { "notes": "c3e3g3c4", "tones": "0011", "volumes": "7777", "effects": "0000", "speed": 30 }
```

manifest 必須・images / tilemaps / sounds は任意（存在するものだけロード）。実装は v0.2 で詰める。

## 6. プロジェクト名 `Pyxift` の衝突確認 — 解決

2026年5月7日時点の Web 検索で GitHub・Swift Package Index に `Pyxift` を名乗る既存ライブラリは見当たらず。リポジトリ作成時点で確保。

---

# 本家突き合わせセッションで残った未解決事項（2026-05-07 追記）

## 7. `pal` の合成順序（透明色判定 vs パレット差し替え） — 解決

`crates/pyxel-core/src/canvas.rs:803` の `apply_pixel` を確認:

```rust
fn apply_pixel(src: T, transparent: Option<T>, palette: Option<&[T]>) -> Option<T> {
    if transparent.is_some_and(|tkey| src == tkey) {
        return None;  // 透明判定はソース色（パレット差し替え前）で行う
    }
    Some(palette.map_or(src, |pal| pal[src.to_index()]))
}
```

**確定挙動**: `blt` の透明色キーは**ソース画像の元の色**と比較する。`pal(from:to:)` でパレット差し替えを設定していても、透明判定はパレット適用前の値で行う。

実装上の含意: Pyxift の C++ コア `Canvas::blit` も同じ順序で実装する。「`pal(.red, .blue)` を設定した状態で `blt(transparent: .red)` すると、ソース画像の赤ピクセルは透明扱いになり、青ピクセルは赤として描かれる」が期待される挙動ではない（赤→青置換は実行されない、なぜなら赤は透明として弾かれるから）。本家 Pyxel と一致。

## 8. `Pyx.quit()` の意味論 — 解決

`crates/pyxel-core/src/system.rs:149` の `quit()` は `platform::quit()` を呼ぶだけ。プラットフォーム層でループ終了フラグを立て、**現フレームの `update`/`draw` は最後まで実行してから** `run` から正常 return する。例外も panic も発生させない。

**Pyxift 採用挙動**: `Pyx.quit()` は内部フラグを立てるだけで、呼び出し直後にも `update`/`draw` は完了する。`Pyx.run()` の次イテレーション開始時にフラグを見て break、ユーザーの `main` 関数に正常 return する。`fatalError` は使わない。

## 9. ゲームパッドのプレイヤー識別子マッピング — 解決

Pyxel本家の方式（`crates/pyxel-core/src/platform/key.rs:239-274`）:

- ゲームパッドのキーコードは整数値自体にプレイヤー番号をエンコード
- `GAMEPAD_KEY_START_INDEX = 0x50000200`、`GAMEPAD_KEY_STRIDE = 0x100`
- `GAMEPAD1_*` = base、`GAMEPAD2_*` = base+0x100、`GAMEPAD3_*` = base+0x200、`GAMEPAD4_*` = base+0x300
- 最大 4 プレイヤー固定。SDL3 `SDL_JoystickID`（接続デバイス毎の不定 ID）はプラットフォーム層で「接続順インデックス 0..3」に正規化し、3人目以降の接続は無視

**Pyxift 採用挙動**:

- `Pyx.button(.a, player: 0)` の `player` は **接続順インデックス 0..3**（4プレイヤーまで対応）
- 接続が切れた後の再接続は別インデックス扱いにせず、空きスロットの最小値に再割り当て
- 5 台目以降の接続は Pyxift 側でも無視
- 内部キーコード表現は本家と同じ「base + stride×player」方式を採用（C++ コア層とのキーマッピングを単純化）

## 10. `enum Key` の Swift 命名規則 — 解決

SDL3 `SDLK_*` を Swift 慣用に翻訳する規則を以下で固定:

- 修飾子・特殊キー: lowerCamelCase（`SDLK_ESCAPE` → `case escape`、`SDLK_BACKSPACE` → `case backspace`、`SDLK_RETURN` → `case `return`` ※予約語のため backtick）
- 矢印キー: `SDLK_LEFT` → `case leftArrow`（単独 `left` だとマウスボタン等と混同しやすいため）
- ファンクションキー: `SDLK_F1` → `case f1`（数字部分は小文字続け）
- 数字キー: `SDLK_0`〜`SDLK_9` → `case digit0`〜`case digit9`（識別子先頭が数字にできないため接頭辞）
- 文字キー: `SDLK_A`〜`SDLK_Z` → `case a`〜`case z`（小文字 1 文字）
- テンキー: `SDLK_KP_0` → `case keypad0`、`SDLK_KP_PLUS` → `case keypadPlus`
- 記号キー: `SDLK_PLUS` → `case plus`、`SDLK_MINUS` → `case minus`、`SDLK_LEFTBRACKET` → `case leftBracket`
- 修飾キー: `SDLK_LSHIFT` → `case leftShift`、`SDLK_RCTRL` → `case rightControl`

`Key(rawValue: UInt32)` で SDL3 キーコード直アクセス可能（`Key: RawRepresentable`）。網羅方針は `implementation-plan.md` M3 を参照。

## 11. Swift 標準にない `noise` (Perlin) の扱い — 保留継続

決定事項として「v0.1 では提供せず、必要時点で再検討」を維持。現時点で Pyxift から外部依存を増やす理由がないため、ユーザーが `swift-noise` 等の外部パッケージを併用すれば足りる、という前提で v0.1 リリースまで放置する。v0.1.x で「Pyxift 単体でレトロゲームを書ききれない」と判明した場合のみ再検討する。
