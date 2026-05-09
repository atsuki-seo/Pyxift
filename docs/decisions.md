# Design Decisions Summary

Captured during the `grill-me` session on 2026-05-07.
Additional notes were added during the upstream (kitao/pyxel) code cross-check session on 2026-05-07; see the trailing "Cross-check with upstream Pyxel" section.

## Language and distribution

- **User-facing language**: Swift only. C++ is an internal engine implementation, called from Swift through a pure-C header
- **Distribution format**: a single Swift Package Manager (SPM) package, self-contained. No CMake
- **Dependencies**: SDL3 (Homebrew on macOS, apt or a manual build on Linux). Referenced via a `SystemLibrary` target
- **License**: MIT
- **Publication**: GitHub Public, referenced directly via SPM Git URL. Swift Package Index registration happens after the v0.1.0 tag
- **Target OSes**: Linux and macOS (Windows, iOS, and Android are out of scope)

## Constraints (full Pyxel compliance)

All numeric values match Pyxel's defaults. The concrete values (resolution, color count, image bank count, tilemaps, FPS, audio channel count, etc.) live in the "Constraint values" table in `pyxel-reference.md`.

## API shape

- Static functions under the `Pyx` namespace (a caseless enum) — keeps function names short while avoiding collisions
- Buttons, colors, and similar values are exposed as type-safe enums (`Button.left`, `Color.red`, etc.) rather than raw `Int` literals
- The lifecycle is the `App` protocol (`mutating func update()` / `func draw()`) plus `Pyx.run(_:width:height:)`
- Error handling: `throws` is not used. Unrecoverable errors call `fatalError`; recoverable cases return Optional
- Game loop: fixed time step (30 FPS), at most two catch-up steps, drawing is VSync-synchronized

## Drawing primitives (Pyxel-compatible set)

`cls` / `pset` / `line` / `rect` / `rectb` / `circ` / `circb` / `tri` / `trib` / `blt` / `bltm` / `text` / `clip` / `pal` / `camera`

## Input

- Keyboard, mouse, and gamepad (via the SDL3 GameController subsystem)
- A virtual button abstraction (`.left/.right/.up/.down/.a/.b/.start`) unifies keys and pads
- Touch and accelerometer input are out of scope

## Audio (v0.2 and later)

