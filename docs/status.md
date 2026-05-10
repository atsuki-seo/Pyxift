# Pyxift Status

Tracks open tasks and open questions. Completed items are visible via `git log --grep '^M[0-9]'`.

## Open tasks

- (deferred past v0.7.0) `screencast` / `reset_screencast`, `Pyx.reset`, `flip`, and shader-based `screen_mode` filters (smooth / retro). M10 ships an API stub for `screen_mode` via `SDL_SetTextureScaleMode` (NEAREST / LINEAR) since the SDL_Renderer backend has no GLSL pipeline; the upstream-equivalent CRT/blur shaders are out of scope. See "Future considerations".
- (M11a, intermediate, stays in v0.7.x) Image class promotion — promote `Image` to a public Swift class with constructors (`Image(width:height:)`, `Image.fromImage(filename:includeColors:)`), per-instance canvas methods (clip / camera / pal / dither / cls / pget / pset / line / rect / rectb / circ / circb / elli / ellib / tri / trib / fill / blt / bltm / text), `data_ptr`-equivalent accessor, and bank slot replacement via `Pyx.images[i] = newImage`. Removes the pre-M11 shorthand `Pyx.imagePset(bank:...)`. Splits `pyxift_c.h` into per-subsystem headers at the start of this milestone. See `decisions.md` → "Object-style resource API (M11)".
- (M11b, intermediate, stays in v0.7.x) Tilemap class promotion — promote `Tilemap` to a public Swift class with constructors, `imageSource: TilemapImageSource` property (renamed from upstream `imgsrc`), per-instance tile-domain canvas methods, `Tilemap.collide(...)`, and bank slot replacement. Removes the pre-M11 shorthands `Pyx.tilemapSetCell(tilemap:...)` and `Pyx.tilemapSetImageBank(tilemap:...)`.
- (M11c, target v1.0.0) Audio class promotion — promote `Channel`, `Tone`, `Sound`, `Music` to public Swift classes with the upstream method shapes, the four bank wrappers (`ChannelBank`, `ToneBank`, `SoundBank`, `MusicBank`), `Sound.mml(_:)` (new grammar only — no legacy `x` / `X` / `~` auto-detect), and `Pyx.screen` exposure as a settable draw target. This is the breaking-change milestone that triggers `v1.0.0` per `decisions.md` → "Versioning policy".
- (M12, target v1.1.0) Distribution packaging: `pyxift-package` CLI that emits `dist/<os>/` trees for macOS / Windows / Linux, Windows native CI build on `windows-latest`, SDL3 VC prebuilt fetch for Windows, `.app` template for macOS, and `LD_LIBRARY_PATH` launcher script for Linux. Scope and policy are settled in `decisions.md` → "Distribution packaging". Originally framed as a v1.0.0 co-driver alongside M11; shifted to v1.1.0 because M11c's breaking change cuts v1.0.0 first per the versioning policy.
- (M13, target v1.2.0) Resource I/O extensions — bundles the items deferred from M11: `Sound.pcm(_:)` (WAV/OGG playback), `Sound.save` and `Music.save` (WAV / FFmpeg-MP4 export), and `Tilemap.from_tmx(...)` (TMX/XML import). Grouped because they all add external-dependency surface (decoder libraries, FFmpeg invocation, XML parsing) that does not fit the v1.0.0 "minimum-viable" scope.
- (M14, target v1.3.0) Custom font support: `Font(filename:font_size:)`, `Font.text_width`, a `font` parameter on `Pyx.text`, and public `FONT_WIDTH`/`FONT_HEIGHT` constants. Deferred past v1.0.0 because the built-in bitmap font is sufficient for minimum-viable game development.
- (M15, target v1.4.0) 3D blit: `blt3d` and `bltm3d`. Spec details (perspective parameters) to be captured by `/pyxel-research`. Deferred past v1.0.0 as a niche extension.
- (M16, target TBD) Headless / test-injection hooks: `headless` startup option, `set_btn`, `set_btnv`, `set_input_text`, `set_dropped_files`, `user_data_dir`. Version not yet assigned — depends on whether Pyxift adopts an automated test-harness path. Deferred past v1.0.0 because this targets Pyxift's own testing rather than user-facing game development.
- (M17, target TBD, v1.x) Bidirectional Pyxel API cross-check (beta) — see "Future considerations" for the original sketch. Deferred past v1.0.0 as an internal maintenance tool.

## Open questions

