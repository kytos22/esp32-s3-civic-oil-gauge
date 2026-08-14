# Gauge settings API

## Purpose

`include/gauge_settings.h` defines the pure, hardware-independent preference model
used by the LVGL menu, NVS adapter, and renderer. It does not acquire sensor data or
change calibration.

## Settings surfaces

### `DataSource`

`demo` selects the synthetic sequence. `sensors` selects the calibration gate: the
renderer shows `--` and `SIN DATOS`, and no ADS1115 path or engineering-unit value is
enabled.

### `GaugeSettings`

Carries brightness, warning sound enable/volume, pressure unit, warning visual mode,
and data source. `sanitizeGaugeSettings()` clamps percentages to 5–100 and replaces
invalid enum representations with safe demo defaults.

### `pressureForDisplay()`

Converts canonical PSI to PSI or bar for display only. It never changes thresholds,
bar fractions, calibration, or alarm evaluation.

## Warning presentation

`evaluateWarningPresentation()` receives separate element and full-screen phases.
Element mode uses the binary 2 Hz phase. Full-screen mode uses the independent 0.5 Hz
phase. Fixed mode keeps attention visible. Every mode keeps the pressure value
visible.

The phase generators are `warningBlinkPhaseOn()` and
`fullScreenWarningPhaseOn()` in `include/demo_sequence.h`. They are pure functions of
the supplied microsecond timestamp and therefore have deterministic boundaries in
native tests.
