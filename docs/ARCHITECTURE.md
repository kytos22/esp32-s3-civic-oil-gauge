# Hardware Architecture

## Decision

**Route A is the selected gauge implementation.** MTS remains available only
as a laptop-side calibration reference; no RS-232 level converter will be
installed in the ESP32 gauge.

| Route | Retained hardware | Added hardware | Advantage | Cost/risk |
|---|---|---|---|---|
| A. Direct sensors | Sensors and installed wiring | ADS1115 + protected analog conditioning | MTX-D can be removed | Both sensor interfaces must be characterized |
| B. Laptop MTS reference | Sensors and existing MTX-D | Innovate serial cable + laptop RS-232 interface | Independent calibration reference | Not part of the replacement gauge |

If laptop MTS capture exposes synchronized pressure and temperature, it can
validate Route A independently of visual readings.

## Route A — direct acquisition

```text
Switched vehicle 12 V
  ├── 2 A fuse near ACC
  ├── automotive TVS + reverse-polarity protection
  └── wide-input 5 V buck, at least 3 A
        └── Waveshare VBUS

Protected power ── characterized pressure excitation circuit
                  (voltage and implementation still unknown)

Waveshare 3V3 ── ADS1115 (bench module first; Q1 part on final PCB)
GPIO15 SDA ───── ADS1115 SDA
GPIO14 SCL ───── ADS1115 SCL
Star ground ──── ESP32 + ADC + sensors

Pressure signal conductor ── divider/protection/RC ── A0
Temperature gauge conductor ── selectable pull-up/RC ── A1
Pressure excitation monitor ── divider/protection/RC ── A2
Protected 12 V lighting input ───────────── A3
```

The supplied MTX-D diagram confirms a two-terminal temperature circuit and a
three-terminal pressure circuit. After characterization and only when the
MTX-D is removed, the temperature gauge conductor will connect to A1 at the
known precision pull-up node and its other conductor to star ground. The
pressure sensor will connect to the separately characterized excitation,
star ground, and A0 signal front end; A2 will monitor the excitation through
its own protected divider. Until measurements identify the two pressure
conductors, neither may be connected to excitation or an ADC channel.

While the MTX-D remains connected for characterization, do not add the new
temperature pull-up or any other bias source in parallel. Measure its existing
bias and the pressure excitation with a high-impedance meter first.

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

The acquisition task reuses the BSP's already-created I²C master bus, probes only
`0x48`, registers the ADS1115 at 100 kHz, and performs four 128-SPS single-shot
readings at PGA ±4.096 V. It publishes one coherent four-channel sample about every
96 ms and logs signed counts and ADC-pin volts. A1 additionally reports resistance
and a filtered provisional temperature when the source menu is set to `SENSORES`.
Pressure remains invalid, so a floating or uncalibrated A0 can never become oil data.

### Provisional bench front end

These values support characterization only; they are not a final PCB design.

| Channel | Provisional circuit | Purpose |
|---|---|---|
| A0 | 33 kΩ / 33 kΩ, 0.1%, 100 nF, low-leakage clamp | Pressure signal up to 5 V |
| A1 | 4.99 kΩ pull-up to 3.3 V, 0.1%; optional 100 nF | Provisional resistor/thermistor bench test |
| A2 | 33 kΩ / 33 kΩ, 0.1%, 100 nF | Excitation monitoring |
| A3 | 150 kΩ / 22 kΩ, 0.1%, 100 nF, clamp | Lighting detection |

For the candidate 0.5–4.5 V pressure hypothesis, the equal 33 kΩ / 33 kΩ
divider maps the nominal signal to 0.25–2.25 V. That intentionally gives the
16-bit ADC more voltage headroom than a 10 kΩ / 20 kΩ divider, whose output
would reach 3.333 V at a 5 V input and leave effectively no tolerance margin at
3.3 V VDD. The divider is still provisional until the real range is measured.

Measure the pressure signal minimum/maximum before connecting A0. The A1 bench
curve currently spans about 5458 Ω/10 °C to 80 Ω/140 °C with a 4.99 kΩ pull-up.
Measure sensor resistance only while unpowered and disconnected. Never parallel
the 3.3 V A1 pull-up with the powered MTX-D temperature input.

### Deferred automatic brightness

The Waveshare has no onboard ambient-light sensor. If true automatic brightness is
added later, the preferred final candidate is an automotive-qualified `OPT4001-Q1`
on the existing 3.3 V I²C bus. It can coexist with ADS1115 address `0x48` by selecting
and verifying a free address. Do not add another strong SDA/SCL pull-up pair on the
final PCB.

