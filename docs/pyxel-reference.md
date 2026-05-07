# Pyxel本家から流用するデータ・数値の一覧

出典: [kitao/pyxel](https://github.com/kitao/pyxel) （MIT License, Copyright (c) 2018-2026 Takashi Kitao）

Pyxift もMITライセンスで配布する前提で、これらは流用箇所のソースファイル冒頭に出典コメントを残した上で取り込む。
ライセンス全文は [`THIRD_PARTY_LICENSES/pyxel-MIT.txt`](../THIRD_PARTY_LICENSES/pyxel-MIT.txt)、関係性の表明は [`ACKNOWLEDGMENTS.md`](../ACKNOWLEDGMENTS.md) を参照。

<!-- pyxel-upstream-sync -->
## 上流追従ステータス

`/pyxel-sync` スキルが管理する追跡台帳。手で書き換えず、スキル実行で更新する。

- **参照元**: 親ディレクトリの `../pyxel/`（kitao/pyxel の clone）。スキル起動時に `git fetch && git reset --hard origin/main` で上書き同期される
- **Tracked SHA**: `84c674966f0cab0e47cf43a0d39111976b5932f5`
- **Last synced**: 2026-05-08
- **追跡対象ファイル**: 下記「追跡対象テーブル」を参照（SSOT）

差分検出時の運用は `.claude/skills/pyxel-sync/SKILL.md` を参照。
<!-- /pyxel-upstream-sync -->

## 追跡対象テーブル

`/pyxel-sync` と `.claude/hooks/check-source-comment.sh` がこの表を機械パースする。表の前後にある `pyxel-tracked-files` マーカーは削除しないこと。

「本家側」を主キーとし、1行 = 1本家ファイル。Pyxift 側で複数ファイルに分かれる場合は `<br>` 区切りで併記する。

「状態」列の値ドメイン:

- `実装済み`: Pyxift 側にコードがある（Pyxift 側列にパス必須）
- `予約:vX.Y.Z`: 指定タグまでに実装する（タグ前 hook が `実装済み` への遷移を検査）
- `保留`: 追跡のみ。実装判断は未定（hook 通過）
- `見送り`: 実装しない決定済み。セル内に `docs/decisions.md` か `docs/status.md` への参照リンク必須（hook 通過）

<!-- pyxel-tracked-files:start -->
| Pyxift 側 | 本家側 | 用途 | diff 提示 | 状態 |
| --- | --- | --- | --- | --- |
| （未実装） | python/pyxel/__init__.pyi | 公開 API 表面 | ○ | 保留 |
| Sources/CPyxiftCore/src/core/Palette.hpp<br>Sources/CPyxiftCore/src/core/Font.hpp | crates/pyxel-core/src/settings.rs | 数値定数（DEFAULT_COLORS / FONT_DATA / DEFAULT_TONE_*） | ○ | 実装済み |
| （未実装） | crates/pyxel-core/src/canvas.rs | 描画ロジック参考 | ○ | 保留 |
| THIRD_PARTY_LICENSES/pyxel-MIT.txt | LICENSE | 著作権年範囲・MIT 全文 | × | 実装済み |
<!-- pyxel-tracked-files:end -->

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
