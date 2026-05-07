# Acknowledgments

## Pyxel

Pyxift は [Pyxel](https://github.com/kitao/pyxel) の API 互換を目指す独立実装です。
Pyxel は Takashi Kitao 氏によるレトロ2Dゲームエンジンで、MIT License のもとで公開されています。

- 本家リポジトリ: https://github.com/kitao/pyxel
- Copyright (c) 2018-2026 Takashi Kitao
- License: MIT（全文は [`THIRD_PARTY_LICENSES/pyxel-MIT.txt`](./THIRD_PARTY_LICENSES/pyxel-MIT.txt) を参照）

### Pyxift と Pyxel の関係

- Pyxift は Pyxel のソースコードを移植したものではなく、Swift / C++ による独立実装です
- 公開 API（関数名・引数の順序・戻り値）と数値仕様（解像度・色数・パレット・フォント等）は Pyxel に準拠します
- 一部のデータ（`FONT_DATA` / `DEFAULT_COLORS` / `DEFAULT_TONE_*`）は Pyxel から転記しており、該当ファイル冒頭に出典コメントを残しています
- Pyxel 本家との同期方針は [`docs/pyxel-reference.md`](./docs/pyxel-reference.md) を参照してください

Pyxel という素晴らしいエンジンと、その API 設計を公開してくださっている Takashi Kitao 氏に深く感謝します。
