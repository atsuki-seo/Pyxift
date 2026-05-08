---
paths:
  - "**/*"
---

# Language Policy

All textual content tracked in this repository MUST be written in English. This includes, but is not limited to:

- Documentation: `README.md`, `ACKNOWLEDGMENTS.md`, files under `docs/`
- Source code comments and string literals intended as developer-facing documentation
- Configuration files and inline comments (`Package.swift`, CI workflows, `.gitignore`, hook scripts, settings JSON)
- Files under `.claude/` (this project's `CLAUDE.md`, skill definitions, rule documents, hook scripts)
- Commit messages

User-facing assistant responses are governed by the user's own global Claude Code configuration and are NOT covered by this policy. The assistant may still converse with the user in any language they prefer; only the artifacts committed to the repository are constrained.

## Exceptions

The following are allowed to contain non-English text:

- Third-party content under `THIRD_PARTY_LICENSES/` and vendored sources under `Sources/CPyxiftCore/vendor/` — preserve upstream wording verbatim.
- Proper nouns and attributions (author names, project names, URLs) regardless of script.
- Generated artifacts under `.build/`, `.git/`, and `.claude/tmp/` — not subject to review.

## Enforcement

`.claude/hooks/check-english-only.sh` performs a best-effort scan for CJK characters on `Edit`/`Write`/`MultiEdit` (the file being changed) and on `Stop` (changed-but-uncommitted files in the working tree). It emits warnings only — it never blocks an operation — so the human or assistant in the loop is responsible for acting on the warning.

For commit messages, no automated check is in place; rely on this rule and review the message before invoking `git commit`.
