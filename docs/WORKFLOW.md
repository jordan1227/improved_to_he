# General modding workflow

This document is the operational contract for work in this repository. It
keeps source discovery, implementation, deployment, and runtime evidence
separate so a successful static check is not mistaken for a successful game
feature.

## Authority and scope

- Canonical editable source: `C:\Games\NLC_OGSR_HE files\improved_to_he`.
- Runnable test installation: `C:\Games\NLC_OGSR_HE`.
- Loose files in the runnable installation's `gamedata` override packed files.
- Default investigation scope is `gamedata/scripts` and `gamedata/config`.
- Documentation-only, analysis-only, and setup work never deploys or modifies
  the runnable installation.

## Task modes

| Mode | Repository edits | Deployment | Manual test | Commit |
| --- | ---: | ---: | ---: | ---: |
| `AUDIT` / `DESIGN` | No | No | No | No |
| `IMPLEMENT` | Yes, after scope is clear | No | No | No |
| `IMPLEMENT_AND_TEST` | Yes | Only when explicitly requested | Yes | No |
| `RELEASE` | Yes | Explicitly requested | Acceptance complete | Explicitly requested |

Cross-module, ambiguous, asset, animation, native-hook, or binary work needs a
design or compatibility report before implementation. A localized, obvious
edit may use `IMPLEMENT` directly when the affected owner and validation are
already established.

## Discovery sequence

1. Confirm the task mode, source root, test root, target build, and requested
   scope.
2. Read `AGENTS.md`, `docs/README.md`, and the relevant entries in
   `PROJECT_MAP.md`, `BASELINE.md`, and `TESTING.md`.
3. Read the task-specific document before changing a shared system.
4. Search narrowly using exact identifiers and bounded paths. Search
   `CLAUDE_MODDING_NOTES.md` selectively; do not load it or generated catalogs
   by default.
5. Trace the actual loader, owner, call path, lifecycle, invalidation path,
   and competing writers before editing.
6. Separate confirmed facts, likely problems, hypotheses, proposals, and
   deliberately excluded work.

## Implementation gates

Before editing, identify:

- the exact files and subsystem owners;
- the existing pattern to preserve;
- the lifecycle and failure cleanup paths;
- the encoding/line-ending requirements;
- the static validators and manual acceptance steps;
- the files explicitly allowed for later deployment.

Do not infer target-build behavior from Anomaly, GAMMA, or another OGSR fork.
Do not modify binary spawn data from string extraction. Native hooks require
build-specific guards, diagnostics, and a safe fallback when the expected
signature or layout is absent.

## Validation and deployment gates

1. Review the changed-file diff.
2. Run `git diff --check` and the relevant checks in `VALIDATION.md`.
3. For legacy runtime files, preserve Windows-1251/no-BOM state and existing
   line endings.
4. If testing was requested, deploy only the explicitly relevant changed
   runtime files to the runnable installation.
5. Report the exact deployed paths and compare source/target SHA-256 hashes.
6. Perform the focused manual test matrix in the target installation.
7. Record expected versus observed behavior using `EVIDENCE_TEMPLATE.md`.

Deployment hashes prove file identity only. They do not prove engine behavior,
visual alignment, AI perception, animation compatibility, or timing.

## Completion report

Every completed task should report:

- scope and assumptions;
- exact files and subsystem owners;
- static source findings;
- tool-generated validation;
- changed-file summary and diff status;
- deployed files and hashes, if any;
- manual runtime results, if any;
- hypotheses, blocked prerequisites, residual risks, and deferred work.

Do not commit unless the user explicitly requests it.
