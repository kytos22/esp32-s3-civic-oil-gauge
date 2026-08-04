# Scope: header and configuration code in this directory

Sources of truth: `docs/03-technical-plan.md`, `docs/02-functional-spec.md`,
`docs/threat-model.md`, and the root `AGENTS.md`.

- Use namespace `oilgauge`; constants use `kPascalCase`, types PascalCase,
  and functions camelCase.
- Preserve the explicit `Fault`/`ConvertedValue` error strategy. Never turn
  an invalid value into a plausible pressure or temperature.
- Keep calibration invalid by default. No sensor curve or pin function may
  be added without stored measurement evidence and a decision entry.
- Before adding a reusable surface, search `docs/api/INDEX.md`. Update the
  index, tests, and documentation in the same change.
- Apply the change-map row in `docs/03-technical-plan.md` before editing.
- Public surfaces require English documentation comments; non-obvious
  safety decisions require a why comment.
- Never log secrets, raw private data, or claim an unverified measurement.
- Update Keel state at the moment a decision or project position changes.
