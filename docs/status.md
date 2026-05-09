# Pyxift Status

Tracks open tasks and open questions. Completed items are visible via `git log --grep '^M[0-9]'`.

## Open tasks

- (deferred to a later v0.x) MML string mode (`Sound.mml(...)`) — was out of v0.2.0 scope
- (M7, target v0.4.0) Expose `Pyx.save(path:exclude*)` as a public Swift API. v0.3.0 already implements `pyxift_engine_save_bundle` in C++ for round-trip testing, but the Swift wrapper is intentionally withheld until external authoring tooling is decided.

## Open questions

- noise (Perlin) API: not provided in v0.1; revisit when actually needed. See the math / RNG API section of `decisions.md` for details.

## Roadmap from v0.4 onward

- v0.4.0: expose `Pyx.save(path:exclude*)` as public Swift API (see Open tasks)

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
