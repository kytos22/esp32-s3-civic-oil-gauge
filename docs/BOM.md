# Bill of Materials

Purchasing is staged so no final PCB is built around sensor curves that remain unknown.

## Buy or locate now

| Qty | Item | Recommended reference | Purpose |
|---:|---|---|---|
| 1 | Bench ADS1115 ADC | Adafruit Product 1085 or a reputable module with genuine TI ADS1115 | Capture direct sensor signals |
| 1 | 0–100 kΩ decade box | 0.1% preferred; 1% acceptable for discovery | Map thermistor input |
| 1 lot | 0.1%, ≤25 ppm/°C resistors | 1 k, 2.49 k, 4.99 k, 10 k, 22 k, 33 k, 47 k, 100 k, 150 k | Dividers and pull-ups |
| 1 lot | X7R capacitors | 100 nF, 1 µF, 10 µF | Filtering and decoupling |
| 1 | Solderless breadboard + short jumpers | 2.54 mm | Initial characterization |
| 1 | Solderable protoboard | 2.54 mm | Stable bench front end |
| 1 lot | Screw terminals, headers, Dupont housings | 2.54/5.08 mm | Reversible bench wiring |
| 1 | Short certified USB-C data cable | — | Programming and measurements |
| 1 | Branded 5 V / 3 A USB supply | — | Bench power |
| 1 | High-impedance multimeter | DC V, Ω, continuity | Pin and curve identification |
| 1 | Fine insulated back-probe set | No insulation piercing | Measure with MTX-D connected |
| 1 | Reference contact thermometer/thermocouple | — | Temperature validation |
| 1 | Shielded four-core cable | 0.22–0.35 mm² | ADC/sensor signals |
| 1 lot | Heat-shrink, braid, ferrules, automotive terminals | — | Protected reversible harness |

## Innovate parts that should already exist

Verify before ordering:

| Part | Innovate P/N | Action |
|---|---:|---|
| MTX serial programming cable | 38400 | Locate for LogWorks/MTS |
| Gauge-to-pressure harness | 08-0256C | Keep intact |
| 0–150 PSI pressure sensor | 12-0074 | Already installed |
| Thermistor | 15-0049 | Already installed |

If the computer has no native RS-232 port, use Innovate USB-serial P/N 37330
or a quality compatible **USB-to-RS-232** adapter. Do not substitute a
TTL-UART FTDI/CP2102/CH340 adapter. The laptop serial path is calibration
equipment and no MAX3232E/TRS3232E is required in the ESP32 gauge.

## Vehicle-prototype power

| Qty | Item | Reference | Note |
|---:|---|---|---|
| 1 | 12 V→5 V buck | Pololu D36V28F5, #3782 | 5 V/3.2 A, 5.3–50 V, reverse protection; prototype |
| 1 | Inline mini/ATO fuse holder | Automotive | Near ACC |
| 3 | 2 A mini/ATO fuses | — | One installed, two spares |
| 1 | Automotive load-dump TVS | Littelfuse SLD8S24A | After fuse; coordination still required |
| 1 | Power perfboard/PCB + enclosure | To select | Isolate TVS, buck, and terminals |

The Pololu module is a prototype platform, not an automotive qualification.

## Useful optional equipment

- Logic analyzer or oscilloscope for MTS level/frame verification.
- Spare Innovate 15-0049 sensor for a controlled bath.
- 0–10 bar pressure calibrator and reference gauge.
- Spare 08-0256C harness for a no-cut adapter.
- USB-C current meter.

## Do not buy yet

- Lookalike Innovate connectors before photographs, measurements, and keying are known.
- Loose ADS1115-Q1, final clamp parts, or a custom PCB before measurements.
- Final fixed thermistor pull-up.
- 52 mm printed mount/inserts before measuring the board and vehicle support.
- LiPo battery; the gauge is powered from ACC.

## Official data/purchase sources

- [Adafruit ADS1115](https://www.adafruit.com/product/1085)
- [TI ADS1115-Q1](https://www.ti.com/product/ADS1115-Q1)
- [Pololu D36V28F5](https://www.pololu.com/product/3782)
- [Littelfuse SLD8S24A](https://www.littelfuse.com/products/overvoltage-protection/tvs-diodes/automotive-tvs-diodes/sld8s/sld8s24a)
- [Innovate MTX-D accessories](https://www.innovatemotorsports.com/mtx-d-oil-pressure-temperature.html)

The machine-readable list remains in `docs/bom.csv`.
