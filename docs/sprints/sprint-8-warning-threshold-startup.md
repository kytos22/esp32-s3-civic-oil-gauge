# Sprint 8 — Configurable pressure warning and startup logo

- Scope: add two persistent menu settings without enabling sensor acquisition.
- Acceptance: AC-43 and AC-44.
- Safety boundary: warning remains engine-state gated; demo mode and invalid sensor
  calibrations remain binding; no vehicle or analog input is involved.
- Status: exact-board runtime passed; guided visual/touch and persistence judgment
  in progress.

## Slices

| Slice | Status | Test point result | Notes |
|---|---|---|---|
| 8.1 Warning threshold | software green | red compile then native 31/31 | Stores canonical whole PSI in NVS, clamps 1–30 PSI, presents PSI or one-decimal BAR, and feeds the same pressure-state/audio path |
| 8.2 Honda/Civic startup splash | corrected runtime green; JUDGMENT pending | 23/23 source contract + complete build + exact-board flash/boot | Reuses the boost project's 320×215 Honda and 310×42 Civic artwork on opaque black; all four writes verified and boot is clean; persistent 0–10 s duration, 1 s default and 0 disables |
| 8.3 Exact-board proof | runtime green; HARDWARE/JUDGMENT pending | exact identity, four verified writes and bounded boot | Exact app boots with 59.555 Hz TE, touch/audio/ADS1115 ready and zero timeout/error/fatal counters; check menu touch, NVS persistence, 0/1/10 s timing, artwork colors/centering, transition and no tearing |
