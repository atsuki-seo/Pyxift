# Data and numeric values adopted from upstream Pyxel

Source: [kitao/pyxel](https://github.com/kitao/pyxel) (MIT License, Copyright (c) 2018-2026 Takashi Kitao)

Pyxift is also distributed under the MIT License, so these items are adopted with an attribution comment at the top of every source file containing them.
The full license text lives in [`THIRD_PARTY_LICENSES/pyxel-MIT.txt`](../THIRD_PARTY_LICENSES/pyxel-MIT.txt); the formal acknowledgment is in [`ACKNOWLEDGMENTS.md`](../ACKNOWLEDGMENTS.md).

<!-- pyxel-upstream-sync -->
## Upstream sync status

The tracking ledger maintained by the `/pyxel-sync` skill. Do not edit by hand; let the skill update it.

- **Source**: `../pyxel/` in the parent directory (a clone of kitao/pyxel). The `/pyxel-ref-update` skill force-syncs it to the latest stable release tag (full checkout, `--depth=1`) before each `/pyxel-sync` run
- **Tracked tag**: `v2.9.5`
- **Last synced**: 2026-05-09
- **Tracked files**: see the "Tracked-files table" below (SSOT)

For the diff-handling workflow, see `.claude/skills/pyxel-sync/SKILL.md`.
<!-- /pyxel-upstream-sync -->

## Tracked-files table

`/pyxel-sync` and `.claude/hooks/check-source-comment.sh` parse this table mechanically. Do not remove the surrounding `pyxel-tracked-files` markers.

The table acts as a **drift-detection filter** for `/pyxel-sync`: only paths listed here are diffed between the previously-recorded `Tracked tag` and the current upstream release tag. It does not drive sparse-checkout — `../pyxel/` is a full checkout managed by `/pyxel-ref-update`.

The "upstream" column is the primary key, so each row is one upstream file. When the Pyxift side spans multiple files, list them separated by `<br>`.

Values for the "Status" column:

- `implemented`: code exists on the Pyxift side (a path is required in the Pyxift column)
- `reserved:vX.Y.Z`: scheduled to be implemented by the given tag (the pre-release-tag hook checks the transition to `implemented`)
- `pending`: tracked only; the implementation decision is undecided (passes the hook)
- `declined`: decided not to implement. The cell must include a reference link to `docs/decisions.md` or `docs/status.md` (passes the hook)

<!-- pyxel-tracked-files:start -->
| Pyxift side | Upstream side | Purpose | Show diff | Status |
| --- | --- | --- | --- | --- |
| (not implemented) | python/pyxel/__init__.pyi | Public API surface | ○ | pending |
| Sources/CPyxiftCore/src/core/Palette.hpp<br>Sources/CPyxiftCore/src/core/Font.hpp<br>Sources/CPyxiftCore/src/core/Tone.hpp | crates/pyxel-core/src/settings.rs | Numeric constants (DEFAULT_COLORS / FONT_DATA / DEFAULT_TONE_*) | ○ | implemented |
| (not implemented) | crates/pyxel-core/src/canvas.rs | Reference for drawing logic | ○ | pending |
| Sources/CPyxiftCore/src/core/AssetBundle.cpp | crates/pyxel-core/src/resource_data.rs | Asset bundle JSON schema (images / tilemaps / sounds / musics) | ○ | implemented |
| Sources/CPyxiftCore/src/core/AssetBundle.cpp | crates/pyxel-core/src/utils.rs | compress_vec1 / compress_vec2 trailing-zero compaction | ○ | implemented |
| THIRD_PARTY_LICENSES/pyxel-MIT.txt | LICENSE | Copyright year range and full MIT text | × | implemented |
<!-- pyxel-tracked-files:end -->

## Default 16-color palette

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

(From Pyxel's `DEFAULT_COLORS`.)

## Constraint values

| Item | Value | Pyxel constant |
| --- | --- | --- |
| Default FPS | 30 | `DEFAULT_FPS` |
| Color count | 16 | `NUM_COLORS` |
| Image bank count | 3 | `NUM_IMAGES` |
| Image size | 256×256 | `IMAGE_SIZE` |
| Tilemap count | 8 | `NUM_TILEMAPS` |
| Tilemap size | 256×256 | `TILEMAP_SIZE` |
| Tile size | 8×8 | `TILE_SIZE` |
| Audio channel count | 4 | `NUM_CHANNELS` |
| Tone count | 4 (triangle / square / pulse / noise) | `NUM_TONES` |
| Sound count | 64 | `NUM_SOUNDS` |
| Music count | 8 | `NUM_MUSICS` |
| Audio sample rate | 22050 Hz | `AUDIO_SAMPLE_RATE` |
| Audio bit depth | 16 bit | `AUDIO_SAMPLE_BITS` |

## Built-in font

- Character range: ASCII 0x20–0x7F (96 glyphs)
- Glyph size: 4×6 pixels (`FONT_WIDTH=4`, `FONT_HEIGHT=6`)
- Data format: each glyph is a `u32` (24 bits used, bit-packed)
- Array: `FONT_DATA: [u32; 96]`

Implementation note: 4×6 = 24 bits are filled from the most significant bit, in row-major then column-major order (to be confirmed). Pyxift holds the same array as a Swift `[UInt32]` or a C++ `std::array<uint32_t,96>`, and unpacks bits at text-render time.

## Background color

`BACKGROUND_COLOR = 0x202224` (outside the palette, used for the window's letterbox area)

## Items not adopted

- Icon data (`ICON_DATA`) — Pyxift will ship its own icon
- Cursor data (`CURSOR_DATA`) — v0.1 uses the SDL3 standard cursor
- (Previously listed `DEFAULT_TONE_*`: adopted byte-for-byte in v0.2 — see `Sources/CPyxiftCore/src/core/Tone.hpp`.)
