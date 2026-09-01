# Automatic brightness API

## Purpose

`include/automatic_brightness.h` defines the pure state machine that converts a
`CivicAuxSnapshot` plus saved user preferences into one applied brightness
percentage. It performs no I/O, NVS access, LVGL work, or panel calls.

## Public surfaces

### `automaticBrightnessPercentForMillilux()`

Maps millilux to 5–100% by linearly interpolating over `log1p(lux)` between the
nine provisional control points documented in
[`CIVIC_AUX_INTEGRATION.md`](../CIVIC_AUX_INTEGRATION.md). It is monotonic and
saturates at both ends.

### `AutomaticBrightnessController`

`reset()` establishes AUTO or MANUAL with the saved manual backup.
`setPreferences()` handles explicit mode/backup changes. `update()` consumes a
coherent receiver snapshot and local monotonic milliseconds, returning an
`AutomaticBrightnessStatus`.

The states are:

- `manual`: always apply the saved slider value and ignore lux for output;
- `waitingForSamples`: hold the backup until two new consecutive usable frames;
- `automatic`: apply the current mapped lux while it remains fresh;
- `fallback`: interpolate to the backup over 1.5 seconds and require two new
  consecutive usable frames before resuming AUTO.

`persistenceRequested` is always false. Automatic adaptation can therefore not
cause NVS wear; persistence belongs to explicit settings actions.
