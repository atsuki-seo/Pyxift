# Acknowledgments

## Pyxel

Pyxift is an independent implementation aiming for API compatibility with [Pyxel](https://github.com/kitao/pyxel).
Pyxel is a retro 2D game engine by Takashi Kitao, released under the MIT License.

- Upstream repository: https://github.com/kitao/pyxel
- Copyright (c) 2018-2026 Takashi Kitao
- License: MIT (full text in [`THIRD_PARTY_LICENSES/pyxel-MIT.txt`](./THIRD_PARTY_LICENSES/pyxel-MIT.txt))

### Relationship between Pyxift and Pyxel

- Pyxift is not a port of the Pyxel source code; it is an independent implementation in Swift and C++
- The public API (function names, argument order, return values) and numeric specifications (resolution, color count, palette, font, etc.) follow Pyxel
- A small amount of data (`FONT_DATA` / `DEFAULT_COLORS` / `DEFAULT_TONE_*`) is transcribed from Pyxel; the relevant files carry an attribution comment at the top
- For the upstream sync policy, see [`docs/pyxel-reference.md`](./docs/pyxel-reference.md)

We are deeply grateful to Takashi Kitao for creating such a wonderful engine and for openly publishing its API design.
