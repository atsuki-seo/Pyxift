---
paths:
  - "**/*.swift"
  - "**/*.cpp"
  - "**/*.hpp"
  - "**/*.h"
  - "**/*.c"
  - "**/*.sh"
---

# Comment Style

Only write a comment when **both** of the following hold:

1. There is an **external constraint or non-obvious premise** at play (an external-spec quirk, a hardware constraint, library behavior, a compatibility requirement — something that cannot be inferred from the code itself).
2. To accommodate that constraint, the code adopts an **implementation that would not normally be the obvious choice**.

"Self-explanatory from the code," "expressible by types or identifiers," "explanation of the spec," "narration of what a test or sample is doing" — none of these warrant comments. The form is free, but both elements above must be present in the comment text.

## Examples of What NOT to Write

- "How" explanations ("convert X to Y", "sum with a loop")
- Role descriptions for types or functions ("X represents Y", "this file handles Z")
- Section headers/dividers for enums or function groups ("// controls", "// arrows")
- Task references ("added in M1", "issue #123", "changed in PR")
- File-header summaries (already conveyed by the filename, location, and includes)

If you find existing comments of these kinds, feel free to delete them as part of an unrelated edit (provided the change does not alter behavior).

## Examples of What to Write

```cpp
// SDL3 keycodes have a wide, non-contiguous value range, so we use a set rather than an array.
std::unordered_set<std::uint32_t> keys_held;
```

```cpp
// Match upstream behavior: drop alpha by forcing 3-channel RGB load.
stbi_load(..., &w, &h, &n, 3);
```

In both cases, the external constraint (SDL3 value range / upstream-compatibility requirement) and the unusual choice (using a set / discarding alpha) appear together in the comment.

## Exception

Attribution comments on derivative code that requires license attribution are exempt from this rule (see "Upstream-Derived Code — Attribution Comment Required" in CLAUDE.md).
