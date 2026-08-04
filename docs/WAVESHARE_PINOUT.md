# Verified Pinout — ESP32-S3-Touch-AMOLED-2.16

Source: official Waveshare schematic, silkscreen, and Arduino example reviewed
on 2026-07-28.

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
| P8 | GPIO43 / UART TX |
| P9 | GPIO44 / UART RX |

Project use:

- ADS1115: P2, P3, P6, P7.
- Optional MTS: P3 and P9 through an RS-232 receiver.
- Vehicle power: regulated 5 V to P1/GND, never 12 V.

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
