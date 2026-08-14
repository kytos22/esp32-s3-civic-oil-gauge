# Sprint 6 — Settings menu and warning presentation

- Scope: add the approved 700 ms settings surface, safe preference persistence,
  PSI/bar display, configurable warning presentation/audio, and corrected
  thermometer geometry without changing sensor calibration or enabling real inputs.
- Acceptance: AC-33 through AC-38.
- Status: physical-review correction complete in software — the first exact-board
  pass confirmed runtime/FPS and reboot persistence; revised warning/audio/menu
  behavior awaits a newly authorized flash

## Slices

| Slice | Status | Test point result | Notes |
|---|---|---|---|
| 6.1 Pure settings model | complete | red observed, then native 19/19 | Sanitization, units, warning presentation |
| 6.2 NVS and runtime audio controls | complete in software | complete firmware build | Save on menu close; defaults on error |
| 6.3 LVGL settings surface | hardware partially verified | 9/9 menu invariants + exact-board runtime | Reboot persistence confirmed; revised source/menu flow pending |
| 6.4 Thermometer and full-screen warning | hardware partially verified | 11/11 warning invariants + 24 consecutive 63–76 FPS windows | Pressure number never hidden in software; three-mode visual proof pending |
| 6.5 Editable simulator parity | complete locally | fragment/wrapper synchronization + Keel pass | Public Pages remains unchanged until an authorized push |
| 6.6 Physical-review correction | complete in software | red-first compile/0-of-12 contract; then native 21/21 + review 12/12 + full build | Exact-board no-tearing, no-puff, 0.5 Hz, and persistent-menu proof pending |

## Software evidence

- Red-first native link failure proved the new settings behavior was absent.
- Native Unity suite: 19/19 pass.
- Keel verifier: all checks pass, including 9/9 settings and 11/11 warning
  invariants.
- ESP-IDF 6.0.2 clean build from commit `65ebbfa`: 750,720-byte app,
  SHA-256 `f2de29c39b3cf7bdc4b06e24c85f98f02b55e17f05fb3fdeecc0743e1afd65fa`.
- Marcos authorized the exact-board demo test on 2026-08-14. App `65ebbfa` and all
  write regions passed esptool digest verification; bootloader, partition table, and
  application also passed post-boot verification. The ignored bounded capture
  `.artifacts/hardware/2026-08-14/sprint6-first-boot.typescript`, SHA-256
  `78391016db981bda1f6284d51b4b56260c8ccf5b9e86232fb01ad22d2a430da8`, records
  one completed warning tone and 24 consecutive 63–76 FPS windows with no runtime
  fault. No sensors, ADS1115, MTX-D, 12 V, or vehicle connection was used.
- Marcos then confirmed that settings persist after reboot and reported five
  physical-review issues. Red-first tests failed because `DataSource` and the
  independent full-screen phase did not exist; the new review contract failed 0/12.
  The correction makes `SENSORES` selectable/persistent while displaying only `--`
  and `SIN DATOS`, removes menu inactivity close, separates element 2 Hz from
  full-screen 0.5 Hz, adds `PELIGRO` / `PRESIÓN MUY BAJA`, toggles only a prebuilt
  overlay's visibility, and leaves the codec active at digital zero between tones.
- Corrected software evidence: native 21/21; settings 9/9; warning 11/11; physical
  review 12/12; clean ESP-IDF 6.0.2 app `da7cbfb`, 751,472 bytes, SHA-256
  `995a743bb3b3e3153381669bc88fefa8b209ca96c22e6481656ec0e2d9af40f9`.
  A new flash was deliberately not performed without explicit authorization.
