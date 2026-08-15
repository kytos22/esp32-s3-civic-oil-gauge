# Sprint 6 — Settings menu and warning presentation

- Scope: add the approved 700 ms settings surface, safe preference persistence,
  PSI/bar display, configurable warning presentation/audio, and corrected
  thermometer geometry without changing sensor calibration or enabling real inputs.
- Acceptance: AC-33 through AC-38.
- Review-extension acceptance: AC-39 and AC-40.
- Second physical-review acceptance: AC-41 plus revised AC-32, AC-33, and AC-38.
- Status: review extension flashed and runtime-verified on the exact board;
  guided visual, touch, persistence, and audio-edge judgment remains

## Slices

| Slice | Status | Test point result | Notes |
|---|---|---|---|
| 6.1 Pure settings model | complete | red observed, then native 19/19 | Sanitization, units, warning presentation |
| 6.2 NVS and runtime audio controls | complete in software | complete firmware build | Save on menu close; defaults on error |
| 6.3 LVGL settings surface | hardware partially verified | 9/9 menu invariants + revised exact-board runtime | Reboot persistence was confirmed on the earlier image; revised source/menu touch flow pending |
| 6.4 Thermometer and full-screen warning | hardware partially verified | 11/11 warning invariants + revised exact-board 61–77 FPS | Pressure number never hidden in software; three-mode visual proof pending |
| 6.5 Editable simulator parity | complete locally | fragment/wrapper synchronization + Keel pass | Public Pages remains unchanged until an authorized push |
| 6.6 Physical-review correction | hardware partially verified | red-first compile/0-of-12 contract; then native 21/21 + review 12/12 + full build + exact-board runtime | Corrected image boots at 61–77 FPS; visual no-tearing, no-puff, 0.5 Hz, and persistent-menu proof pending |
| 6.7 Temperature units and BAR glyph | hardware partially verified | red observed, then native 22/22 + temperature-unit 7/7 + BAR-font 2/2 + full clean build + exact-board flash/digest | Persistent °C/°F presentation and U+002E numeric font are on the exact board; guided touch/render/persistence confirmation remains |
| 6.8 Resident menu, warning loop, full-frame QSPI buffers | software implemented; hardware pending | red native compile plus 4/9 contract; then native 22/22 + display/audio 9/9 | Menu freezes gauge rendering and stays open; double beep loops; two 480×480 PSRAM draw buffers replace 50-line bands; no verified TE pin |
| 6.9 Edge-triggered red overlay | correction implemented; hardware retest pending | exact-board 44–56 FPS/no-loop reproduction; red verifier 9/11 before fix | Full-screen visibility changes only on phase edges; settings hides it once |
| 6.10 Static-red render freeze | correction implemented; hardware retest pending | edge-gated exact-board loop pass but 45–57 FPS red windows; second red verifier 11/14 | Only the visible pressure value updates during opaque red; dynamic-gauge FPS excludes intentional static phases |

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
- The review extension first reproduced both absences: the native temperature-unit
  fixture failed to compile and the BAR-font contract found no U+002E. The completed
  slice keeps canonical Celsius for states and alarms, persists a display-only
  °C/°F enum, shows `<122` below the sensor's 50 °C floor, adds the same choice to
  the editable simulator, and regenerates the large numeric face with a real decimal
  point. Native 22/22, temperature-unit 7/7, BAR-font 2/2, all 40 acceptance rows,
  and a full clean ESP-IDF build pass. Clean app `8c2cc46` is 752,960 bytes with
  SHA-256 `6b854240f99d4edf92e8bdf0507ca83b26b0ad0fb69161d3146961c5d7543127`;
  exact-board visual confirmation remains pending.
- Marcos authorized the exact-board flash on 2026-08-15. USB identity and chip MAC
  matched the locally recorded exact display; all four write regions passed hash
  verification, and bootloader, partition table, and the complete application passed
  post-boot digest verification. Ignored capture
  `.artifacts/hardware/2026-08-15/sprint6-fahrenheit-bar-8c2cc46.typescript`, SHA-256
  `53d431227357bf5f3eaab1f5ac8467a6b08769f06919e73897877232ed40ea93`, records a
  clean demo boot and 19 consecutive 61–77 FPS windows with no runtime fault. No
  sensors, ADS1115, MTX-D, 12 V, or vehicle connection was used. Visual/touch/audio
  judgment is still required.
- Marcos then confirmed the prior checks and reported tearing on the red transition
  and menu. The second-review regression failed natively before start/stop loop
  commands existed and passed only 4/9 display/audio invariants. The implementation
  now keeps settings visible across warning, returns before all gauge-widget updates
  while settings is open, repeats the existing double beep while warning remains,
  and registers the CO5300 through the BSP's public primitives with two 480×480
  RGB565 PSRAM draw buffers. Native 22/22, display/audio 9/9, all 41 acceptance rows,
  and the complete ESP-IDF 6.0.2 build from clean implementation commit `1b28eac`
  pass. The 753,856-byte app SHA-256 is
  `56ea0cddcf76b7079619489cdaf2814899d0a328db346be7cabddb73a2304f4e`.
  No flash was authorized or performed; TE is not exposed by the verified BSP pin
  map, so tearing and continuous-audio FPS remain exact-hardware judgments.
