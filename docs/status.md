# Pyxift Status

Tracks open tasks and open questions. Completed items are visible via `git log --grep '^M[0-9]'`.

## Open tasks

- (deferred to a later v0.x) MML string mode (`Sound.mml(...)`) — was out of v0.2.0 scope
- (M10, target v0.7.0) Window/system APIs: `fullscreen`, `resize`, `screen_mode`, `integer_scale`, `icon`, `perf_monitor`, `screenshot`, `screencast`/`reset_screencast`, `Pyx.reset`, `show`, `flip`, and a public `VERSION` constant.
- (M13, target v0.8.0) Custom font support: `Font(filename:font_size:)`, `Font.text_width`, a `font` parameter on `Pyx.text`, and public `FONT_WIDTH`/`FONT_HEIGHT` constants.
- (M14, target v0.9.0) 3D blit: `blt3d` and `bltm3d`. Spec details (perspective parameters) to be captured by `/pyxel-research`.
- (M15, target TBD) Headless / test-injection hooks: `headless` startup option, `set_btn`, `set_btnv`, `set_input_text`, `set_dropped_files`, `user_data_dir`. Version not yet assigned — depends on whether Pyxift adopts an automated test-harness path.
- (M16, target v0.10.0) Bidirectional Pyxel API cross-check (beta) — see "Future considerations" for the original sketch.
- (milestone number TBD, target version TBD) Distribution packaging: `pyxift-package` CLI that emits `dist/<os>/` trees for macOS / Windows / Linux, Windows native CI build on `windows-latest`, SDL3 VC prebuilt fetch for Windows, `.app` template for macOS, and `LD_LIBRARY_PATH` launcher script for Linux. Scope and policy are settled in `decisions.md` → "Distribution packaging"; the milestone number and target version will be assigned via `/next-milestone` when this work is ranked against the other open milestones.

## Open questions

- noise (Perlin) API: not provided in v0.1; revisit when actually needed. See the math / RNG API section of `decisions.md` for details.
- Object-style resource APIs (`Image` / `Tilemap` as public types with per-instance `clip`/`camera`/`pal`/`pset`/`pget`/drawing methods, `Image.load`/`save`, `Tilemap.from_tmx`, `load_pal`/`save_pal`, `Tilemap.cls`/`collide`): adopting these would substantially rework the current fixed-bank surface (`Pyx.imagePset(bank:...)`, `Pyx.tilemapSetCell(tilemap:...)`). Decide whether Pyxift follows upstream's object API or keeps the fixed-bank model before scheduling this work as a milestone.
- Object-style audio APIs (`Sound` / `Music` / `Channel` / `Tone` as public types with mutable properties — `Sound.notes`/`tones`/`volumes`/`effects`/`speed`, `Channel.gain`/`detune`, etc. — plus `Sound.mml(_:)` string mode (already deferred), `play(sec:resume:)`, `playm(sec:)`, `Sound.pcm`, `Sound.save`, `Sound.total_sec`, `gen_bgm`): same trade-off as the resource-object question above. Resolve the API-shape decision before scheduling.
- macOS `Info.plist` minimum key set for `pyxift-package`'s `.app` template: which keys Pyxift fills in by default (`CFBundlePackageType`, `LSMinimumSystemVersion`, ...) and which the user must supply (`CFBundleIdentifier`, version strings, ...). Defer to implementation time when the `.app` can actually be launched and tested.

## Roadmap from v0.4 onward

- v0.6.0: drawing extensions — `blt`/`bltm` rotate/scale, `elli`/`ellib`, `fill`, `dither` (M9)
- v0.7.0: window/system — `fullscreen`, `resize`, `screen_mode`, `integer_scale`, `icon`, `perf_monitor`, `screenshot`, `screencast`, `reset_screencast`, `Pyx.reset`, `show`, `flip`, `VERSION` (M10)
- v0.8.0: custom font support — `Font(filename:font_size:)`, `Font.text_width`, `text(font:)`, expose `FONT_WIDTH`/`FONT_HEIGHT` (M13)
- v0.9.0: 3D blit — `blt3d`, `bltm3d` (M14)
- v0.10.0: bidirectional Pyxel API cross-check (beta) — extract Swift public-API signatures and mechanically compare against `__init__.pyi` (M16; see Future considerations)

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

### Steamworks VDF template generation

`pyxift-package` could grow a `--steam` flag that emits Steamworks VDF templates (the `appbuild` / `depot` files consumed by `steamcmd`) alongside the per-OS `dist/` trees. Deferred from the 2026-05-10 distribution-packaging design session because VDF is a few dozen lines of text that the Steamworks docs already template, and the value isn't clear until at least one Pyxift game is actually being uploaded to Steam. Revisit once such a use case shows up.
