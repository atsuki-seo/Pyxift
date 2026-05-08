# Pyxift Status

Tracks open tasks and open questions. Completed items are visible via `git log --grep '^M[0-9]'`.

## Open tasks

- v0.2.0 audio (in progress):
  - [x] M2a: synthesis core (Tone / Sound / AudioMixer / SDL_AudioStream adapter)
  - [x] M2b: Sound layer + Swift API (`Pyx.sound` / `Pyx.play` / `Pyx.stop` / `Pyx.isPlaying`) + tests + sample SE in PyxiftDemo
  - [x] M2c: Music API (`Pyx.music` / `Pyx.playMusic`) and `Pyx.playPos`
  - [ ] (deferred to a later v0.x) MML string mode (`Sound.mml(...)`) — out of v0.2.0 scope

## Open questions

- noise (Perlin) API: not provided in v0.1; revisit when actually needed. See the math / RNG API section of `decisions.md` for details.

## Roadmap from v0.2 onward

- v0.2.0: audio synthesis (4 channels: triangle / square / pulse / noise) — Sound + Music; MML mode is deferred
- v0.3.0: asset bundle in the `.pyxift` format

## Pre-release-tag checklist

Run before every `git tag v*.*.*` (version-independent, applies to all releases).
Tagging via Claude is mechanically blocked by `.claude/hooks/check-release-tag.sh`.

- [ ] `/pyxel-sync` has been run, and any diff against upstream Pyxel is resolved (or has been deliberately declined)
- [ ] The Tracked SHA in `docs/pyxel-reference.md` matches the `origin/main` HEAD of `../pyxel`
- [ ] `swift build -Xswiftc -warnings-as-errors` reports zero warnings

Release-specific extra items, when needed, go in a separate section within this file.

## Future considerations

### After the v0.1.0 tag: revisit a bidirectional Pyxel API check (beta)

During v0.1 development, Pyxift's API signatures are still in flux, so a bidirectional cross-check against the SSOTs (`docs/pyxel-reference.md` and `../pyxel/python/pyxel/__init__.pyi`) is not introduced. Once the v0.1.0 tag is cut and the API surface stabilizes, revisit the following:

- A script that extracts the Swift public-API signatures
- Mechanical cross-check against Pyxel's `__init__.pyi`
- Either integrate into `/pyxel-sync` or split into an independent hook
