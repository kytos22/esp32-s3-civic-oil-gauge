# Research and Sources

Evidence cutoff: **2026-08-14**. Current purchasing/status information can drift;
reverify before ordering.

## Confirmed facts

### Innovate MTX-D

- Current model: P/N 39130, 52 mm (2 1/16 inch).
- Display range: 0–145 PSI / 10 bar and 120–280 °F / 49–138 °C.
- Replacement pressure sensor: P/N 12-0074, 0–150 PSI / 10 bar.
- Thermistor: P/N 15-0049.
- Pressure harness: P/N 08-0256C.
- Programming cable: P/N 38400.
- Switched 12 V supply, minimum 2 A fuse, clean ground.
- White lighting input connects to 12 V lighting, not a dimmer rheostat.
- Pressure and temperature sensors are 1/8 NPT.
- Innovate warns against mounting the pressure sensor directly next to a
  pump/switch where pulsation is severe.
- OUT supports MTS/LogWorks logging.

### Pressure transfer-function evidence

- The official `12-0074` product page confirms only P/N, 0–150 PSI / 10 bar,
  and MTX-D/ECF-1 compatibility in its public text.
- Innovate document `11-0161A` describes a separate "10 BAR (150 PSI)
  Pressure Sensor with SSI-4 PLUS Adapter": red to the SSI-4 PLUS 5 V terminal,
  black to ground, white to channel positive, and a three-pin sensor connector.
  It gives `0 PSI = 0.5 V` and `150 PSI = 4.5 V`.
- No consulted official source explicitly states that the sensor in `11-0161A`
  is P/N `12-0074`, that both use the same internal transducer, or which physical
  pins/08-0256C colours carry excitation and signal. The matching range makes
  `5 V / 0.5–4.5 V` the leading hypothesis, not a confirmed 12-0074 calibration.

Sources:

