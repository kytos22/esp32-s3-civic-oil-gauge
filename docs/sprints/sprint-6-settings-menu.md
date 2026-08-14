# Sprint 6 — Settings menu and warning presentation

- Scope: add the approved 700 ms settings surface, safe preference persistence,
  PSI/bar display, configurable warning presentation/audio, and corrected
  thermometer geometry without changing sensor calibration or enabling real inputs.
- Acceptance: AC-33 through AC-38.
- Status: software complete — exact-board touch/visual/FPS proof awaits explicit
  flash authorization

## Slices

| Slice | Status | Test point result | Notes |
|---|---|---|---|
| 6.1 Pure settings model | complete | red observed, then native 19/19 | Sanitization, units, warning presentation |
| 6.2 NVS and runtime audio controls | complete in software | complete firmware build | Save on menu close; defaults on error |
| 6.3 LVGL settings surface | software complete | 9/9 menu invariants + complete build | Physical touch flow pending |
| 6.4 Thermometer and full-screen warning | software complete | 11/11 warning invariants + complete build | Pressure number never hidden; physical proof pending |
| 6.5 Editable simulator parity | complete locally | fragment/wrapper synchronization + Keel pass | Public Pages remains unchanged until an authorized push |

## Software evidence

- Red-first native link failure proved the new settings behavior was absent.
- Native Unity suite: 19/19 pass.
- Keel verifier: all checks pass, including 9/9 settings and 11/11 warning
  invariants.
- ESP-IDF 6.0.2 clean build from commit `65ebbfa`: 750,720-byte app,
  SHA-256 `f2de29c39b3cf7bdc4b06e24c85f98f02b55e17f05fb3fdeecc0743e1afd65fa`.
- No flash or vehicle action was performed.
