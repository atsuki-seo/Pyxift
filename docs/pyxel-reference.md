# Pyxel本家から流用するデータ・数値の一覧

出典: [kitao/pyxel](https://github.com/kitao/pyxel) （MIT License, Copyright (c) 2018-2026 Takashi Kitao）

該当ファイルは `crates/pyxel-core/src/settings.rs`（クローン物では `~/ドキュメント/pyxel/...`）。

Pyxift もMITライセンスで配布する前提で、これらは流用箇所のソースファイル冒頭に出典コメントを残した上で取り込む。

## 既定16色パレット

```
0  BLACK       0x000000
1  NAVY        0x2b335f
2  PURPLE      0x7e2072
3  GREEN       0x19959c
4  BROWN       0x8b4852
5  DARK_BLUE   0x395c98
6  LIGHT_BLUE  0xa9c1ff
7  WHITE       0xeeeeee
8  RED         0xd4186c
9  ORANGE      0xd38441
10 YELLOW      0xe9c35b
11 LIME        0x70c6a9
12 CYAN        0x7696de
13 GRAY        0xa3a3a3
14 PINK        0xff9798
15 PEACH       0xedc7b0
```

（Pyxel `DEFAULT_COLORS` より）

## 制約数値

| 項目 | 値 | Pyxel定数名 |
| --- | --- | --- |
| FPS 既定 | 30 | `DEFAULT_FPS` |
| 色数 | 16 | `NUM_COLORS` |
| 画像バンク数 | 3 | `NUM_IMAGES` |
| 画像サイズ | 256×256 | `IMAGE_SIZE` |
| タイルマップ数 | 8 | `NUM_TILEMAPS` |
| タイルマップサイズ | 256×256 | `TILEMAP_SIZE` |
| タイルサイズ | 8×8 | `TILE_SIZE` |
| 音声チャンネル数 | 4 | `NUM_CHANNELS` |
| 音色数 | 4（三角/矩形/パルス/ノイズ） | `NUM_TONES` |
| サウンド数 | 64 | `NUM_SOUNDS` |
| ミュージック数 | 8 | `NUM_MUSICS` |
| 音声サンプルレート | 22050 Hz | `AUDIO_SAMPLE_RATE` |
| 音声ビット深度 | 16 bit | `AUDIO_SAMPLE_BITS` |

## 内蔵フォント

- 文字範囲: ASCII 0x20〜0x7F（96 グリフ）
- グリフサイズ: 4×6 ピクセル（`FONT_WIDTH=4`, `FONT_HEIGHT=6`）
- データ形式: 各グリフ `u32`（24ビット使用、ビットパッキング）
- 配列: `FONT_DATA: [u32; 96]`

実装メモ: 4×6 = 24ビットを上位から行順・列順で埋めている（要確認）。Pyxift では同じ配列を Swift `[UInt32]` か C++ `std::array<uint32_t,96>` として持ち、テキスト描画時にビット展開する。

## 背景色

`BACKGROUND_COLOR = 0x202224`（パレット外、ウィンドウのレターボックス領域用）

## 流用しないもの

- アイコンデータ（`ICON_DATA`）— Pyxift 独自のアイコンを用意する
- カーソルデータ（`CURSOR_DATA`）— v0.1 では SDL3 標準カーソルを使う
- 音色テーブル（`DEFAULT_TONE_*`）— v0.2 で実装する際に再評価（そのまま使う可能性が高い）
