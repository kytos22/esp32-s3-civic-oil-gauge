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
| `WarningToneGate` | class | `include/warning_tone_gate.h` | [warning tone gate](warning-tone-gate.md) | Emits start/stop commands for the persistent warning-audio loop |
| `warningBlinkPhaseOn()` | function | `include/demo_sequence.h` | `docs/api/display-state.md` | Deterministic binary 2 Hz warning phase |
| `fullScreenWarningPhaseOn()` | function | `include/demo_sequence.h` | [gauge settings](gauge-settings.md) | Deterministic 0.5 Hz full-screen warning phase |
| `DataSource` | enum | `include/gauge_settings.h` | [gauge settings](gauge-settings.md) | Select demo or provisional A1 sensor bench source |
| `TemperatureUnit` | enum | `include/gauge_settings.h` | [gauge settings](gauge-settings.md) | Select Celsius or Fahrenheit presentation |
| `BrightnessMode` | enum | `include/gauge_settings.h` | [gauge settings](gauge-settings.md) | Select automatic CivicAux lux or saved manual brightness |
| `UiLanguage` | enum | `include/gauge_settings.h` | [gauge settings](gauge-settings.md) | Select persistent Spanish or English presentation |
| `GaugeSettings` | struct | `include/gauge_settings.h` | [gauge settings](gauge-settings.md) | Sanitized persistent display/audio/source preferences, including AUTO limits and curve adjustment |
| `sanitizeGaugeSettings()` | function | `include/gauge_settings.h` | [gauge settings](gauge-settings.md) | Clamp preferences, order AUTO limits, and reject invalid enum values |
| `pressureForDisplay()` | function | `include/gauge_settings.h` | [gauge settings](gauge-settings.md) | Convert canonical PSI for display only |
| `temperatureForDisplay()` | function | `include/gauge_settings.h` | [gauge settings](gauge-settings.md) | Convert canonical Celsius for display only |
| `temperatureWarningThresholdForDisplay()` | function | `include/gauge_settings.h` | [gauge settings](gauge-settings.md) | Present the canonical warning threshold in °C or °F |
| `temperatureWarningThresholdCelsiusFromDisplay()` | function | `include/gauge_settings.h` | [gauge settings](gauge-settings.md) | Convert the menu threshold back to canonical Celsius |
| `WarningPresentation` | struct | `include/gauge_settings.h` | [gauge settings](gauge-settings.md) | Renderer-independent warning visibility decision |
| `evaluateWarningPresentation()` | function | `include/gauge_settings.h` | [gauge settings](gauge-settings.md) | Apply separate element/full-screen warning phases |
| `CivicAuxFrameV1` | struct | `include/civic_aux_receiver.h` | [CivicAux receiver](civic-aux-receiver.md) | Fixed-storage decoded protocol-v1 frame |
| `CivicAuxFrameParser` | class | `include/civic_aux_receiver.h` | [CivicAux receiver](civic-aux-receiver.md) | Non-blocking fixed-buffer stream parser with resynchronization |
| `CivicAuxReceiver` | class | `include/civic_aux_receiver.h` | [CivicAux receiver](civic-aux-receiver.md) | Oil-target acceptance, freshness, continuity, and diagnostics |
| `CivicAuxSnapshot` | struct | `include/civic_aux_receiver.h` | [CivicAux receiver](civic-aux-receiver.md) | Coherent trivially-copyable UART-to-main snapshot |
| `civicAuxCrc16CcittFalse()` | function | `include/civic_aux_receiver.h` | [CivicAux receiver](civic-aux-receiver.md) | Protocol CRC-16/CCITT-FALSE implementation |
| `AutomaticBrightnessController` | class | `include/automatic_brightness.h` | [automatic brightness](automatic-brightness.md) | AUTO/MANUAL limits, target deadband, output ramp, fallback, and recovery state machine |
| `automaticBrightnessPercentForMillilux()` | function | `include/automatic_brightness.h` | [automatic brightness](automatic-brightness.md) | Provisional log1p lux-to-percent mapping |

Full per-surface documentation is created when a surface is next changed. Until then this index is the complete lookup layer.
