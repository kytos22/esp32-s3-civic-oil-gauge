# Sprint 6 — Settings menu and warning presentation

- Scope: add the approved 700 ms settings surface, safe preference persistence,
  PSI/bar display, configurable warning presentation/audio, and corrected
  thermometer geometry without changing sensor calibration or enabling real inputs.
- Acceptance: AC-33 through AC-38.
- Status: planned — design resolved; implementation not yet evidenced

## Slices

| Slice | Status | Test point result | Notes |
|---|---|---|---|
| 6.1 Pure settings model | planned | red-first native tests required | Sanitization, units, warning presentation |
| 6.2 NVS and runtime audio controls | planned | native + firmware build required | Save on menu close; defaults on error |
| 6.3 LVGL settings surface | planned | renderer contract + firmware build required | 700 ms hold, timeout, interruption |
| 6.4 Thermometer and full-screen warning | planned | renderer contract + physical proof later | Pressure number never hidden |
| 6.5 Editable simulator parity | planned | source/standalone checks required | No deployed update without approval |
