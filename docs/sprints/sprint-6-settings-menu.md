# Sprint 6 — Settings menu and warning presentation

- Scope: add the approved 700 ms settings surface, safe preference persistence,
  PSI/bar display, configurable warning presentation/audio, and corrected
  thermometer geometry without changing sensor calibration or enabling real inputs.
- Acceptance: AC-33 through AC-38.
- Status: hardware partially verified — exact-board flash/runtime/FPS pass; guided
  touch, visual, audio-volume, and reboot-persistence proof remains

## Slices

| Slice | Status | Test point result | Notes |
|---|---|---|---|
| 6.1 Pure settings model | complete | red observed, then native 19/19 | Sanitization, units, warning presentation |
| 6.2 NVS and runtime audio controls | complete in software | complete firmware build | Save on menu close; defaults on error |
| 6.3 LVGL settings surface | hardware partially verified | 9/9 menu invariants + exact-board runtime | Physical touch flow and reboot persistence pending |
| 6.4 Thermometer and full-screen warning | hardware partially verified | 11/11 warning invariants + 24 consecutive 63–76 FPS windows | Pressure number never hidden in software; three-mode visual proof pending |
| 6.5 Editable simulator parity | complete locally | fragment/wrapper synchronization + Keel pass | Public Pages remains unchanged until an authorized push |

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
