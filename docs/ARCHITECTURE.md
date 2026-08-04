# Hardware Architecture

## Decision

Two reversible routes are retained; **Route A** is the target.

| Route | Retained hardware | Added hardware | Advantage | Cost/risk |
|---|---|---|---|---|
| A. Direct sensors | Sensors and installed wiring | ADS1115 + protected analog conditioning | MTX-D can be removed | Both sensor interfaces must be characterized |
| B. MTS | Sensors and hidden MTX-D electronics | 3.3 V RS-232 receiver | Retains Innovate's conversions | MTX-D body remains powered/hidden; MTS must be decoded |

Route B is also a development reference. If MTS exposes synchronized pressure
and temperature, it can validate Route A independently of visual readings.

## Route A — direct acquisition

```text
Switched vehicle 12 V
  ├── 2 A fuse near ACC
  ├── automotive TVS + reverse-polarity protection
  └── wide-input 5 V buck, at least 3 A
        ├── Waveshare VBUS
        └── pressure-sensor excitation (value still unknown)

Waveshare 3V3 ── ADS1115 (bench module first; Q1 part on final PCB)
GPIO15 SDA ───── ADS1115 SDA
GPIO14 SCL ───── ADS1115 SCL
Star ground ──── ESP32 + ADC + sensors

Pressure signal ── divider/protection/RC ── A0
Thermistor ─────── selectable pull-up/RC ── A1
Pressure excitation monitor /2 ──────────── A2
Protected 12 V lighting input ───────────── A3
```

### Why an external ADC

An external ADC is effectively required. The board conveniently exposes I²C
and UART; GPIO14/15 already form the shared I²C bus with 2.2 kΩ pull-ups,
GPIO19/20 are native USB, and GPIO43/44 are not ADC inputs. Reusing internal
board signals would be invasive and less repeatable for two analog sensors.

ADS1115-Q1 provides four single-ended channels, 16-bit conversion, PGA,
internal reference, up to 860 SPS, I²C, and AEC-Q100 Grade 1 temperature
qualification. Oil changes slowly; 128 SPS plus digital filtering is ample.

The ADC runs at **3.3 V** and address `0x48`. An input must never exceed
VDD + 0.3 V even when the selected PGA full-scale range is larger. Every
possible 5/12 V signal therefore requires division and protection.

### Provisional bench front end

These values support characterization only; they are not a final PCB design.

| Channel | Provisional circuit | Purpose |
|---|---|---|
| A0 | 33 kΩ / 33 kΩ, 0.1%, 100 nF, low-leakage clamp | Pressure signal up to 5 V |
| A1 | selectable 2.49/4.99/10 kΩ pull-up, 0.1%, 100 nF | Thermistor |
| A2 | 33 kΩ / 33 kΩ, 0.1%, 100 nF | Excitation monitoring |
| A3 | 150 kΩ / 22 kΩ, 0.1%, 100 nF, clamp | Lighting detection |

Measure A0 minimum/maximum before connecting it. Measure thermistor resistance
only while disconnected. Select the pull-up from evidence so the useful range
uses the ADC well without excessive self-heating.

### Grounding

Innovate requires the pressure sensor's additional black wire to share the
gauge ground. Use a star point for sensor, ADC, ESP32, and converter input,
away from ignition, fuel pump, radio, alternator, and audio grounds.

## Route B — MTX-D as the conditioner

```text
Sensors ── MTX-D OUT ── Innovate 38400 cable ── RS-232
                                                   │
                                                   ▼
                                      MAX3232E-Q1 / TRS3232E-Q1
                                                   │ 3.3 V UART
                                                   ▼
                                      Waveshare GPIO44 RX
```

Public MTS documentation suggests 19200 baud, 8N1, big-endian 16-bit words.
Treat that as a hypothesis. Capture this MTX-D's frames and compare with
LogWorks. Start RX-only so the ESP32 cannot send accidental commands.

If stable, Route B does not need ADS1115 for oil values, but the powered 52 mm
MTX-D body must remain hidden.

## Power

### Bench

- Quality USB-C data cable and 5 V / 3 A supply.
- Sensors disconnected for first power-on.
- ADS1115 powered only from the 3V3 pad.

### Vehicle prototype

- Dedicated 2 A fuse as close to ACC as practical.
- Automotive load-dump TVS.
- Wide-input 5 V converter.
- Prototype candidate: Pololu D36V28F5 (5 V, 3.2 A, up to 50 V input,
  reverse-polarity protection).

The prototype module alone is not automotive validation. Final verification
must cover transients, jump start, alternator charging, EMI, temperature, and
USB/external-supply interaction.

### Final PCB candidates

Select only after real current and signal measurements:

- `ADS1115-Q1` VSSOP-10.
- 60/65 V automotive buck such as `LM76003-Q1`/`LM65635-Q1`.
- `SLD8S24A` TVS coordinated with fuse and converter.
- `TRS3232E-Q1` only if MTS is adopted.

## Mechanical concept

The official enclosure is about 46×46×8.3 mm and the useful display is about
39×39 mm. A 52 mm circular gauge opening cannot contain the square enclosure
corners directly.

The mount therefore needs:

- a cylindrical stem for the 52 mm opening;
- a square face larger than the opening;
- lower/side USB-C and harness exit;
- PWR/BOOT service access;
- heat-resistant ASA or technical PETG, not PLA.

Freeze no CAD dimension until the delivered board and vehicle mount are measured.
