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
saturates at both ends. The controller adds the saved −30…+30-point user offset
and then clamps that adjusted curve to the saved AUTO minimum and maximum.

### `AutomaticBrightnessController`

`reset()` establishes AUTO or MANUAL with the saved manual backup, AUTO limits,
and curve offset. `setPreferences()` handles explicit mode, backup, limit, and
offset changes.
`update()` consumes a coherent receiver snapshot and local monotonic
milliseconds, returning an `AutomaticBrightnessStatus`. Changing either AUTO
limit or curve offset recomputes the constrained target from the current fresh
sample even when its receive generation has not changed.

The AUTO target uses a two-percentage-point deadband, so a mapped 34↔35% input
does not toggle the panel. The applied output slews toward an accepted target at
40 percentage points/s while brightening and 25 percentage points/s while
dimming. Across the default 20–100% range this is about 2.0 s up and 3.2 s down.
MANUAL remains immediate. Fallback keeps its independent 1.5 s interpolation.
`automaticPercent` reports the stabilized AUTO target; `appliedPercent` reports
the current point on the physical ramp.

The states are:

- `manual`: always apply the saved slider value and ignore lux for output;
- `waitingForSamples`: hold the backup until two new consecutive usable frames;
- `automatic`: apply the current mapped lux while it remains fresh;
- `fallback`: interpolate to the backup over 1.5 seconds and require two new
  consecutive usable frames before resuming AUTO.

`persistenceRequested` is always false. Automatic adaptation can therefore not
cause NVS wear; persistence belongs to explicit settings actions.
