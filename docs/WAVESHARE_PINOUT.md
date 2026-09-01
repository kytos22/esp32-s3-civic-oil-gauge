# Verified Pinout — ESP32-S3-Touch-AMOLED-2.16

Source: official Waveshare schematic, silkscreen, and pinned BSP 2.0.1 source,
reviewed again against the physically wired board on 2026-08-25.

## Exposed pads

| Pad | Signal |
|---|---|
| P1 | VBUS |
| P2 | 3V3 |
| P3 | GND |
| P4 | GPIO19 / USB D− |
| P5 | GPIO20 / USB D+ |
| P6 | GPIO14 / I²C SCL |
| P7 | GPIO15 / I²C SDA |
| P8 | GPIO43 / display TE input (wired from panel TP3) |
| P9 | GPIO44 / CivicAux UART1 RX only |

Project use:

- ADS1115: P2, P3, P6, P7.
- Display synchronization: panel TP3 → P8/GPIO43; do not reuse it as UART TX.
- CivicAux one-way UART input: Hub GPIO17/TX → measured approximately 326 Ω
  series resistance → P9/GPIO44. Configure UART1 at 115200 8N1 with no TX.
- Hub GND and oil-display GND are joined (approximately 0.1 Ω measured); their
  3.3 V and 5 V rails remain separate.
- MTS is laptop-only calibration equipment; it is not connected to a board pin.
- Vehicle power: regulated 5 V to P1/GND, never 12 V.
- The board has no onboard ambient-light sensor. A future external sensor may share
  P2/P3/P6/P7 only after its I²C address and pull-up loading are verified.

## Display and touch

| Function | GPIO |
|---|---:|
| CO5300 SDIO0 | 4 |
| CO5300 SDIO1 | 5 |
| CO5300 SDIO2 | 6 |
| CO5300 SDIO3 | 7 |
| CO5300 SCLK | 38 |
| CO5300 CS | 12 |
| CO5300 RESET | 39 |
| CST9220 interrupt | 11 |
| CST9220 reset | 40 |
| I²C SDA | 15 |
| I²C SCL | 14 |

## Known I²C addresses

| Device | Address |
|---|---:|
| AXP2101 | 0x34 |
| ES7210 | 0x40 |
| project ADS1115 | 0x48 |
| PCF85063 | 0x51 |
| CST9220 | 0x5A |
| QMI8658 | 0x6B |

Bench diagnostic wiring uses P2/P3/P6/P7 only: 3V3, GND, SCL GPIO14 and SDA
GPIO15. `ADDR` is tied to GND for `0x48`; ALERT and A0–A3 remain disconnected for
the first probe. The onboard 2.2 kΩ pull-ups remain authoritative.

The board has **2.2 kΩ pull-ups to 3.3 V** on SDA/SCL. A commercial breakout
may add pull-ups during bench work if the bus remains healthy, but the final
PCB must not add another strong pair.

## Other occupied GPIOs

- microSD: GPIO1, 2, 3, 41;
- audio: GPIO8, 9, 10, 42, 45, 46;
- programmable button: GPIO18;
- RTC/IMU interrupts: GPIO13, 17, 21;
- native USB: GPIO19, 20.

Do not reassign these pins without reviewing the schematic.

## Integrated audio output

| Function | GPIO / bus |
|---|---:|
| ES8311 control | shared I²C on SDA 15 / SCL 14 |
| I²S data out to ES8311 | 8 |
| I²S bit clock | 9 |
| I²S master clock | 42 |
| I²S left/right clock | 45 |
| Speaker power-amplifier enable | 46 |

The firmware uses `bsp_audio_codec_speaker_init()` and `esp_codec_dev`; GPIO46
is only the amplifier-enable line and is not driven as if the speaker were a
passive buzzer. The microphone data input on GPIO10 is not used by the gauge.
