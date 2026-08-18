# MTX-D Characterization and Calibration

## Why this is mandatory

Innovate identifies pressure sensor `12-0074` (0–150 PSI) and thermistor
`15-0049`, but the public MTX-D/12-0074 material does not publish:

- excitation voltage;
- transducer pin order;
- voltage-to-PSI function;
- thermistor nominal resistance, Beta, tolerance, or R/T table.

Assuming a common 0.5–4.5 V sensor or 10 kΩ NTC can create false readings or
damage the ADC. Calibrated output remains disabled until this procedure passes.

Innovate's separate `11-0161A` instructions for a 10 bar sensor with SSI-4 PLUS
adapter specify 5 V excitation and 0.5–4.5 V for 0–150 PSI. No consulted
official source equates that sensor explicitly with P/N `12-0074`; therefore
those values are the leading test hypothesis rather than installed-sensor proof.

## Core rule

Keep the MTX-D connected and operational while collecting high-impedance
parallel measurements. Only after matched curves and fault tests pass may the
new electronics power the sensors directly.

## 1. Inventory and photographs

The MTX-D wiring diagram supplied by Marcos on 2026-08-10 confirms the
following physical topology:

- oil temperature has two conductors: one gauge-side conductor and one
  dedicated ground conductor;
- oil pressure has three conductors: two gauge-side conductors and one
  dedicated ground conductor.

This is connector-topology evidence only. It does not identify which of the
two pressure conductors is excitation or signal, nor prove the excitation
voltage, signal range, thermistor type, or either conversion curve. The diagram
shows colours, but functions must still be assigned from measurements.

The MTX-D switched-12-V feed is not a sensor signal and must never reach the
ESP32 or ADS1115. It is still relevant during characterization because the
gauge derives the temperature bias and pressure excitation from its supply.
Record the MTX-D supply, sensor bias, and excitation with ignition on/engine
stopped and again while charging, so the replacement can reproduce only the
regulated sensor-side electrical conditions.

With the vehicle off:

1. Photograph MTX-D rear label and every physical connector face.
2. Capture both sides, latch/keying, pin numbering, and wire colors.
3. Confirm the main ground and both documented sensor-ground conductors by
   continuity evidence while the circuit is unpowered and disconnected.
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

### Candidate model to verify

If measurements prove that this installed `12-0074` receives 5 V and produces
0.5 V at 0 PSI and 4.5 V at 150 PSI, the candidate linear conversions are:

```text
PSI = (V_sensor - 0.5) × 37.5
bar = (V_sensor - 0.5) × 2.5
```

With equal 33 kΩ / 33 kΩ dividers on A0 and A2, `V_sensor = 2 × V_A0` and
`V_excitation = 2 × V_A2`. Do not compile these equations as valid calibration
until measured points across the operating range pass the residual/error gates.

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
6. If useful, capture MTS on the laptop through the Innovate cable and a real
   RS-232 or USB-to-RS-232 interface.

Never connect RS-232 voltage directly to a GPIO or TTL-UART adapter.

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
