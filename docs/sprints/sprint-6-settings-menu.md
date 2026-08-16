# Sprint 6 — Settings menu and warning presentation

- Scope: add the approved 700 ms settings surface, safe preference persistence,
  PSI/bar display, configurable warning presentation/audio, and corrected
  thermometer geometry without changing sensor calibration or enabling real inputs.
- Acceptance: AC-33 through AC-38.
- Review-extension acceptance: AC-39 and AC-40.
- Second physical-review acceptance: AC-41 plus revised AC-32, AC-33, and AC-38.
- Status: D-057 exact-board transport is stable, but completed cadence is only
  about 26.5–29.0 FPS dynamically; physical visual judgment remains pending

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
| 6.8 Resident menu, warning loop, full-frame QSPI buffers | runtime partially verified | red native compile plus 4/9 contract; then native 22/22 + final display/audio 14/14 | Menu freezes gauge rendering and stays open; double beep loops; two 480×480 PSRAM draw buffers replace 50-line bands; no verified TE pin |
| 6.9 Edge-triggered red overlay | runtime verified; judgment pending | exact-board 44–56 FPS/no-loop reproduction; red verifier 9/11 before fix; six later loop pairs | Full-screen visibility changes only on phase edges; settings hides it once |
| 6.10 Static-red render freeze | runtime verified; judgment pending | edge-gated 45–57 FPS reproduction; second red verifier 11/14; final 44 dynamic windows at 63–76 FPS | Only the visible pressure value updates during opaque red; dynamic-gauge FPS excludes intentional static phases |
| 6.11 50 Hz performance experiment | software complete; hardware comparison pending | red 13 ms/60 FPS profile failed the new expectation; then native 23/23 + complete ESP-IDF 6.0.2 build | The 754,192-byte candidate targets 20 ms/50 FPS and keeps physical QSPI at 40 MHz because ESP32-S3 GPSPI cannot generate 50 MHz from its 80 MHz APB source |
| 6.12 80 MHz QSPI comparison | runtime verified; visual judgment pending | contract 6/6, native 23/23, full build, exact-board flash/digests; 27 windows at 48–50 FPS, eight matched warning phases, no lock/reset failure in 71.5 s | Force-include a project-owned QSPI macro override into the pinned Waveshare BSP without editing managed sources; retain 20 ms/50 FPS |
| 6.13 Official CO5300 GPIO-TE path | exact combined candidate failed; causes partly isolated | red at 20 ms and 12/18; then native 23/23, display/audio 23/23, full ESP-IDF 6.0.2 build, exact-board flash/digests | GPIO43 TE is usable at 59.483 Hz. `MADCTL 0xA0 -> 0x60` explains the 180-degree inversion; FULL/single-buffer serialization explains about 14.85 FPS; the returned diagonal still needs a one-variable orientation A/B |
| 6.14 `MADCTL=0xA0` isolation A/B | exact-board isolation complete; visual failure confirmed | red display/audio contract 21/24; then green 24/24, native 23/23, clean full build, exact-MAC flash/hash verification, bounded runtime and Marcos's visual judgment | `0xA0` restores correct orientation but the diagonal persists. It does not follow the `0x60` address-direction change; FULL/single-buffer `TE_SYNC` remains serialized at normally 14.82–14.85 FPS |
| 6.15 Native-scan immutable presenter | exact-board transport failed | red source contract 15/33 and missing slot-policy compile; then contract 34/34, native 26/26 and clean 734,816-byte ESP-IDF 6.0.2 app at `50dee93`; flash hashes passed but first direct DMA transfer underflowed | Native `MADCTL=0x00`, LVGL PARTIAL 270-degree tiled rotation and immutable ownership were retained; its direct-PSRAM-DMA transport is rejected by D-055 |
| 6.16 Bounded QSPI staging | exact-board transport and tearing judgment verified | red missing-profile compile plus 6/13 contract; then native 27/27, QSPI 13/13, display/audio 34/34, clean 734,896-byte app `aa38f5f`, exact-board write hashes, 30-second zero-error runtime and Marcos's visual judgment | Preserve D-054 scan order and ownership; internal 8-row bounce chunks remove the DMA underflow, with about 17–35 completed FPS under observed demo load; Marcos confirmed no tearing or diagonal, while the 180-degree logical orientation is accepted for physical mounting |
| 6.17 Native logical orientation | exact-board accepted | display contract red 31/35, then 35/35; native 27/27, clean 733,232-byte app `9b59722`, exact-board hashes, 30-second zero-error capture and Marcos's visual/touch judgment | USB-C-right orientation and touch align; no tearing/diagonal; menu feels smoother than D-055 but remains low-FPS. Composition falls to about 0.3–1.1 ms average, while 16–21 ms full snapshot copies keep completed presentation near 17–33 FPS |
| 6.18 Zero-snapshot full-frame presenter and bar cleanup | exact-board transport verified; cadence failed; visual judgment pending | display contract red 34/46, then 46/46; native 27/27; clean app `9febd47`; exact identity and four write hashes; 35-second boot/runtime capture | Two complete `RGB565_SWAPPED` PSRAM buffers in `FULL` mode are released only after DMA completion. Exact board reports TE 59.554 Hz, DMA 13.2–14.6 ms and zero timeout/error/fatal counters, but only about 26.5–29.0 dynamic FPS and about 16 FPS in the lower-activity demo interval. Fixed notes are removed, dynamic states remain, and both bars are 15 px; physical tearing/color/menu judgment pending |