The protected A3 lighting input remains useful as an optional headlight/illumination
signal, but it is binary vehicle state rather than ambient lux. A true optical sensor
also requires a characterized enclosure window and a filtered, hysteretic brightness
mapping. Both routes are deferred; the v1 settings menu keeps manual brightness.

### Grounding

Innovate requires the pressure sensor's additional black wire to share the
gauge ground. Use a star point for sensor, ADC, ESP32, and converter input,
away from ignition, fuel pump, radio, alternator, and audio grounds.

## Laptop-only MTS calibration reference

```text
Sensors ── MTX-D OUT ── Innovate 38400 cable ── RS-232
                                                   │
                                                   ▼
                                      Laptop RS-232 port or
                                      proper USB-to-RS-232 adapter
```

Public MTS documentation suggests 19200 baud, 8N1, big-endian 16-bit words.
Treat that as a hypothesis. Capture this MTX-D's frames on the laptop and
compare with LogWorks. A TTL UART adapter such as FTDI, CP2102, or CH340 is not
an RS-232 adapter and must not be connected directly to MTX-D OUT.

This path is calibration equipment only. Production oil measurements use the
ADS1115 and the MTX-D can be removed after direct readings pass comparison.

## Display presentation

The hardware-accepted rollback base keeps the CO5300 in native `MADCTL=0x00`
scan order, native touch coordinates, two complete PSRAM buffers and LVGL `FULL`
mode. It is tear-free at approximately 30.5–32.7 completed presentations/s.

The current integration candidate keeps the same proven physical presentation
path but changes rendering to generation-aware `PARTIAL` mode:

- one 480×32 RGB565_SWAPPED LVGL draw buffer;
- three persistent complete 480×480 PSRAM canvases;
- 32×32 damage tiles plus an eight-generation history;
- exactly one coherent `READY` generation;
- `READY → IN_FLIGHT` only on the GPIO43 TE rising edge;
- immediate production of the next generation after that hand-off;
- immutable in-flight canvas until `on_color_trans_done`;
- full-frame physical QSPI transfer at TE, even when LVGL rendered only damaged
  blocks.

The full physical transfer is deliberate: this QSPI CO5300 owns internal GRAM
and cannot scan directly from an ESP32 framebuffer. Rendering fewer blocks saves
CPU/PSRAM work; sending one coherent frame preserves the no-tearing behavior of
the accepted base and avoids visible multi-window updates.

`esp_lvgl_adapter` 0.6.3 remains responsible for LVGL lifecycle, locking, timers
and touch, but the project deliberately does not use its display-registration
bridge. The display refresh timer is retained but held paused; project-owned
`lv_refr_now()` calls are the sole refresh authority. A guard immediately
neutralizes LVGL's built-in `LV_EVENT_REFR_REQUEST` timer resume, the missing
ownership rule that caused the first triple-buffer prototype to freeze.

Direct PSRAM DMA remains disabled because the exact board proved that path can
underflow at 80 MHz QSPI. ESP LCD instead stages bounded 8-row chunks through at
most 23,040 bytes of internal DMA memory. The exact panel measures about 59.5 TE
edges/s and 13–15 ms per complete transfer. The new PARTIAL candidate compiles
and passes native invariants, but remains **unflashed and not hardware-accepted**.

The detailed accepted path, rejected adapter mode and current candidate are maintained in
[`reference/display-pipeline.md`](reference/display-pipeline.md).

## Onboard warning audio

The synthetic demo uses the display board's existing ES8311 codec, I²S output
and integrated speaker. A renderer-independent edge gate starts a repeating
double-beep loop for as long as pressure remains in `warning`; the menu's
`PROBAR` control plays one double beep. A dedicated FreeRTOS CPU1-pinned task
performs 512-sample blocking PCM writes so the 15 ms UI producer never waits for
audio. The codec is opened and settled once, then remains unmuted at digital zero
between cues; each enveloped tone is wrapped in 40 ms of zero samples so its edges
do not toggle the analogue mute path. Initialization or write failure is
logged and degrades to a silent visual gauge rather than stopping the display.

The current pattern is approximately 2.2 kHz, 120 ms on, 90 ms off and 120 ms
on at the configured 5–100% codec volume. It is enabled only for the
calibration-safe demo. A
future calibrated vehicle alarm policy must be safety-reviewed separately; the
reported edge puff, physical loudness, cabin audibility, and post-change FPS remain
hardware checks for the revised image.

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
