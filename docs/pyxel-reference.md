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
| Sources/CPyxiftCore/src/core/Canvas.cpp | crates/pyxel-core/src/canvas.rs | Reference for drawing logic; DITHERING_MATRIX and ELLIPSE_ROUNDING_BIAS adopted byte-for-byte | ○ | implemented |
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

## Drawing extensions (M9 reference)

Upstream specs for the drawing extensions shipped in Pyxift M9 (v0.6.0). Facts below are extracted from `crates/pyxel-core/src/canvas.rs` and `crates/pyxel-core/src/image.rs` at the synced tag.

### `blt` / `bltm` `rotate` / `scale`

[confirmed] `rotate` is in degrees, clockwise positive — the implementation flips the sin sign (`sin = -sin(rad)`).

[confirmed] `scale < f32::EPSILON` (effectively `<= 0`) is a no-op. Negative scale is not used for flips; horizontal/vertical flips are expressed by negative `w`/`h`.

[confirmed] Pivot is the source-rect center, computed as `(half_width, half_height) = ((w - 1) / 2.0, (h - 1) / 2.0)`. For odd-sized rects the pivot lands on a half-pixel.

[confirmed] Sampling is nearest-neighbor via inverse mapping: each output pixel computes `(sx, sy) = src_center + (Δx · cos/scale - Δy · sin/scale, Δx · sin/scale + Δy · cos/scale)` and rounds to the nearest source pixel; the source-rect bounds are checked before reading.

[confirmed] `bltm(rotate, scale)` rasterizes the requested tile region into a same-size scratch image, then runs the standard `blit_with_transform` path on it.

### `elli(x, y, w, h, col)` / `ellib(x, y, w, h, col)`

[confirmed] `(x, y)` is the bounding-box top-left corner; `cx = x + (w-1)/2.0`, `cy = y + (h-1)/2.0`.

[confirmed] Rasterized via direct evaluation of the ellipse equation `dy = rb · sqrt(1 - dx²/ra²)` (and the symmetric `dx` form). Upstream uses a two-pass scan (vertical strips for the left/right halves plus horizontal scans for the top/bottom halves) so corner pixels are not lost; Pyxift mirrors this two-pass structure.

[confirmed] An `ELLIPSE_ROUNDING_BIAS = 0.01` is added before rounding so half-pixel ties bias outward and the resulting ellipse stays symmetric.

[confirmed] `ellib` reuses the same scan but only writes the four extremal pixels of each strip (the equivalent of the four ellipse-arc endpoints at that scan position).

### `fill(x, y, col)`

[confirmed] Scanline span flood fill, 4-neighbor, stack-based (no recursion). Reads the seed color at `(x, y)`, returns immediately if the seed equals the new color, then fills runs along each row and seeds the row above and below at each gap-end.

[confirmed] Run extents are clamped to the active clip rectangle.

[confirmed] When `dither` is active (`alpha < 1.0`), the row write goes through the same Bayer mask as other primitives, so unwritten pixels remain at the seed color and the algorithm still terminates (the next-row probe still sees the seed color and stops).

### `dither(alpha)`

[confirmed] Adopted byte-for-byte: 4×4 Bayer matrix `DITHERING_MATRIX[y][x]` with the threshold values `{1, 9, 3, 11, 13, 5, 15, 7, 3, 11, 1, 9, 15, 7, 13, 5} / 16` in row-major order.

[confirmed] Write predicate is strict greater-than: `should_write(x, y) = alpha > matrix[y mod 4][x mod 4]`. `alpha >= 1.0` is a fast-path that bypasses the Bayer test entirely; `alpha <= 0.0` writes nothing.

[confirmed] Applies to every drawing primitive (`pset`/`line`/`rect`/`circ`/`tri`/`elli`/`fill`/`blt`/`bltm`) except `cls`, which always overwrites the entire framebuffer.

Source: crates/pyxel-core/src/canvas.rs @ v2.9.5
Source: crates/pyxel-core/src/image.rs @ v2.9.5
Source: python/pyxel/__init__.pyi @ v2.9.5

