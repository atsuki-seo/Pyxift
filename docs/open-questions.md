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
