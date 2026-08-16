# Sprint 7 — ADS1115 bench diagnostics

- Scope: prove the externally wired ADS1115 on the display's shared I²C bus before
  any Innovate sensor or analog front end is connected.
- Acceptance: AC-42.
- Safety boundary: USB bench power only; A0–A3 and ALERT remain unconnected;
  demo mode and invalid sensor calibrations remain binding.
- Status: software candidate in progress; no flash authorized.

## Slices

| Slice | Status | Test point result | Notes |
|---|---|---|---|
| 7.1 Protocol core | software green | red missing header, then native 29/29 | Builds 0xC383–0xF383 single-shot words at ±4.096 V and converts signed counts at 125 µV/LSB |
| 7.2 Shared-bus diagnostic | build pending | AC-42 source/build/runtime gates | Probe 0x48 after BSP bus initialization; log raw floating A0–A3 once per second without changing the gauge |
| 7.3 Exact-board proof | pending authorization | HARDWARE | Confirm 0x48, four raw readings, touch/audio/display coexistence and zero I²C/display faults |