- Custom software synthesis in the C++ core layer (square / triangle / pulse / noise)
- PCM is fed into an SDL3 `SDL_AudioStream`
- Songs are written in a short, MML-like text notation (similar to Pyxel's `sound.set("c3e3g3", ...)`)

## Asset format (v0.2 and later)

- A single bundle file `.pyxift` (a zip + JSON + PNG structure)
- One load API: `Pyx.load("mygame.pyxift")`
- v0.1 only ships a simple API for loading raw PNGs via `Pyx.loadImage("hero.png")`
- v0.4.0 promotes `Pyx.save(_:excludeImages:excludeTilemaps:excludeSounds:excludeMusics:)` to a public API. The earlier "withhold until external authoring tooling is decided" stance was overturned: round-trip parity with `Pyx.load` is the primary use case (e.g. runtime-generated bundles for tests, debugging, or save-state snapshots), and this is independent of any future authoring tool. A separate authoring tool, if added later, can be designed without rolling back this surface.

## Text

- A single built-in bitmap font (full spec in the "Built-in font" section of `pyxel-reference.md`)
- Japanese, custom fonts, and TTF are not supported in v0.1 / v0.2

## Hot reload

- Not supported. Re-run `swift run` to rebuild and relaunch

## Cross-check with upstream Pyxel

The upstream repository's location, key files, and out-of-scope directories are documented in the "Pyxel upstream repository reference" section of `CLAUDE.md` (the primary ledger). The cross-check is limited to two targets: `crates/pyxel-core/` (the engine itself) and `python/pyxel/__init__.pyi` (the public API surface).

### Engine implementation policy

- **Do not copy upstream code. Reimplement in C++ by referring to the spec.**
  - Rationale: upstream is Rust, while Pyxift is built on a C++ core. The dependencies (`glow` / `blip_buf` / `sdl2-sys`) also differ, and upstream bundles plenty of components (editor, screencast, tmx_parser, bgm_generator, shaders, etc.) that v0.1 does not need.
- **Numeric data is adopted byte-for-byte**: `DEFAULT_COLORS` / `FONT_DATA` / `DEFAULT_TONE_*` (v0.2) are mechanically converted from array literals into Swift / C++.
- **Rejected alternative — "use the Rust core directly via FFI"**: it would overturn the foundational decisions of a C++ core, single-package SPM distribution, and no CMake.

### Drawing-algorithm fidelity

- **Pixel-exact parity is not a goal**. Lines, circles, and triangles are implemented with textbook algorithms (Bresenham, midpoint circle, scanline).
- **Semantics follow upstream**: `clip` / `camera` / `pal`, the transparent color, and the source-rect handling and compositing order in `blt` are kept consistent with upstream.
- As a result, when porting an upstream sample game, a 1- or 2-pixel rasterization difference is acceptable.

### Coordinates, integer types, out-of-range access

- The drawing API takes **`Int`** (upstream takes `float` and floors internally). Pyxift does not provide fractional-coordinate APIs.
- **Off-screen coordinates are silently clipped**, and **negative sizes are no-ops**. This is consistent with the no-`throws` policy.
- A debug-build warn log is not implemented in v0.1 (avoiding premature optimization concerns).

### Input frame boundaries

- **The SDL3 event queue is drained once immediately before each `update` call**, with press/release events accumulated as bit flags during the frame.
- `button(_:)` returns the final state. `buttonPressed(_:)` / `buttonReleased(_:)` indicate whether the event occurred during this frame.
- If press → release → press happens within a single frame, both `Pressed` and `Released` are true and `button` reflects the final state (false → true → false yields a final `false`).
- This matches upstream Pyxel's edge-accumulation scheme. Under a fixed 30 FPS, prioritizing the final state is an acceptable trade-off against chatter.

### Public API surface vs. upstream

Functions present in upstream Pyxel that Pyxift renames:

- `Pyx.mouseCursor(visible: Bool)` ← upstream `mouse(visible:)` (renamed to avoid a function-name collision)

Editor-integration features will never be adopted.

Watch/reload-related environment-variable constants (`BASE_DIR`, `WINDOW_STATE_ENV`, `WATCH_STATE_FILE_ENV`, `WATCH_RESET_EXIT_CODE`) are not provided. Consistent with the "Hot reload — not supported" decision, the surrounding watch/reload mechanism that gives these constants meaning is out of scope.

### Math and RNG APIs: prefer Swift idioms over Pyxel's spec

The Pyxel public APIs `ceil` / `floor` / `clamp` / `sgn` / `sqrt` / `sin` / `cos` / `atan2` / `rseed` / `rndi` / `rndf` / `nseed` / `noise` are **not provided in Pyxift**.

Rationale: Swift already ships with Foundation's `sin` / `cos` / `sqrt`, `Int.random(in:)` / `Double.random(in:)`, `SystemRandomNumberGenerator`, etc. Adding thin Pyxel-compatible wrappers on top would damage the "Swift-native" feel. Perlin noise is the only thing missing from the standard library; if it becomes necessary we will revisit it as a separate package or in v0.1.x.

This is the explicit "Swift side" branch of the principle: "user-facing ergonomics follow Swift idioms; everything else follows Pyxel."

### Full coverage of key-code constants

Upstream defines 215 `KEY_*` constants. Pyxift's `enum Key` covers SDL3's `SDL_Keycode` (`SDLK_*`) one-to-one. Naming follows Swift conventions in lowerCamelCase, e.g. `case escape` / `case leftArrow`. Raw SDL3 keycode access remains available through `Key(rawValue:)`.

## Versioning policy

Pyxift versioning follows SemVer with one project-specific deviation about when `v1.0.0` is cut.

- **Minor (`v0.x+1.0` / `vX.y+1.0`)**: a milestone that adds Pyxel-compatible APIs or extends an existing one without breaking callers.
- **Patch (`vX.Y.z+1`)**: bug fixes, internal refactors, performance work, and documentation-only changes — no public API surface change.
- **Major (`vX+1.0.0`)**: cut on **either** of the following (OR condition), whichever comes first:
  1. A breaking change to Pyxift's public Swift API surface lands.
  2. A defined functional milestone is reached — e.g. "Pyxel API coverage is sufficient for the project's goals." Full 100% Pyxel API coverage is not expected to be realistic, so `v1.0.0` is gated on a functional checkpoint rather than coverage parity.

Implication for `v0.x`: a breaking change does not get folded into a minor bump to preserve `v0.y` semantics. If a breaking change is needed before the functional checkpoint, `v1.0.0` is cut early; the major number is not held back artificially.

The decision of which bump applies to a given milestone is made by `/next-milestone` (proposes a tentative version with each candidate) and confirmed by `/milestone-update` (records the chosen version onto the Roadmap) before the release-tagging `M<n>: vX.Y.Z ...` commit per `CLAUDE.md`'s Milestone Conventions.