- [MTX-D product](https://www.innovatemotorsports.com/mtx-d-oil-pressure-temperature.html)
- [MTX-D manual](https://www.innovatemotorsports.com/wp/content/uploads/2022/05/MTX-D-Oil-Press-Temp.pdf)
- [Pressure sensor](https://www.innovatemotorsports.com/sensor-pressure-0-150-psi-10-bar-for-mtx-d-ecf-1.html)
- [11-0161A 10 bar sensor with SSI-4 PLUS adapter](https://www.innovatemotorsports.com/wp/content/uploads/2022/05/11-0161A-10-BAR-Pressure-Sensor_2pg.pdf)
- [Programming cable](https://www.innovatemotorsports.com/program-cable-mtx-series-gauges-lm-2-lc-2-scg-1-psb-1-and-psn-1.html)
- [LogWorks manual](https://www.innovatemotorsports.com/wp/content/uploads/2022/08/LogWorks3_Manual.pdf)

### Waveshare ESP32-S3-Touch-AMOLED-2.16

- ESP32-S3R8, 8 MB PSRAM, 16 MB flash.
- 480×480 CO5300 QSPI AMOLED and CST9220 I²C touch.
- Integrated AXP2101, QMI8658, PCF85063, ES8311, and ES7210.
- Exposed VBUS, 3V3, GND, USB, I²C, and UART pads.
- Enclosure approximately 46×46×8.3 mm.
- Official repository snapshot consulted: `713f8bdcc0fc2356ac22335ed4f381096e45ceea`.
- Official matrix listed ESP-IDF 5.5.4/6.0.2 and Arduino-ESP32 3.3.10.
- The 2026-07-28 scaffold used Arduino_GFX 1.6.7; that is historical only.
- Current firmware uses the official native Waveshare BSP 2.0.1 with ESP-IDF
  6.0.2 and LVGL 9.5.0, all exact-pinned in the component manifest/lock.
- Official Espressif component registry BSP observed at version 2.0.1.

Sources:

- [product](https://www.waveshare.com/esp32-s3-touch-amoled-2.16.htm)
- [wiki](https://docs.waveshare.com/ESP32-S3-Touch-AMOLED-2.16)
- [official repository](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-2.16)
- [schematic](https://files.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-2.16/ESP32-S3-Touch-AMOLED-2.16-Schematic.pdf)
- [Espressif component](https://components.espressif.com/components/waveshare/esp32_s3_touch_amoled_2_16)

### Ambient-light sensing for automatic brightness

- The Waveshare onboard-resource list and complete three-page schematic contain no
  ambient-light sensor. The display therefore cannot measure cabin illumination by
  itself.
- A true digital ambient-light sensor can share the exposed 3.3 V I²C bus on GPIO14
  and GPIO15 with the ADS1115. The final board must reuse the Waveshare's existing
  2.2 kΩ pull-ups rather than adding another strong pair.
- Preferred final-vehicle candidate: TI `OPT4001-Q1`. It is AEC-Q100 qualified,
  operates from 1.6–3.6 V, has selectable I²C addressing, human-eye spectral
  response with infrared rejection, and supports automatic-ranging measurements.
  A non-conflicting address must be fixed in the final schematic and verified by an
  I²C scan.
- Easier bench candidate: Vishay `VEML7700`, powered at 3.3 V over I²C. It measures
  approximately 0–140 klx and rejects 100/120 Hz lighting flicker, but its cited
  datasheet does not provide the `-Q1` automotive qualification required for the
  final in-car PCB.
- The already reserved protected lighting input on ADS1115 A3 is a separate binary
  alternative: it can report that the vehicle illumination circuit is energized,
  but it does not measure actual ambient lux and cannot react correctly to every
  tunnel, shadow, glare, or daytime-headlights case.
- Any optical sensor needs a clear or characterized dark window facing cabin/
  windshield light, not an opaque enclosure. Firmware must use filtering, hysteresis,
  a minimum dwell time, gradual brightness ramps, and a manual brightness fallback so
  passing shadows do not make the AMOLED pump visibly.
- Automatic brightness and manual `DÍA`/`NOCHE` presets remain deferred. No light
  sensor is added to the current BOM until packaging and the final analog PCB are
  selected.

Sources:

- [Waveshare board resources](https://docs.waveshare.com/ESP32-S3-Touch-AMOLED-2.16)
- [Waveshare schematic](https://files.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-2.16/ESP32-S3-Touch-AMOLED-2.16-Schematic.pdf)
- [TI OPT4001-Q1](https://www.ti.com/product/OPT4001-Q1)
- [TI automotive-display ambient-light note](https://www.ti.com/lit/ab/sboa359/sboa359.pdf)
- [Vishay VEML7700](https://www.vishay.com/en/product/84286/)

### ADC and automotive supply

- ADS1115-Q1: four single-ended channels, 16 bits, 8–860 SPS, PGA, internal
  reference, I²C, AEC-Q100 Grade 1.
- At VDD=3.3 V, an analog input must not exceed VDD+0.3 V even if the PGA
  full-scale range is larger.
- A vehicle 12 V network must account for overvoltage, overload, reverse
  polarity, jump start, and load dump.

Sources:

- [ADS1115-Q1](https://www.ti.com/product/ADS1115-Q1)
- [datasheet](https://www.ti.com/lit/ds/symlink/ads1115-q1.pdf)
- [TI automotive reference](https://www.ti.com/tool/TIDA-01167)
- [LM76003-Q1](https://www.ti.com/product/LM76003-Q1)
- [SLD8S24A](https://www.littelfuse.com/products/overvoltage-protection/tvs-diodes/automotive-tvs-diodes/sld8s/sld8s24a)

## Open measurements

1. Real P/N 12-0074 pinout and excitation; test the official adjacent-sensor
   `5 V` hypothesis without assigning pins from colour.
2. Confirm or reject the candidate `0.5–4.5 V = 0–150 PSI` transfer function.
3. P/N 15-0049 R/T curve and tolerance.
4. Exact installed harness connectors.
5. MTX-D MTS channel content/order.
6. Waveshare peak current with final UI.
7. Enclosure temperature in the vehicle mount.
8. Evidence-backed source and semantics for engine-running/RPM state.
9. Ambient-light sensor optical placement, cover-window transmission, lux thresholds,
   hysteresis, and brightness mapping if automatic brightness is selected later.

None is resolved by assumption; each has a procedure in `CALIBRATION.md` or
`ARRIVAL_CHECKLIST.md`.

## Historical software validation

On 2026-07-28 the project recorded a successful build with pioarduino
55.03.311, Arduino-ESP32 3.3.11, Arduino_GFX 1.6.7, Adafruit ADS1X15 2.6.2,
and explicit ESP32-S3 N16R8 selection. Eight native conversion tests passed.

This proves only that the recorded software snapshot built then. Physical
display, current, analog inputs, and present-session reproducibility remain
unverified.
