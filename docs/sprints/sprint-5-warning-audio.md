# Sprint 5 — Onboard warning audio

- Scope: exercise the Waveshare ESP32-S3-Touch-AMOLED-2.16 integrated speaker
  when the calibration-safe demo enters its pressure-warning state, without
  changing the approved visual behavior or blocking the 60 FPS UI loop.
- Acceptance:
  - the double beep repeats while the engine-gated warning remains active;
  - leaving warning stops the loop and a later warning starts it again;
  - PCM playback uses the pinned Waveshare BSP's ES8311/I²S path in a separate
    FreeRTOS task;
  - audio initialization or playback failure is logged and leaves the visual
    gauge operational;
  - `CONFIG_OIL_GAUGE_DEMO_MODE=y` remains enabled;
  - native tests and the complete ESP-IDF firmware build pass;
  - physical sound and post-change FPS remain `HARDWARE` until Marcos separately
    authorizes flashing the exact board.
- Status: complete — accepted by Marcos on 2026-08-11

## Hardware verification log

- Exact-board app `0c33fe6` flashed on 2026-08-11; all four regions passed
  esptool hash verification and the app booted with demo audio at 35%.
- The first bounded run initialized the ES8311 successfully and was stable, but
  periodic audio-load windows fell to 58–59 completed FPS. D-032 records the
  14 ms remediation. Its longer retest still exposed one 58 FPS window at the
  third warning entry. D-033 CPU1 isolation improved that window to 59 FPS but
  still failed. D-034 exact-board app `bf5c932` passes: all four regions verified,
  and the retained bounded log records three completed tone paths plus 29
  consecutive 66–77 FPS windows with no runtime fault. Marcos then confirmed
  that the integrated speaker's double beep was physically audible.
- The 2026-08-15 loop extension is also runtime-proven on the exact board. Ignored
  capture `sprint6-warning-loop-7c3a7c5.typescript` records six matched loop-start
  and loop-idle pairs, each warning lasting roughly 6.7 seconds, with persisted
  sound enabled. Final app `9d49ead` retains the same audio implementation; its
  persisted sound switch was off during the final graphics capture, so continuous
  audible quality and absence of an edge puff remain guided physical judgments.

## Slices

| Slice | Status | Test point result | Notes |
|---|---|---|---|
| 5.1 Warning-loop gate | complete | original red observed, latest regression then native 22/22 pass | Pure start/stop transition logic |
| 5.2 ES8311 playback worker | complete | complete ESP-IDF 6.0.2 build pass | Non-blocking double beep; silent degradation on error |
| 5.3 Physical proof | complete | exact-board 66–77 FPS and audible double beep | Accepted by Marcos on 2026-08-11 |
