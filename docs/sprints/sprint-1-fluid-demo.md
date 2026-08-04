# Sprint 1 — Fluid 60 FPS demo and glanceable states

- Scope: replace the stepped four-hertz demo with continuous interpolation,
  target at least 60 completed display frames per second, measure the real panel
  flush rate, and enlarge the semantic state labels without changing the approved
  50/50 geometry, icons, thresholds, or calibration safety gate.
- Acceptance:
  - demo values interpolate continuously between all seven existing scenes;
  - application updates and LVGL refresh are scheduled every 15 ms;
  - the adapter logs completed display FPS over one-second windows;
  - sustained hardware evidence reports at least 60 FPS, or the slice remains open
    with the measured bottleneck recorded honestly;
  - pressure and temperature state labels use a 24 px Montserrat Spanish subset,
    remain right-aligned, and do not overlap or clip on the physical 480×480 panel;
  - native tests and the complete ESP-IDF firmware build pass;
  - demo mode remains enabled and no sensor, ADC, 12 V, or vehicle path is enabled.
- Status: in progress — explicitly requested by Marcos on 2026-08-04

## Slices

| Slice | Status | Test point result | Notes |
|---|---|---|---|
| 1.1 Continuous deterministic demo | complete | software and completed-frame cadence pass | Linear interpolation at a 15 ms cadence with fractional-pixel bar edges |
| 1.2 Larger semantic labels | pending | pending | 24 px, right-aligned, physical check required |
| 1.3 Real FPS instrumentation | complete | eight consecutive windows at 65–67 FPS | Exact-board app `3e0298a`; passing bounded log retained |
| 1.4 Hardware proof and close-out | in progress | exact-board flash/boot/FPS pass | Straight-on visual judgment remains before Sprint close |
