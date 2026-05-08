---
name: copyright-sync
description: Synchronize the copyright-year ranges of upstream Pyxel (Takashi Kitao) and Pyxift itself (atsuki.seo) against their respective SSOTs (the upstream LICENSE file and the current year). Invoked when the user runs `/copyright-sync`, or when delegated to from the pyxel-sync skill. Only the year notation is in scope — the license body is not synced.
allowed-tools: Bash(grep:*) Bash(sed:*) Bash(xargs:*) Bash(date:*) Bash(git add:*) Bash(git commit:*) Bash(git diff:*) Bash(git status:*)
---

# copyright-sync

Mechanically synchronize the copyright-year notations scattered throughout Pyxift against their respective SSOTs. Two separate tracks are handled:

- **Upstream (Takashi Kitao)** year range: SSOT is `../pyxel/LICENSE`.
- **Pyxift itself (atsuki.seo)** year range: SSOT is the current year (`date +%Y`); the start year is hardcoded to 2026.

The MIT license body is out of scope (the license is versionless and historical grants do not change, so no follow-up is required after the initial copy).

## Prerequisites

- This skill assumes that `../pyxel/` is already synced to the latest `main`. When delegated from `pyxel-sync`, this is taken care of in §1 of that skill. When `/copyright-sync` is invoked standalone, the caller is responsible.
- If detection finds nothing to change, exit with a single-line "no drift" report and do not create any commit.

## Behavior on Launch

### 1. Upstream (Takashi Kitao) year range

Extract the year range from the `Copyright (c) <range> Takashi Kitao` line in `../pyxel/LICENSE` and compare it against the following locations in Pyxift:

- `THIRD_PARTY_LICENSES/pyxel-MIT.txt`
- `.claude/hooks/check-source-comment.sh` (the template string)
- `CLAUDE.md` (the attribution-comment example)
- The opening attribution block of `docs/pyxel-reference.md`
- `ACKNOWLEDGMENTS.md`
- `README.md` (the upstream-acknowledgment paragraph)
- `LICENSE` (the upstream year range in the NOTICE section)
- Anywhere under `Sources/` (a `grep` for `Copyright (c) <range> Takashi Kitao` picks up only files carrying an attribution comment)

If drift is detected, rewrite mechanically and create a single dedicated commit:

```
chore(copyright-sync): bump Pyxel copyright notice from <old-range> to <new-range>
```

Detection example:

```bash
UPSTREAM_RANGE=$(grep -oP 'Copyright \(c\) \K[0-9]{4}-[0-9]{4}' ../pyxel/LICENSE | head -1)
LOCAL_RANGE=$(grep -oP 'Copyright \(c\) \K[0-9]{4}-[0-9]{4}' THIRD_PARTY_LICENSES/pyxel-MIT.txt | head -1)
if [ "$UPSTREAM_RANGE" != "$LOCAL_RANGE" ]; then
    grep -rlE "Copyright \(c\) $LOCAL_RANGE Takashi Kitao" \
        THIRD_PARTY_LICENSES/ ACKNOWLEDGMENTS.md CLAUDE.md README.md LICENSE \
        docs/pyxel-reference.md .claude/ Sources/ 2>/dev/null \
      | xargs -r sed -i "s|Copyright (c) $LOCAL_RANGE Takashi Kitao|Copyright (c) $UPSTREAM_RANGE Takashi Kitao|g"
fi
```

If there is no drift, do nothing and continue.

### 2. Pyxift itself (atsuki.seo) year range

Normalize the `Copyright (c) <year-or-range> atsuki.seo` line in `LICENSE` to the range `2026-<current_year>`, where the start year is 2026 and the end year is `date +%Y`. While the current year is still 2026, keep the single-year form (`Copyright (c) 2026 atsuki.seo`).

```bash
CURRENT_YEAR=$(date +%Y)
if [ "$CURRENT_YEAR" = "2026" ]; then
    DESIRED="Copyright (c) 2026 atsuki.seo"
else
    DESIRED="Copyright (c) 2026-$CURRENT_YEAR atsuki.seo"
fi

# Match either form currently present (single year or range) and rewrite to DESIRED.
sed -i -E "s|Copyright \(c\) 2026(-[0-9]{4})? atsuki\.seo|$DESIRED|g" LICENSE
```

If a rewrite happens, create a separate, dedicated commit independent of the upstream-year commit:

```
chore(copyright-sync): bump Pyxift copyright year from <old> to <new>
```

If there is no drift, do nothing and exit.

## Commit Granularity

§1 (upstream year) and §2 (Pyxift's own year) have independent motivations — the former tracks upstream, the latter tracks the passage of time for Pyxift itself. When drift is detected, **each gets its own dedicated commit**; do not mix them. If neither has drifted, exit without committing.

## Out of Scope (What This Skill Does NOT Do)

- Syncing the full MIT license body (the body of `THIRD_PARTY_LICENSES/pyxel-MIT.txt`).
- Adding or removing attribution comments (this is `.claude/hooks/check-source-comment.sh`'s responsibility).
- Updating the tracked-files table itself (this is `pyxel-sync`'s responsibility).

## Related Files

- Upstream-year SSOT: `../pyxel/LICENSE`
- Pyxift-year SSOT: the current year (`date +%Y`) plus a hardcoded start year of 2026 (hardcoded in this file)
- Caller: `.claude/skills/pyxel-sync/SKILL.md` §4
