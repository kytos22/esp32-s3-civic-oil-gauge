# API Index — Civic ESP32 Oil Gauge

> One line per externally reusable measurement-core surface. Search here before adding another.

| Surface | Kind | Code file | Doc | Purpose |
|---|---|---|---|---|
| `LinearCalibration` | struct | `include/gauge_core.h` | progressive backfill | Pressure slope/offset plus validity gate |
| `SteinhartHartCalibration` | struct | `include/gauge_core.h` | progressive backfill | Thermistor coefficients plus validity gate |
| `FrontendConfig` | struct | `include/gauge_core.h` | progressive backfill | Divider, pull-up, and excitation parameters |
| `Fault` | enum | `include/gauge_core.h` | progressive backfill | Explicit conversion/acquisition failure reason |
| `ConvertedValue` | struct | `include/gauge_core.h` | progressive backfill | Value coupled to its validity/fault |
| `restoreDividerInput()` | function | `include/gauge_core.h` | progressive backfill | Recover sensor voltage from divider output |
| `thermistorResistance()` | function | `include/gauge_core.h` | progressive backfill | Convert divider node voltage to resistance |
| `convertPressure()` | function | `include/gauge_core.h` | progressive backfill | Calibrated pressure conversion with faults |
| `convertTemperature()` | function | `include/gauge_core.h` | progressive backfill | Calibrated thermistor conversion with faults |
| `clamp()` | function | `include/gauge_core.h` | progressive backfill | Bound a scalar |
| `lowPass()` | function | `include/gauge_core.h` | progressive backfill | First-order sample filter |
| `AlarmThresholds` | struct | `include/gauge_core.h` | `docs/api/display-state.md` | Provisional severe alarm thresholds |
| `EngineState` | struct | `include/gauge_core.h` | `docs/api/display-state.md` | Hidden known/RPM input and running predicate |
| `AlarmState` | struct | `include/gauge_core.h` | `docs/api/display-state.md` | Pressure, temperature, and sensor fault flags |
| `evaluateAlarms()` | function | `include/gauge_core.h` | `docs/api/display-state.md` | Engine-gated alarm evaluation |
| `PressureState` | enum | `include/gauge_core.h` | `docs/api/display-state.md` | Deterministic pressure visual state |
| `TemperatureState` | enum | `include/gauge_core.h` | `docs/api/display-state.md` | Deterministic temperature visual state |
| `RgbColor` | struct | `include/gauge_core.h` | `docs/api/display-state.md` | Renderer-independent RGB token |
| `DisplayState` | struct | `include/gauge_core.h` | `docs/api/display-state.md` | Complete deterministic renderer decision |
| `evaluatePressureState()` | function | `include/gauge_core.h` | `docs/api/display-state.md` | RPM-aware pressure state mapping |
| `evaluateTemperatureState()` | function | `include/gauge_core.h` | `docs/api/display-state.md` | Approved temperature-band mapping |
| `temperatureColor()` | function | `include/gauge_core.h` | `docs/api/display-state.md` | Approved continuous color interpolation |
| `evaluateDisplayState()` | function | `include/gauge_core.h` | `docs/api/display-state.md` | Warning motion, below-range, colors, and bars |
| `faultName()` | function | `include/gauge_core.h` | progressive backfill | Stable diagnostic name for a fault |
| `pressureStateName()` | function | `include/gauge_core.h` | `docs/api/display-state.md` | Stable pressure-state diagnostic name |
| `temperatureStateName()` | function | `include/gauge_core.h` | `docs/api/display-state.md` | Stable temperature-state diagnostic name |

Full per-surface documentation is created when a surface is next changed. Until then this index is the complete lookup layer.