_Researched against kitao/pyxel v2.9.5 (2026-05-03)._

## Input extensions (M8 reference)

Upstream specs for the input APIs scheduled for Pyxift M8 (v0.5.0). All facts below are extracted from kitao/pyxel at the synced tag (see "Upstream sync status" above). Pyxift's own naming/typing decisions for these APIs are made separately in `docs/decisions.md`.

### `btnp(key, hold=0, repeat=0) -> bool`

[confirmed] Signature: `def btnp(key: int, hold: int = 0, repeat: int = 0) -> bool` (`python/pyxel/__init__.pyi`).

[confirmed] Frame-edge semantics (`crates/pyxel-core/src/input.rs::is_button_pressed`):

- Returns `true` on the frame the key transitions to pressed.
- If `repeat == 0`, no repeat logic — only the initial press frame returns `true`.
- If `repeat > 0`, after the key has been held for `hold` frames, returns `true` every `repeat` frames. Concretely: with `frame_count` = current frame and `pressed_frame` = the frame the press was recorded, the API returns `true` when `(frame_count - (pressed_frame + hold)) >= 0 && (frame_count - (pressed_frame + hold)) % repeat == 0`.
- If the key was pressed-and-released within the same frame, only that frame returns `true`; subsequent repeat ticks do not fire.
- `hold` and `repeat` are `u32` (Rust) / `int` (Python). Negative values are not part of the API.

[confirmed] Asserts (panics) when called with an analog key (`MOUSE_POS_*`, `MOUSE_WHEEL_*`, gamepad axes).

### `btnv(key) -> int`

[confirmed] Signature: `def btnv(key: int) -> int`. Returns the analog value (`KeyValue = i32`) for the given key, or `0` if no value has been recorded (`crates/pyxel-core/src/input.rs::button_value`).

[confirmed] Asserts (panics) when called with a non-analog key.

[confirmed] Analog keys (`is_analog_key`):

- `MOUSE_POS_X`, `MOUSE_POS_Y` — current mouse position in screen-pixel coordinates (after window-scale conversion).
- `MOUSE_WHEEL_X`, `MOUSE_WHEEL_Y` — wheel deltas for the current frame; reset to `0` at the start of each input frame.
- Gamepad axes: keys with `key >= GAMEPAD_KEY_START_INDEX` and `(key % GAMEPAD_KEY_STRIDE) < GAMEPAD_AXIS_COUNT`. With `GAMEPAD_KEY_START_INDEX = 0x5000_0200`, `GAMEPAD_KEY_STRIDE = 0x100`, `GAMEPAD_AXIS_COUNT = 6`, the six axes per gamepad are: `LEFTX(+0)`, `LEFTY(+1)`, `RIGHTX(+2)`, `RIGHTY(+3)`, `TRIGGERLEFT(+4)`, `TRIGGERRIGHT(+5)`. Up to 4 gamepads (`GAMEPAD1..GAMEPAD4`).

[confirmed] Source of axis values: SDL2 `SDL_CONTROLLERAXISMOTION.value` (`i32`, range `-32768..=32767`) is forwarded verbatim; trigger axes use the SDL convention `0..=32767`.

### `set_mouse_pos(x, y) -> None`

[confirmed] Signature: `def set_mouse_pos(x: float, y: float) -> None`. The pyi docstring describes it as "Mainly for headless mode input simulation", but in non-headless mode the implementation also warps the OS cursor (`crates/pyxel-core/src/input.rs::set_mouse_position`).

[confirmed] Behavior:

- `x`, `y` are first floored to `i32` (`f32_to_i32`).
- Updates `pyxel.mouse_x` / `pyxel.mouse_y` (the public state) and the `MOUSE_POS_X` / `MOUSE_POS_Y` analog key values.
- If not in headless mode, calls `platform::set_mouse_pos(x * screen_scale + screen_x, y * screen_scale + screen_y)` to warp the OS cursor.

