# Sprint 5 — Onboard warning audio

- Scope: exercise the Waveshare ESP32-S3-Touch-AMOLED-2.16 integrated speaker
  when the calibration-safe demo enters its pressure-warning state, without
  changing the approved visual behavior or blocking the 60 FPS UI loop.
- Acceptance:
  - one double beep is requested only on a non-warning to warning transition;
  - remaining in warning does not retrigger audio and leaving warning re-arms it;
  - PCM playback uses the pinned Waveshare BSP's ES8311/I²S path in a separate
    FreeRTOS task;
  - audio initialization or playback failure is logged and leaves the visual
    gauge operational;
  - `CONFIG_OIL_GAUGE_DEMO_MODE=y` remains enabled;
  - native tests and the complete ESP-IDF firmware build pass;
  - physical sound and post-change FPS remain `HARDWARE` until Marcos separately
    authorizes flashing the exact board.
- Status: in progress — explicitly requested by Marcos on 2026-08-11

## Hardware verification log

- Exact-board app `0c33fe6` flashed on 2026-08-11; all four regions passed
  esptool hash verification and the app booted with demo audio at 35%.
- The first bounded run initialized the ES8311 successfully and was stable, but
  periodic audio-load windows fell to 58–59 completed FPS. D-032 records the
  14 ms remediation. Its longer retest still exposed one 58 FPS window at the
  third warning entry; D-033 isolates larger PCM writes on CPU1 pending retest.

## Slices

| Slice | Status | Test point result | Notes |
|---|---|---|---|
| 5.1 Warning-entry gate | complete | red observed, then native 15/15 pass | Pure state transition logic |
| 5.2 ES8311 playback worker | complete in software | complete ESP-IDF 6.0.2 build pass | Non-blocking double beep; silent degradation on error |
| 5.3 Physical proof | pending | `HARDWARE` | Separate flash authorization required |
