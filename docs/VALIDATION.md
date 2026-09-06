# Validation matrix

Use the smallest checks that cover the changed files. A passing static check
only establishes the property it actually tests; it is not general runtime
proof.

## Common checks

- Inspect the focused diff and run `git diff --check`.
- Confirm that only intended files changed.
- Check references against the current checkout rather than a different mod
  branch or engine fork.
- For deployment, compare source and target SHA-256 hashes.

## File-type checks

| Changed content | Required static checks | Runtime limit |
| --- | --- | --- |
| Lua/script | Use a real Lua 5.1-compatible parser when available; inspect signal registration, lifecycle cleanup, and shared-writer ownership | Parsing does not prove callbacks fire or timing is correct |
| XML | Parse the exact file; for include fragments, validate the loaded/root context and check referenced IDs | XML validity does not prove the file is loaded or the UI/dialog is reachable |
| LTX/config | Resolve effective inheritance, section identity, and referenced sections; inspect the actual target configuration | Static fields do not prove native consumers use them |
| Windows-1251 UI/config | Verify decoding, BOM state, and line endings; retain literal Cyrillic | Byte correctness does not prove visual layout |
| Inventory/trade/corpse UI | Run the focused checks in `INVENTORY_UI.md`; count shared Lua call sites after signature changes | Each context and aspect ratio needs a separate visual test |
| Native hook/binary/animation | Verify target-build signature/layout guards and compatibility prerequisites before implementation | Source inspection cannot prove engine, model, skeleton, or motion behavior |
| Gameplay option | Run the existing targeted audit: `pwsh -NoProfile -File docs/validate_options_menu.ps1 -Quiet` | Persistence, cancel/reset behavior, and layout still need a game test |
| Generated catalog | Rebuild from canonical source when required and inspect the generated header/source gaps | A catalog is source-reference data, not proof of live behavior |

## Runtime evidence

For a runtime claim, record the target installation, source revision, changed
files, save/state prerequisites, exact steps, expected result, observed result,
relevant logs, and whether the result was accepted, pending, or failed. Use
`EVIDENCE_TEMPLATE.md` rather than adding undocumented numbers to a general
workflow file.

## Known limits

There is currently no unified automated runtime suite. Do not label a feature
“working” solely because parsing, static reference checks, deployment hashes,
or diagnostic output passed. Use the task-specific acceptance plan and report
uncovered paths explicitly.