## Software evidence

- D-055 source evidence: the exact D-054 boot produced ESP-IDF's documented
  `DMA TX underflow detected`; the first color transaction surfaced
  `ESP_ERR_INVALID_STATE`, and all later two-second windows stayed at
  `presented=0.000 fps`. The correction first failed to compile because the new
  transfer-row/queue-depth profile did not exist and its expanded contract passed
  only 6/13. It now passes native 27/27, the bounded-QSPI contract 13/13 and the
  display/audio contract 34/34. ESP LCD keeps the snapshots immutable but copies
  each one through at most three queued 7,680-byte internal DMA buffers; direct
  PSRAM DMA is disabled while QSPI remains 80 MHz. Any start error or DMA timeout
  latches a persistent fatal flag and ends the presenter. Clean commit `aa38f5f`
  produces a 734,896-byte ESP-IDF 6.0.2 app with SHA-256
  `92392058e67e0dde440f805f159e98c60754dca4c83164ddf87aa03dc3d6065a`.
  On the separately authorized exact board all four writes passed hashes and app
  `aa38f5f` booted with TE at 59.438 Hz. The bounded 30-second capture reports
  about 17–35 completed FPS, 13.0–13.5 ms DMA and
  `timeouts=0 errors=0 no_slot=0 fatal=0`; visual judgment remains pending.
- D-054 source evidence: official guidance and exact source tracing replace the
  rejected `TE_SYNC` architecture rather than moving its phase again. The first
  source-contract run passed only 15/33, and the first valid native red failed on
  the deliberately absent `frame_slot_policy.h`. The implementation now passes
  34/34 display/audio invariants and 26/26 native tests. It keeps the panel in
  native scan order, rotates dirty LVGL areas in cache-local 32x32 tiles, snapshots
  only at the last flush, selects only the newest complete generation at TE, and
  cannot select an IN_FLIGHT DMA buffer for rendering. Both snapshots were aligned,
  verified external-DMA-capable and sent with `psram_dma_direct`. Clean commit
  `50dee93` produces a 734,816-byte app with SHA-256
  `d02ba8f1a9a5cb819a7fa63b6d05c6eae859a842a18e2d543b365aeb5b6fabc1`.
  On the separately authorized exact board all four write-time hashes passed and
  app `50dee93` booted, but direct PSRAM DMA immediately underflowed and no frame
  completed. D-054 is rejected as a transport implementation before visual review.
- D-053 source evidence: the revised contract first failed 21/24 while D-051's
  panel orientation calls and log remained. The candidate now makes no post-init
  `esp_lcd_panel_swap_xy()` or `esp_lcd_panel_mirror()` call, reports that it is
  preserving Waveshare `MADCTL=0xA0`, and keeps the rest of the D-051 presentation
  configuration unchanged. The display/audio contract passes 24/24 and the native
  suite passes 23/23. A clean ESP-IDF 6.0.2 build at `dbdc856` produced a
  755,744-byte app with SHA-256
  `7b4ae20345c537cd7a329bc5649153babe43ca1e40bac9a0dcbec471f0e960ce`.
  The separately authorized exact-board MAC matched before flash and esptool
  verified every written region. The bounded boot/runtime capture confirms `MADCTL=0xA0`, GPIO43
  TE at 59.403 Hz and no panic, watchdog or unexpected reset. Normal dynamic
  windows deliver 14.82–14.85 FPS with approximately 29–31 ms non-nested drawing
  plus 32–36 ms synchronous flush. Marcos confirmed correct orientation and a
  persistent diagonal. D-053 therefore isolates the inversion from the presentation
  defect but is not an acceptable display architecture.
