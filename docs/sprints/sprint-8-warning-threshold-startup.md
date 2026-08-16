# Sprint 8 — Configurable pressure warning and startup logo

- Scope: add two persistent menu settings without enabling sensor acquisition.
- Acceptance: AC-43 and AC-44.
- Safety boundary: warning remains engine-state gated; demo mode and invalid sensor
  calibrations remain binding; no vehicle or analog input is involved.
- Status: software candidate complete; exact-board flash and judgment pending fresh
  authorization.

## Slices

| Slice | Status | Test point result | Notes |
|---|---|---|---|
| 8.1 Warning threshold | software green | red compile then native 31/31 | Stores canonical whole PSI in NVS, clamps 1–30 PSI, presents PSI or one-decimal BAR, and feeds the same pressure-state/audio path |
| 8.2 Honda startup splash | software green | source contract + complete build | Reuses the boost project's 320×215 artwork on opaque black; persistent 0–10 s duration, 1 s default and 0 disables |
| 8.3 Exact-board proof | pending HARDWARE/JUDGMENT | fresh authorization required | Check menu scrolling/touch, NVS reboot persistence, 0/1/10 s timing, artwork colors/centering, transition and no tearing |