### `input_text: str` and `input_keys: list[int]`

[confirmed] Module-level mutable state, reset at the start of each input frame (`start_input_frame` clears both, plus `dropped_files`).

[confirmed] `input_text` is appended to whenever an `Event::TextInput { text }` is processed in `system.rs`. Upstream uses SDL2's `SDL_TEXTINPUT` event (`poll_events.rs::SDL_TEXTINPUT`), which delivers UTF-8 text from the OS IME / keyboard layer.

[confirmed] Text input is **always on** in upstream Pyxel — there is no public `start_text_input` / `stop_text_input` switch. SDL2 enables text input by default unless explicitly stopped. (Pyxift's SDL3 backend will need to call `SDL_StartTextInput(window)` once at init, since SDL3 changed the default to off.)

[confirmed] `input_keys` is a `list[int]` (`Vec<Key>` in Rust) of the *non-mouse, non-analog* keys that were pressed during this frame. Populated by `press_key` only when `key < MOUSE_KEY_START_INDEX`. Cleared at the start of each input frame.

[confirmed] `set_input_text(text)` clears `input_text` then appends the supplied text. Used for headless-mode injection.

### `dropped_files: list[str]`

[confirmed] Module-level `Vec<String>`, populated when an `Event::FileDropped { filename }` arrives (from SDL2 `SDL_DROPFILE` in `poll_events.rs`). Cleared at the start of each input frame.

[confirmed] On drop, upstream also raises (`SDL_RaiseWindow`) the window before recording the filename.

[confirmed] `set_dropped_files(files)` clears the list then appends each path. Used for headless-mode injection. (M8 ships only the read side per `docs/status.md` — `set_dropped_files` is a separate M15 task.)

### Frame timing and lifecycle (consolidated)

[confirmed] In upstream, `start_input_frame` runs once at the start of every frame, before the SDL event queue is processed:

1. Reset `MOUSE_WHEEL_X` / `MOUSE_WHEEL_Y` analog values to `0` and `pyxel.mouse_wheel` to `0`.
2. Clear `pyxel.input_keys`, `pyxel.input_text`, `pyxel.dropped_files`.

This matches Pyxift's existing "drain SDL events once before each `update`" model documented in `docs/decisions.md` (Input frame boundaries). The new `input_text` / `input_keys` / `dropped_files` slots can hang off the same edge.

### M8a / M8b split candidate

[inferred] A natural split based on the data above:

- **M8a (core input semantics)**: `btnp(hold:repeat:)`, `btnv` (analog axes for mouse pos / wheel / gamepad). Touches `InputState` and `Pyx.button*` only; no new SDL3 subsystems.
- **M8b (event-driven input)**: `set_mouse_pos`, `input_text` / `input_keys`, `dropped_files`. Requires SDL3 `SDL_StartTextInput`, drop-file event handling, and an OS-cursor warp call. Naturally bundled because all three depend on event/IO plumbing in the platform/adapter layer.

[todo] Decide Pyxift's Swift naming for these (e.g. `Pyx.buttonValue(_:)` vs `Pyx.btnv(_:)`, whether `Button` enum gains analog cases). Captured for `/milestone-update`.

[todo] Decide whether SDL3 text input should be enabled at startup (matching upstream's "always on") or gated behind a Swift API. Affects whether the Pyxift surface includes a `Pyx.startTextInput()` / `Pyx.stopTextInput()` pair.

Source: crates/pyxel-core/src/input.rs @ v2.9.5
Source: crates/pyxel-core/src/pyxel.rs @ v2.9.5
Source: crates/pyxel-core/src/platform/key.rs @ v2.9.5
Source: crates/pyxel-core/src/platform/sdl2/poll_events.rs @ v2.9.5
Source: crates/pyxel-core/src/system.rs @ v2.9.5
Source: python/pyxel/__init__.pyi @ v2.9.5

_Researched against kitao/pyxel v2.9.5 (2026-05-03) via /pyxel-research._

## Object-style resource API (M11 reference)

Upstream specs for the public resource and audio classes that Pyxift M11 (target v1.0.0) lifts from index-based access to object-style access. Facts below are extracted from `python/pyxel/__init__.pyi` and `crates/pyxel-binding/src/*_wrapper.rs` plus the corresponding `crates/pyxel-core/src/*.rs` files at the synced tag.

### Module-level instance lists

[confirmed] Six module-level lists are exposed via Python's `__getattr__`: `pyxel.images`, `pyxel.tilemaps`, `pyxel.channels`, `pyxel.tones`, `pyxel.sounds`, `pyxel.musics`. Each is a length-fixed sequence of `Rc<UnsafeCell<T>>` slots populated at `pyxel.init` (`crates/pyxel-binding/src/variable_wrapper.rs`, `crates/pyxel-core/src/pyxel.rs::init_*`).

[confirmed] Slot counts match the constants in the "Constraint values" table above: 3 images, 8 tilemaps, 4 channels, 4 tones, 64 sounds, 8 musics.

[confirmed] Slot assignment is supported (`pyxel.images[0] = new_image`): the binding's `wrap_ptr_vec_as_python_object_sequence!` macro replaces the underlying `Rc` in the vector. This is reference-swap semantics, not a deep copy — any object holding a reference to the previous slot keeps the previous instance live.

[confirmed] In addition, `pyxel.screen`, `pyxel.cursor`, `pyxel.font` are individual `Image` references (not lists). They are assignment targets only via the global state, not via subscription.

### Storage model

[confirmed] Each resource is stored in core as `Rc<UnsafeCell<T>>` (alias `RcImage` / `RcTilemap` / `RcSound` / `RcMusic` / `RcChannel` / `RcTone`; defined via `define_rc_type!` in `crates/pyxel-core/src/utils.rs`). The wrapper holds the same `Rc` — there is no copy-on-set, so multiple Python references to the same slot mutate the same underlying instance.

[inferred] On the Pyxift side this maps cleanly to a Swift `class` (reference type) per resource. Reference identity is the upstream-equivalent property — two `Image` references obtained from the same `Pyx.images[0]` slot must refer to the same buffer, just like upstream.

### `Image`

[confirmed] Properties: `width: u32` (getter), `height: u32` (getter). Both are read-only.

[confirmed] Constructors:
- `Image(width, height)` — fresh empty image of the given size.
- `Image.from_image(filename: str, include_colors: bool = False) -> Image` — load from a PNG/GIF/JPEG. `include_colors=True` overwrites the global palette with the file's palette.
- A `incl_colors=` alias is accepted but warns "deprecated. Use include_colors instead."

[confirmed] Data operations:
- `data_ptr() -> ctypes.c_uint8 array` — exposes the raw `width*height` byte buffer to Python. Pyxift's analogue would be a `withUnsafeMutableBufferPointer` style accessor.
- `set(x, y, data: list[str])` — bulk-write hex digits ("0123" etc.).
- `load(x, y, filename, include_colors=False)` — load file at offset.
- `save(filename, scale)` — write a scaled PNG.

[confirmed] Canvas operations (per-image, identical to module-level `pyxel.*`): `clip` (0 or 4 args), `camera` (0 or 2 args), `pal` (0 or 2 args), `dither(alpha)`, `cls(col)`, `pget(x,y)`, `pset(x,y,col)`, `line`, `rect`, `rectb`, `circ`, `circb`, `elli`, `ellib`, `tri`, `trib`, `fill`, `blt`, `bltm`, `text`. The same `Image.blt(...)` and `Image.bltm(...)` accept `int | Image` / `int | Tilemap` for the source argument (polymorphic).

[confirmed] `blt3d` / `bltm3d` are also methods on `Image` — these belong to M14 and remain out of scope for M11.

### `Tilemap`

[confirmed] Properties: `width: u32`, `height: u32` (read-only), `imgsrc: int | Image` (read-write). Setting `imgsrc` accepts either an image-bank index or an `Image` instance and is stored as `pyxel::ImageSource::{Index(u32), Image(RcImage)}` (`crates/pyxel-core/src/tilemap.rs`).

[confirmed] Constructors:
- `Tilemap(width, height, img: int | Image)` — fresh empty tilemap.
- `Tilemap.from_tmx(filename, layer) -> Tilemap` — load from a TMX (Tiled) file. Out of M11 scope; tracking only.

[confirmed] Data operations: `data_ptr() -> ctypes.c_uint16 array` (4 bytes per tile: `image_tx`, `image_ty`), `set(x, y, data: list[str])` (4-digit hex per tile), `load(x, y, filename, layer)` for TMX import.

[confirmed] Canvas operations are tile-valued analogues of `Image`'s pixel-valued ones: `clip`, `camera`, `cls(tile)`, `pget(x,y) -> (tx, ty)`, `pset(x,y,tile)`, `line`, `rect`, `rectb`, `circ`, `circb`, `elli`, `ellib`, `tri`, `trib`, `fill` — each takes a `tile: tuple[int, int]` instead of `col: int`. Plus `blt(x, y, tm: int | Tilemap, u, v, w, h, tilekey=None, rotate=0, scale=1)` for tile-domain blits.

[confirmed] `Tilemap.collide(x, y, w, h, dx, dy, walls: list[(int, int)]) -> (dx, dy)` — pixel-space movement clamped against tile-coordinate walls. New since M9; relevance to M11 is "include in the public class" but the implementation can ride on existing tilemap data.

### `Channel`

[confirmed] Properties: `gain: f32` (default 0.125), `detune: i32` cents (default 0). Both read-write.

[confirmed] Constructor: `Channel()` — fresh channel.

[confirmed] Methods:
- `play(snd, sec=0, loop=False, resume=False)` — `snd` is a sum type: `int` (sound index), `list[int]`, `Sound`, `list[Sound]`, or `str` (MML). Index-form looks up `pyxel.sounds[idx]`. The deprecated `tick=` alias is converted to `sec = tick / 120`.
- `stop()` — stop playback.
- `play_pos() -> (sound_index, sec) | None`.

[confirmed] All audio-mutating calls take `pyxel::AudioLock::lock()` for the duration of the call. The Pyxift-side mixer mutex must wrap the equivalent operations.

### `Tone`

[confirmed] Properties: `mode: u32` (0=Wavetable, 1=ShortPeriodNoise, 2=LongPeriodNoise), `sample_bits: u32` (default 4), `wavetable: list[int]` (each value in `0..(2^sample_bits)`), `gain: f32` (default 1.0). All read-write. `wavetable` is exposed via a sequence wrapper that supports both element access and bulk replace.

[confirmed] Constructor: `Tone()` — fresh tone with defaults.

[confirmed] Deprecated aliases (warn-only, kept for source compatibility): `noise` ↔ `mode`, `waveform` ↔ `wavetable`. Pyxift can omit these per the existing "do not adopt deprecated upstream names" pattern; flag for explicit decision in `decisions.md`.

### `Sound`

[confirmed] Properties: `notes: list[u32]` (range 0–59, rests = -1; element type is `pyxel::SoundNote`), `tones: list[u32]` (`SoundTone`), `volumes: list[u32]` (0–7, `SoundVolume`), `effects: list[u32]` (0–5, `SoundEffect`), `speed: u32` (`SoundSpeed`, larger = slower; 120 ⇒ 1 sec/note). All four list properties are exposed via the same sequence-wrapper macro as `Tone.wavetable` — element access, slice access, and full replacement all work.

[confirmed] Constructor: `Sound()` — fresh empty sound.

[confirmed] Methods:
- `set(notes: str, tones: str, volumes: str, effects: str, speed: int)` — bulk parse from short strings (e.g. `"g2b-2d3"`, `"ttss"`, `"7777 7531"`, `"nfnf"`).
- `set_notes(s)`, `set_tones(s)`, `set_volumes(s)`, `set_effects(s)` — individual setters with the same string grammar.
- `mml(code: str | None = None)` — switch to MML mode; `mml(None)` clears MML mode and returns to the parametric mode. Old MML syntax (`x` / `X` / `~` characters) is auto-detected and routed to a deprecated parser with a warning.
- `pcm(filename: str | None = None)` — load a WAV/OGG file as PCM playback; `pcm(None)` clears PCM mode.
- `save(filename, sec, ffmpeg=False)` — render to WAV (and optionally MP4 via FFmpeg).
- `total_sec() -> f32 | None` — playback duration; `None` means infinite-loop.

### `Music`

[confirmed] Property: `seqs: list[list[u32]]` — a 2-D list, outer = channels, inner = sound indices. Exposed via a hand-written `Seqs` wrapper (`music_wrapper.rs`) that yields `Seq` element wrappers per channel; supports slice get/set and 2-D mutation.

[confirmed] Constructor: `Music()` — fresh empty music.

[confirmed] Methods:
- `set(*seqs: list[int])` — variadic; one list per channel. Empty list = unused channel.
- `save(filename, sec, ffmpeg=False)` — same shape as `Sound.save`.

[confirmed] Deprecated alias: `snds_list` (warn-only, redirects to `seqs`).

### `Font` (out of M11 scope)

[confirmed] `Font(filename, font_size=10.0)` and `Font.text_width(s)`. Already deferred to M13 in `status.md`; included here only so the reader sees the full set of resource classes upstream exposes.

### Mapping considerations for Pyxift's Swift API

[decided] The six module-level lists are exposed as dedicated wrapper types `ImageBank` / `TilemapBank` / `ChannelBank` / `ToneBank` / `SoundBank` / `MusicBank` (Collection-conforming, fixed-length, `subscript(Int) { get set }`). Resource types are Swift `class` (reference type). See `decisions.md` → "Object-style resource API (M11)".

[decided] Slot replacement (`Pyx.images[i] = newImage` etc.) is supported in v1.0.0 with reference-swap semantics. The pre-M11 `Pyx.imagePset(bank:...)`, `Pyx.tilemapSetCell(tilemap:...)`, and `Pyx.tilemapSetImageBank(tilemap:...)` shorthands are removed by M11.

[decided] Of the three single-`Image` references upstream exposes, only `Pyx.screen` is public on Pyxift's surface (settable for draw-target switching). `Pyx.cursor` and `Pyx.font` are not exposed.

[decided] `Tilemap.imgsrc` is renamed to `imageSource` on the Pyxift surface, backed by `enum TilemapImageSource { case bank(Int), image(Image) }`.

[decided] `Sound.mml(_:)` is adopted in M11c (new MML grammar only; no legacy auto-detect). `Sound.pcm(_:)` is deferred to M13 (target v1.2.0).

[decided] `Sound.save` and `Music.save` are deferred to M13 (target v1.2.0). `Tilemap.collide` is adopted in M11b; `Tilemap.from_tmx` is deferred to M13.

[decided] Deprecated upstream aliases (`Tone.noise`, `Tone.waveform`, `Music.snds_list`, `Image.from_image(incl_colors=)`) are not surfaced. See `decisions.md` → "Public API surface vs. upstream".

Source: python/pyxel/__init__.pyi @ v2.9.5
Source: crates/pyxel-binding/src/{image,tilemap,channel,tone,sound,music,variable}_wrapper.rs @ v2.9.5
Source: crates/pyxel-core/src/{image,tilemap,channel,tone,sound,music,pyxel,utils}.rs @ v2.9.5

_Researched against kitao/pyxel v2.9.5 (2026-05-03) via /pyxel-research._

## Items not adopted

- Icon data (`ICON_DATA`) — Pyxift will ship its own icon
- Cursor data (`CURSOR_DATA`) — v0.1 uses the SDL3 standard cursor
- (Previously listed `DEFAULT_TONE_*`: adopted byte-for-byte in v0.2 — see `Sources/CPyxiftCore/src/core/Tone.hpp`.)
