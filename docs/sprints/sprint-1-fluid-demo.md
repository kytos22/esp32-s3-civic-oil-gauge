# Sprint 1 — Fluid 60 FPS demo and glanceable states

- Scope: replace the stepped four-hertz demo with continuous interpolation,
  target at least 60 completed display frames per second, measure the real panel
  flush rate, and enlarge the semantic state labels without changing the approved
  50/50 geometry, icons, thresholds, or calibration safety gate.
- Acceptance:
  - demo values interpolate continuously between all seven existing scenes;
  - application updates and LVGL refresh are scheduled every 13 ms;
  - the adapter logs completed display FPS over one-second windows;
  - sustained hardware evidence reports at least 60 FPS, or the slice remains open
    with the measured bottleneck recorded honestly;
  - pressure and temperature state labels use a 24 px Montserrat Spanish subset,
    remain right-aligned, and do not overlap or clip on the physical 480×480 panel;
  - native tests and the complete ESP-IDF firmware build pass;
  - demo mode remains enabled and no sensor, ADC, 12 V, or vehicle path is enabled.
- Status: reopened — software, exact-board flash, and runtime complete; Marcos's
  physical warning-animation judgment remains required

## Slices

| Slice | Status | Test point result | Notes |
|---|---|---|---|
| 1.1 Continuous deterministic demo | complete | software and completed-frame cadence pass | Linear interpolation at a 13 ms cadence with fractional-pixel bar edges |
| 1.2 Larger semantic labels | complete | physical visual pass | Every short and long state is readable, right-aligned, unclipped, and free of overlap in the supplied 60 FPS video |
| 1.3 Real FPS instrumentation | complete | eight consecutive windows at 65–67 FPS | Exact-board app `3e0298a`; passing bounded log retained |
| 1.4 Hardware proof and close-out | complete | exact-board flash/boot/FPS and physical visual pass | Full demo cycle reviewed; warning blink is intentional and numeric values remain continuously visible |
| 1.5 Hard 2 Hz warning blink | runtime pass; judgment pending | red-first 0/6, then 16/16 native + 6/6 contract + full build; exact-board app `002581d`, 16 windows at 64–77 FPS | Dotted 20% phase removed; 250 ms on/off timing flashed; Marcos must confirm the physical appearance |

## Physical close-out evidence

- User-supplied video `VID20260814204944.mp4`: 1920×1080, 60 FPS,
  28.423 seconds, 116,279,355 bytes, SHA-256
  `f27838f6ffb06107d2663e3cd1fef015730935794a4293358aedceede95f749b`.
- User-supplied photo `IMG20260814205043.jpg`: 3,473,988 bytes, SHA-256
  `7680d3fc5ee6320971c5544c2f6dff49f00c83ecb045bbeeddb0e74b29428cfb`.
- Corrected exact-board app `002581d` was flashed on 2026-08-14 after matching the
  locally recorded VID/PID, USB serial, and ESP32-S3 MAC. All four regions passed
  write-time verification; immutable bootloader, partition table, and application
  passed post-boot verification. The retained 9,274-byte serial capture has SHA-256
  `6251e71a1df6f8fd38447216e068198d7b3bf9004691d9a33bd4776f26893b63`
  and records a clean demo boot, two warning tones, and 16 consecutive 64–77 FPS
  windows without panic, watchdog, or audio/display error. Post-boot `otadata`
  differs from its empty initial image because the bootloader selects app0.
- The complete physical demo cycle shows `DEMO`, `<50`, every pressure and
  temperature semantic label, centered values, intact Spanish accents, the exact
  50/50 split, and the black AMOLED background without glyph corruption, clipping,
  or overlap. It also exposed a dotted-looking low-opacity warning phase, so it no
  longer closes the corrected warning animation; the pressure number remains visible
  throughout.
- This evidence retains indoor fidelity and handheld-distance readability for all
  non-warning-animation elements. The corrected warning is now running on the exact
  display and needs Marcos's physical judgment.
  Daylight, night, glare, and in-vehicle motion remain separate pre-vehicle checks.
