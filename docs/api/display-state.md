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

`fault`, `belowRange`, `cold`, `warming`, `optimal`, `hot`, or `veryHot`.

### `RgbColor`

An 8-bit red/green/blue value used before conversion to the display's native format.

### `DisplayState`

Carries pressure and temperature semantic states, both colors, the visibility of
warning-attention elements, the `<50` decision, and normalized 0–1 bar fractions.
The numeric pressure value is deliberately not blink-gated.

### `evaluatePressureState()`

```cpp
PressureState evaluatePressureState(
    const ConvertedValue& pressure,
    const EngineState& engine);
```

Priority: invalid/negative pressure → fault; unknown engine → engine unknown; zero RPM
→ stopped; then 0–10 warning, above 10 and below 15 low, 15–80 OK, above 80 high.

### `evaluateTemperatureState()`

```cpp
TemperatureState evaluateTemperatureState(
    const ConvertedValue& temperature);
```

Maps the exact boundaries in AC-08/AC-10. A valid value below 50 °C is
`belowRange`, which instructs the renderer to show `<50`.

### `temperatureColor()`

```cpp
RgbColor temperatureColor(double temperatureC);
```

Linearly interpolates the approved RGB stops at 50, 57, 75, 89, 94, 100, and
138 °C, clamping outside the endpoints. Non-finite input returns the cold endpoint;
callers must still honor the separate fault state.

### `evaluateDisplayState()`

```cpp
DisplayState evaluateDisplayState(
    const ConvertedValue& pressure,
    const ConvertedValue& temperature,
    const EngineState& engine,
    bool blinkPhaseOn,
    bool reducedMotion);
```

Uses the canonical state functions. In warning state, attention elements are visible
when the 1 Hz phase is on, or continuously when reduced motion is requested. Pressure
and temperature bars normalize to 0–150 PSI and 50–138 °C.

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
