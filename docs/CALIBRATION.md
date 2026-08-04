# MTX-D Characterization and Calibration

## Why this is mandatory

Innovate identifies pressure sensor `12-0074` (0–150 PSI) and thermistor
`15-0049`, but the public manual does not publish:

- excitation voltage;
- transducer pin order;
- voltage-to-PSI function;
- thermistor nominal resistance, Beta, tolerance, or R/T table.

Assuming a common 0.5–4.5 V sensor or 10 kΩ NTC can create false readings or
damage the ADC. Calibrated output remains disabled until this procedure passes.

## Core rule

Keep the MTX-D connected and operational while collecting high-impedance
parallel measurements. Only after matched curves and fault tests pass may the
new electronics power the sensors directly.

## 1. Inventory and photographs

With the vehicle off:

1. Photograph MTX-D rear label and every connector.
2. Capture both sides, latch/keying, and wire colors.
3. Identify the main ground and additional pressure-sensor ground by evidence.
4. With sensors disconnected, check threaded-body continuity to their wires.
5. Locate Innovate 38400 and USB-RS232 adapters.

Never measure resistance on a powered/connected sensor circuit.

## 2. Identify the pressure interface

With MTX-D powered on a bench or in the vehicle and engine stopped:

1. Confirm MTX-D reads 0 PSI.
2. Use insulated back-probes without piercing insulation.
3. Measure every connector pin relative to the common ground.
4. Identify ground (~0 V), stable excitation, and pressure-dependent signal.
5. Repeat at idle only after probes are insulated and secured.

Record every value. Never assign function by color.

## 3. Pressure curve

### Minimum installed-sensor method

Capture synchronized:

- MTX-D/LogWorks pressure;
- raw A0 voltage;
- A2 excitation;
- engine/RPM state.

Minimum points: ignition on/engine stopped; cold start; cold idle; cold 2000
and 3000 rpm; hot idle; hot 2000 and 3000 rpm.

Fit `PSI = slope × V_sensor + offset` only when residuals show no meaningful
curvature. Preserve margin to 150 PSI even if the UI displays 0–145 PSI.

### Preferred bench method

If safe removal is possible, use a limited 0–10 bar calibrator and reference
gauge at least at 0, 2, 4, 6, 8, and 10 bar. Use rated fittings and never an
unlimited pressure source.

## 4. Thermistor curve

### Map what MTX-D expects

With thermistor disconnected and MTX-D powered:

1. Connect a decade box to the temperature input.
2. Vary resistance without shorting the input.
3. Record resistance and MTX-D/LogWorks temperature.
4. Concentrate points from 49–138 °C.
5. Repeat points in both directions to detect hysteresis.

### Validate the real sensor

Measure the disconnected thermistor at ambient, fully cold, and multiple warm
states against a reference thermometer. A spare sensor enables a controlled
water bath. Do not use flame or improvise a hot-oil bath above 100 °C.

The firmware can use Steinhart-Hart:

```text
1 / T_kelvin = A + B·ln(R) + C·ln(R)^3
```

Keep original points and validate with points excluded from fitting.

## 5. MTS and LogWorks

1. Connect OUT → Innovate 38400 → USB-RS232 → PC.
2. Confirm both channels in LogWorks.
3. Export timestamped reference data.
4. Capture ESP32 raw channels in parallel.
5. Align streams using a clear ignition/start event.
6. Observe RS-232 through MAX3232 initially in RX-only mode.

Never connect RS-232 voltage directly to GPIO44.

## 6. MTX-D removal gate

- Pinout confirmed by measurement, not color.
- Correct 0 PSI with engine stopped.
- Pressure error target ≤2 PSI in the normal range.
- Temperature error target ≤2 °C from 60–130 °C.
- Open/short detection for both sensors.
- Cold/hot test repeated on at least three drives.
- Alarm behavior tested with simulation, never a deliberately induced engine fault.
- Engine-start reset produces no persistent false value.
- Power, brightness, thermal, harness, and enclosure checks pass.

Until every applicable item has evidence, MTX-D remains the reference instrument.
