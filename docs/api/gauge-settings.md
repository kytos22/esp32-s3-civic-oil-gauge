# Gauge settings API

## Purpose

`include/gauge_settings.h` defines the pure, hardware-independent preference model
used by the LVGL menu, NVS adapter, and renderer. It does not acquire sensor data or
change calibration.

## Settings surfaces

### `DataSource`

`demo` selects the synthetic sequence. `sensors` reads ADS1115 A1 through the
provisional resistor-bench temperature curve; pressure remains behind its
calibration gate and renders `--` / `SIN DATOS`.

### `BrightnessMode`

`automatic` receives CivicAux ambient lux and is the default for a new install,
an absent or invalid NVS mode key, and factory reset. `manual` applies only the
saved slider value. The slider always represents the persistent manual value and
AUTO fallback; automatic samples do not change it.

AUTO has a separate persistent minimum/maximum range. Its defaults are 20% and
100%; both endpoints are constrained to 5–100% and reordered safely if stored
data is corrupt. These limits do not constrain MANUAL or the fallback target.

`brightnessModeFromStoredValue()` provides absent-key migration and rejects
invalid enum bytes back to AUTO. `brightnessModeStoredValue()` provides the
sanitized NVS representation.

### `GaugeSettings`

Carries manual/backup brightness, AUTO minimum/maximum, brightness mode, warning
sound enable/volume, pressure unit, temperature unit,
canonical low-pressure warning PSI, canonical high-temperature warning Celsius,
startup-logo seconds, warning visual mode, and data source.
`sanitizeGaugeSettings()` clamps percentages to 5–100, orders the AUTO limits,
warning pressure to
1–30 PSI, temperature warning to 110–140 °C, startup duration to 0–10 seconds,
and replaces invalid enum representations with safe demo defaults.

### `pressureForDisplay()`

Converts canonical PSI to PSI or bar for display only. It never changes thresholds,
bar fractions, calibration, or alarm evaluation.

### `warningThresholdForDisplay()` and `warningThresholdPsiFromDisplay()`

Convert the canonical whole-PSI warning threshold to the selected menu unit and
back. BAR presentation uses one decimal place; conversion is clamped to 1–30 PSI.
Changing the unit alone never rewrites the canonical setting.

### `TemperatureUnit` and `temperatureForDisplay()`

`celsius` and `fahrenheit` select presentation only.
`temperatureForDisplay(temperatureC, unit)` returns the canonical Celsius input
unchanged or applies `°F = °C × 9/5 + 32`. Temperature states, colors, bar fractions,
calibration, and alarms always consume the original Celsius value. The renderer maps
the demo lower floor to `<50 °C` or `<122 °F`; sensor bench mode can show the
provisional numeric value below that visual floor.

`temperatureWarningThresholdForDisplay()` presents the canonical 110–140 °C
warning threshold in the selected unit. The inverse
`temperatureWarningThresholdCelsiusFromDisplay()` converts a Fahrenheit menu
selection back to canonical Celsius and clamps it before persistence. Unit
changes therefore update the menu label and slider without changing alarm math.

## Warning presentation

`evaluateWarningPresentation()` receives separate element and full-screen phases.
Element mode uses the binary 2 Hz phase. Full-screen mode uses the independent 0.5 Hz
phase. Fixed mode keeps attention visible. Every mode keeps the pressure value
visible.

The phase generators are `warningBlinkPhaseOn()` and
`fullScreenWarningPhaseOn()` in `include/demo_sequence.h`. They are pure functions of
the supplied microsecond timestamp and therefore have deterministic boundaries in
native tests.