- D-051 red-first evidence: the native suite reached the display-profile assertion
  and failed with `Expected 15 Was 20`; the source contract passed 12/18 because
  the official hardware-orientation and adapter `TE_SYNC` route was not implemented
  yet. The initial sandboxed PlatformIO attempt failed for its known external-cache
  permission reason and is not counted as the behavioral red.
- D-051 green evidence: native 23/23 and all Keel checks pass, including the
  expanded 23/23 display/audio contract. A complete ESP-IDF 6.0.2 rebuild confirms
  the effective 15 ms LVGL period, demo mode, CO5300 2.1.0, adapter 0.6.3, and LVGL
  9.5.0. The 756,048-byte app SHA-256 is
  `a5f81c11f43b8a5bb9cc22d6f27717c2dabe5bf59c6a989fb75ff13d9fba2b23`.
  The exact board received a fresh build from the same commit on 2026-08-16:
  756,048 bytes, SHA-256
  `a5463ce6b787f97543049231986abfb36f2c020064fab3a3ba54d852c220f192`.
  Identity gating, all four write-time hashes, and all three immutable post-boot
  digests passed. The retained runtime capture SHA-256 is
  `621f24c2229f3d9656c305c72df3ba8764a56244a83a944400c650013c4d0a4a`:
  GPIO43 TE measured 59.483 Hz, the official hardware-orientation/`TE_SYNC` path
  started, and 33 timing windows contained no panic, watchdog, or reset. Dynamic
  presentation nevertheless stayed mostly at 14.84–14.86 FPS, with periodic
  roughly 9 FPS windows, 61–65 ms average render time, and 31–35 ms average flush
  time. Marcos then observed that the image is rotated 180 degrees and the diagonal
  tearing absent from the previous version has returned. The exact combined
  implementation fails both performance and physical presentation, but D-052's
  source audit finds that it changed independent orientation, render-mode and
  buffering variables. Waveshare's `MADCTL=0xA0` became `0x60`, explaining the
  inversion; adapter FULL/single-buffer serialization explains the throughput. The
  diagonal remains unisolated and hardware orientation is not rejected generally.
  Commit `443eb72` is retained as Golden Prototype 1 and no further flash is
  authorized.

- The 50 Hz experiment first failed with an expected 20 ms value versus the active
  13 ms profile. After moving the cadence and reporting target into the shared
  display profile, native tests pass 23/23. The complete ESP-IDF 6.0.2 build is
  754,192 bytes with SHA-256
  `44ba2a5c69fe8963aefaabf94fc9d4ce0293ebfbf5639da9c2d8c51fe674c964`;
  effective LVGL refresh is 20 ms, demo mode remains enabled, and no flash was
  performed.
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
- Marcos authorized the correction flash on 2026-08-15. App `728c4de` reproduced
  44–56 FPS warning windows with no loop because repeated full-screen invalidation
  starved audio. Edge-gated app `7c3a7c5` then recorded six matched warning-loop
  start/idle pairs, proving that the double beep repeats for each complete warning,
  but hidden gauge rendering still produced 45–57 FPS warning windows. Final app
  `9d49ead` freezes that obscured rendering and pauses the completed-frame assertion
  only during each intentional one-second static red field. Its 754,192-byte image,
  SHA-256 `f0975d82b45ba927a3fccc2ffe6937ed46b0e0487a12789e6517d36e9f34a699`, passed all
  write hashes and all three immutable post-boot digests. The retained final capture,
  SHA-256 `51f326b213544032cbc8aea54ae07f278f86ab622da01df94afbc419d5c3178e`, records 44
  dynamic windows at 63–76 FPS and 12 matched static-red pause/resume pairs without
  a runtime fault. Sound was disabled in final NVS, so physical continuous-audio,
  puff, menu, and tearing judgments remain.