- noise (Perlin) API: not provided in v0.1; revisit when actually needed. See the math / RNG API section of `decisions.md` for details.
- macOS `Info.plist` minimum key set for `pyxift-package`'s `.app` template: which keys Pyxift fills in by default (`CFBundlePackageType`, `LSMinimumSystemVersion`, ...) and which the user must supply (`CFBundleIdentifier`, version strings, ...). Defer to implementation time when the `.app` can actually be launched and tested.

## Roadmap from v0.7 onward

- v1.0.0: object-style resource and audio API surface — `Image`, `Tilemap`, `Channel`, `Tone`, `Sound`, `Music` promoted to public Swift classes; the pre-M11 index-based shorthands are removed (M11a → M11b → M11c, with M11a/b as intermediate milestones and M11c cutting the tag). Cut here per the breaking-change branch of `decisions.md` → "Versioning policy"; the original "M11 + M12 = minimum-viable game development and cross-OS distribution" framing is overturned.
- v1.1.0: distribution packaging — `pyxift-package` CLI emitting per-OS `dist/<os>/` trees (M12)
- v1.2.0: resource I/O extensions — `Sound.pcm`, `Sound.save`, `Music.save`, `Tilemap.from_tmx` (M13)
- v1.3.0: custom font support — `Font(filename:font_size:)`, `Font.text_width`, `text(font:)`, expose `FONT_WIDTH`/`FONT_HEIGHT` (M14)
- v1.4.0: 3D blit — `blt3d`, `bltm3d` (M15)
- v1.x (target TBD): bidirectional Pyxel API cross-check (beta) — extract Swift public-API signatures and mechanically compare against `__init__.pyi` (M17; see Future considerations)

## Pre-release-tag checklist

Tagging is automated by `.github/workflows/release-tag.yml`: pushing a release-tagging
M-commit (subject matching `^M[0-9]+[a-z]?: v[0-9]+\.[0-9]+\.[0-9]+ `) to `main` runs
`swift build -Xswiftc -warnings-as-errors` and `swift test` on macOS+Linux, and on
success the workflow creates and pushes the `vX.Y.Z` tag. Do not run `git tag` by hand.

Items that are **not** mechanically enforced and are the author's responsibility before
pushing the release-tagging M-commit:

- [ ] `/pyxel-sync` has been run, and any diff against upstream Pyxel is resolved (or has been deliberately declined)
- [ ] The Tracked tag in `docs/pyxel-reference.md` reflects that run
- [ ] `docs/status.md` Open tasks / Roadmap reflects what this release ships and what defers

Release-specific extra items, when needed, go in a separate section within this file.

## Future considerations

### After the v0.1.0 tag: revisit a bidirectional Pyxel API check (beta)

During v0.1 development, Pyxift's API signatures are still in flux, so a bidirectional cross-check against the SSOTs (`docs/pyxel-reference.md` and `../pyxel/python/pyxel/__init__.pyi`) is not introduced. Once the v0.1.0 tag is cut and the API surface stabilizes, revisit the following:

- A script that extracts the Swift public-API signatures
- Mechanical cross-check against Pyxel's `__init__.pyi`
- Either integrate into `/pyxel-sync` or split into an independent hook

### M10 deferred surfaces: screencast, Pyx.reset, flip, shader-based screen_mode

M10 (v0.7.0) ships `fullscreen`, `resize`, `screen_mode` (NEAREST / LINEAR stub), `integer_scale`, `icon`, `perf_monitor`, `screenshot`, `show`, and the public `VERSION` constant. The following items from the original M10 scope were intentionally deferred:

- **`screencast` / `reset_screencast`**: GIF-encoder integration (frame ring buffer, palette-aware GIF writer). Decoupled from the screenshot path because the encoder pulls in a separate dependency surface.
- **`Pyx.reset`**: a fresh-engine reset path. Touches every subsystem (canvas / audio / input / window) and warrants its own design pass once the v1.0.0 object-style API (M11) is settled, because reset semantics differ between the global API and per-instance resources.
- **`flip`**: synchronous one-shot draw + present (advance one frame from outside the run loop). Requires re-entrancy into the engine loop; deferred until a use case forces the design.
- **Shader-based `screen_mode` filters (smooth / retro)**: the upstream Pyxel GLSL pipeline. Pyxift's SDL_Renderer backend cannot run shaders today; revisit if/when the renderer migrates to a GPU-shader path.

### Steamworks VDF template generation

`pyxift-package` could grow a `--steam` flag that emits Steamworks VDF templates (the `appbuild` / `depot` files consumed by `steamcmd`) alongside the per-OS `dist/` trees. Deferred from the 2026-05-10 distribution-packaging design session because VDF is a few dozen lines of text that the Steamworks docs already template, and the value isn't clear until at least one Pyxift game is actually being uploaded to Steam. Revisit once such a use case shows up.
