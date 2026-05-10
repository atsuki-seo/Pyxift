# Design Decisions Summary

Captured during the `grill-me` session on 2026-05-07.
Additional notes were added during the upstream (kitao/pyxel) code cross-check session on 2026-05-07; see the trailing "Cross-check with upstream Pyxel" section.

## Language and distribution

- **User-facing language**: Swift only. C++ is an internal engine implementation, called from Swift through a pure-C header
- **Distribution format**: a single Swift Package Manager (SPM) package, self-contained. No CMake
- **Dependencies**: SDL3 (Homebrew on macOS, apt or a manual build on Linux). Referenced via a `SystemLibrary` target
- **License**: MIT
- **Publication**: GitHub Public, referenced directly via SPM Git URL. Swift Package Index registration happens after the v0.1.0 tag
- **Target OSes**: Linux and macOS as **development hosts** (where `swift build` / `swift test` are supported and CI runs the full matrix). Windows is supported as a **runtime / distribution target only** — games built via Pyxift's packaging CLI run natively on Windows, but Pyxift itself is not expected to build on a Windows development machine. iOS and Android are out of scope

## Distribution packaging

Decided during the `grill-me` session on 2026-05-10, scoping how a game written against Pyxift becomes something that end users on macOS, Windows, and Linux can install and run.

- **Packaging CLI**: Pyxift ships an executable `pyxift-package` (`swift run pyxift-package`) that, for the current development host's OS, produces a self-contained directory tree under `dist/<os>/<TargetName>/` containing the game executable, the SDL3 runtime, and any assets. The same CLI runs on each OS in CI to assemble all three OS trees.
  - **Local builds produce only the host OS's tree**. Cross-compilation is not supported. Developers who need all three OS trees push to GitHub and download the per-OS artifacts produced by CI.
  - **Convention over configuration**: no config file in v0.1. The executable name is taken from the `Package.swift` executable target name; assets are copied verbatim from a top-level `Assets/` directory in the user's package. Per-OS overrides (e.g. `--name`, `--os`) are exposed as CLI flags as needed.
  - **macOS**: emits a `.app` bundle with a templated `Info.plist`. App icon is taken from `Assets/Icon.icns` if present, otherwise a Pyxift default. The exact minimum set of `Info.plist` keys is to be settled at implementation time (tracked in `status.md` Open questions).
  - **Windows**: emits the `.exe` with `SDL3.dll` placed alongside it. Windows binaries are built natively on a `windows-latest` GitHub Actions runner — Linux→Windows cross-compilation is rejected as too brittle once C++ and SDL3 are in the mix.
  - **Linux**: emits the executable with `libSDL3.so` and a launcher shell script that sets `LD_LIBRARY_PATH` (or equivalent rpath wiring) so the bundled SDL3 is preferred over any system copy.

- **SDL3 acquisition for Windows**: CI fetches the official prebuilt VC bundle (`SDL3-devel-X.Y.Z-VC.zip`) from `libsdl-org/SDL` releases by URL+hash pin. Linux and macOS continue to use `SystemLibrary` against apt / Homebrew as before — only Windows uses the prebuilt bundle, since neither vcpkg nor MSYS2 is a clean fit for an SPM-only package.

- **Distribution level**: zip-distribution level only. The output of `pyxift-package` is intended to be zipped (or `tar.gz`'d on Linux) and uploaded to itch.io, GitHub Releases, a personal site, or Steam. **Code signing and notarization are out of scope** — the user supplies their own certificates if they need them. This means macOS direct-download distribution will trigger Gatekeeper and Windows direct-download will trigger SmartScreen; Steam-mediated installs bypass both.

- **Steam compatibility**: the `dist/<os>/` trees produced by `pyxift-package` are designed to be uploaded as-is via `steamcmd` depots — Pyxift does not need to know anything Steam-specific for this to work. **The Steamworks SDK (achievements, cloud saves, friends, etc.) is not bundled**: it is closed-source, requires a Valve distribution agreement, and its callback-style C++ API doesn't fit Pyxift's shape. If Steamworks integration becomes desirable, it will live in a separate package (`pyxift-steamworks` or similar), not in the core. Generation of Steamworks VDF templates is also out of scope for v0.1 (tracked under Future considerations).

- **HTML / WebAssembly distribution is not adopted.** Upstream Pyxel's `app2html` is its most popular distribution path, but Swift on WebAssembly plus an SDL3 WASM port is not yet practical, and the implementation cost dwarfs the benefit at this scale. This is a deliberate "won't build", not a deferred consideration.

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
