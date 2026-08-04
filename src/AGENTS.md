# Scope: firmware implementation in this directory

Sources of truth: `docs/03-technical-plan.md`, `docs/02-functional-spec.md`,
`docs/UI_DESIGN.md`, `docs/threat-model.md`, and the root `AGENTS.md`.

- Use namespace `oilgauge`; constants use `kPascalCase`, types PascalCase,
  and functions camelCase.
- Preserve explicit fault propagation. A sensor, ADC, calibration, or math
  fault must render as a fault, never as a retained or fabricated value.
- Keep `CONFIG_OIL_GAUGE_DEMO_MODE=y` until AC-13 through AC-18 carry real evidence.
- Implement the approved 480×480 50/50 UI faithfully; do not redesign it.
- Pressure warnings require an evidence-backed engine-running/RPM state.
- Before changing code, apply the matching change-map row and search
  `docs/api/INDEX.md` for reusable surfaces.
- Every behavioral change requires a regression test and updated affected
  acceptance criteria/documentation in the same slice.
- Do not add a dependency without a decision and license review.
- Never flash hardware or run vehicle tests without explicit user approval.
