# Pyxift Status

Tracks open tasks and open questions. Completed items are visible via `git log --grep '^M[0-9]'`.

## Open tasks

- (deferred to a later v0.x) MML string mode (`Sound.mml(...)`) — was out of v0.2.0 scope
- (deferred past v0.7.0) `screencast` / `reset_screencast`, `Pyx.reset`, `flip`, and shader-based `screen_mode` filters (smooth / retro). M10 ships an API stub for `screen_mode` via `SDL_SetTextureScaleMode` (NEAREST / LINEAR) since the SDL_Renderer backend has no GLSL pipeline; the upstream-equivalent CRT/blur shaders are out of scope. See "Future considerations".
- (M11, target v1.0.0) Object-style resource and audio API surface — promote `Image` / `Tilemap` (and accompanying `Sound` / `Music` / `Channel` / `Tone`) to public types with per-instance methods, replacing the current fixed-bank API (`Pyx.imagePset(bank:...)`, `Pyx.tilemapSetCell(tilemap:...)`, etc.). This is a breaking change to Pyxift's public Swift surface and per `decisions.md` → "Versioning policy" cuts v1.0.0. Resolves the two Open questions about resource/audio object APIs that previously sat in this file.
- (M12, target v1.0.0) Distribution packaging: `pyxift-package` CLI that emits `dist/<os>/` trees for macOS / Windows / Linux, Windows native CI build on `windows-latest`, SDL3 VC prebuilt fetch for Windows, `.app` template for macOS, and `LD_LIBRARY_PATH` launcher script for Linux. Scope and policy are settled in `decisions.md` → "Distribution packaging". Paired with M11 as the second v1.0.0 driver — together they form the "minimum-viable game development and cross-OS distribution" checkpoint that anchors v1.0.0.
- (M13, target v1.1.0) Custom font support: `Font(filename:font_size:)`, `Font.text_width`, a `font` parameter on `Pyx.text`, and public `FONT_WIDTH`/`FONT_HEIGHT` constants. Deferred past v1.0.0 because the built-in bitmap font is sufficient for minimum-viable game development.
- (M14, target v1.2.0) 3D blit: `blt3d` and `bltm3d`. Spec details (perspective parameters) to be captured by `/pyxel-research`. Deferred past v1.0.0 as a niche extension.
- (M15, target TBD) Headless / test-injection hooks: `headless` startup option, `set_btn`, `set_btnv`, `set_input_text`, `set_dropped_files`, `user_data_dir`. Version not yet assigned — depends on whether Pyxift adopts an automated test-harness path. Deferred past v1.0.0 because this targets Pyxift's own testing rather than user-facing game development.
- (M16, target TBD, v1.x) Bidirectional Pyxel API cross-check (beta) — see "Future considerations" for the original sketch. Deferred past v1.0.0 as an internal maintenance tool.

## Open questions

- noise (Perlin) API: not provided in v0.1; revisit when actually needed. See the math / RNG API section of `decisions.md` for details.
- macOS `Info.plist` minimum key set for `pyxift-package`'s `.app` template: which keys Pyxift fills in by default (`CFBundlePackageType`, `LSMinimumSystemVersion`, ...) and which the user must supply (`CFBundleIdentifier`, version strings, ...). Defer to implementation time when the `.app` can actually be launched and tested.

## Roadmap from v0.7 onward

- v1.0.0: object-style resource/audio API surface (M11) and distribution packaging (M12) — the minimum-viable game development and cross-OS distribution checkpoint. M11 is the breaking change that triggers v1.0.0 per the versioning policy; M12 is the functional checkpoint that justifies cutting v1.0.0 here rather than later.
- v1.1.0: custom font support — `Font(filename:font_size:)`, `Font.text_width`, `text(font:)`, expose `FONT_WIDTH`/`FONT_HEIGHT` (M13)
- v1.2.0: 3D blit — `blt3d`, `bltm3d` (M14)
- v1.x (target TBD): bidirectional Pyxel API cross-check (beta) — extract Swift public-API signatures and mechanically compare against `__init__.pyi` (M16; see Future considerations)

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
