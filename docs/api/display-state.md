# Display and alarm state API

## Purpose

The public types and functions in `include/gauge_core.h` convert valid engineering
values plus hidden engine/RPM state into deterministic alarm and display decisions.
They perform no hardware access and never display RPM.

## Engine and alarm surfaces

### `EngineState`

```cpp
struct EngineState {
  bool known;
  std::uint32_t rpm;
  bool running() const;
};
```

- `known=false`: RPM is unavailable; low-pressure alarming is not armed.
- `known=true, rpm=0`: engine stopped; low-pressure alarming is suppressed.
- `known=true, rpm>0`: engine running; pressure thresholds may arm.

### `evaluateAlarms()`

```cpp
AlarmState evaluateAlarms(
    const ConvertedValue& pressure,
    const ConvertedValue& temperature,
    const EngineState& engine,
    const AlarmThresholds& thresholds);
```

Returns sensor-fault/high-temperature flags and a low-pressure flag that can become
true only when pressure is valid, non-negative, and `engine.running()` is true.
The provisional severe-temperature threshold remains independent of the visual
temperature bands.

## Display surfaces

### `PressureState`

`fault`, `engineUnknown`, `engineStopped`, `warning`, `low`, `ok`, or `high`.

### `TemperatureState`

`fault`, `belowRange`, `cold`, `warming`, `optimal`, `veryHot`, or `warning`.

### `RgbColor`

An 8-bit red/green/blue value used before conversion to the display's native format.

### `DisplayState`

Carries pressure and temperature semantic states, independent bar/text and icon
colors, warning-attention visibility, the `<50` decision, and normalized 0–1 bar
fractions. Numeric values are deliberately not blink-gated.

### `evaluatePressureState()`

```cpp
PressureState evaluatePressureState(
    const ConvertedValue& pressure,
    const EngineState& engine,
    double warningThresholdPsi = 10.0);
```

Priority: invalid/negative pressure → fault; unknown engine → engine unknown; zero RPM
→ stopped; then pressure up to the selected 1–30 PSI cut → warning, below 15 → low,
15–80 → OK, and above 80 → high.

### `evaluateTemperatureState()`

```cpp
TemperatureState evaluateTemperatureState(
    const ConvertedValue& temperature,
    double warningThresholdC = 120.0);
```

Maps below 50, 50–59, 60–75, 76–100, 101 to the selected warning cut, and
warning through 140 °C. A valid value below 50 °C is `belowRange`; demo renders
`<50`, while the A1 resistor-test source may show the provisional measured number.

### `temperatureColor()`

```cpp
RgbColor temperatureColor(
    double temperatureC,
    double warningThresholdC = 120.0);
```

Linearly interpolates the approved stops at 50/59 °C blue, 76 °C desaturated
green, 90 °C light amber, 100 °C orange, and the selected 110–140 °C warning
cut red. It clamps outside the endpoints. Non-finite input returns the cold
endpoint; callers must still honor the separate fault state.

### `evaluateDisplayState()`

```cpp
DisplayState evaluateDisplayState(
    const ConvertedValue& pressure,
    const ConvertedValue& temperature,
    const EngineState& engine,
    bool blinkPhaseOn,
    bool reducedMotion,
    double warningThresholdPsi = 10.0,
    double temperatureWarningC = 120.0);
```

Uses the canonical state functions. In warning state, attention elements are visible
when the caller-provided binary 2 Hz phase is on, or continuously when reduced motion
is requested. The pressure bar normalizes to 0–85 PSI and the temperature bar
to 50–140 °C, clamping outside those visual endpoints.

## Diagnostic names

`pressureStateName()` and `temperatureStateName()` return stable English diagnostic
identifiers. They return `"unknown"` for an invalid enum representation.

## Example

```cpp
const oilgauge::DisplayState state = oilgauge::evaluateDisplayState(
    {10.0, oilgauge::Fault::none},
    {92.0, oilgauge::Fault::none},
    {true, 2500},
    true,
    false);
```

This produces pressure `warning`, temperature `optimal`, red pressure attention, and
the interpolated optimal temperature color. No sensor conversion is implied.

### `warningBlinkPhaseOn()`

```cpp
bool warningBlinkPhaseOn(std::uint64_t nowUs);
```

Returns `true` for 250 ms and `false` for 250 ms, repeating as a deterministic
binary 2 Hz cycle. The renderer maps the false phase to full transparency; the
function does not affect reduced-motion behavior or the numeric pressure value.
