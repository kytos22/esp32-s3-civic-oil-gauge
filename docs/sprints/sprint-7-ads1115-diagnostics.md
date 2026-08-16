# Sprint 7 — ADS1115 bench diagnostics

- Scope: prove the externally wired ADS1115 on the display's shared I²C bus before
  any Innovate sensor or analog front end is connected.
- Acceptance: AC-42.
- Safety boundary: USB bench power only; A0–A3 and ALERT remain unconnected;
  demo mode and invalid sensor calibrations remain binding.
- Status: AC-42 exact-board runtime proof complete; channel grounding/divider checks
  are the next separately supervised bench step.

## Slices

| Slice | Status | Test point result | Notes |
|---|---|---|---|
| 7.1 Protocol core | software green | red missing header, then native 29/29 | Builds 0xC383–0xF383 single-shot words at ±4.096 V and converts signed counts at 125 µV/LSB |
| 7.2 Shared-bus diagnostic | green | AC-42 source/build/runtime gates | ESP-IDF 6.0.2 build and source invariants pass; probe 0x48 after BSP bus initialization and log raw floating A0–A3 once per second without changing the gauge |
| 7.3 Exact-board proof | runtime green | HARDWARE | Authorized app `pb2-d057-17-gd770bb5`; exact identity and four write hashes pass; 0x48 found; A0–A3 repeatedly read about 0.552–0.562 V floating; touch/audio/display coexist and display counters remain zero |
