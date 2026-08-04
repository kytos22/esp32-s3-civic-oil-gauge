# Pinout verificado: ESP32-S3-Touch-AMOLED-2.16

Fuente: esquema, serigrafía y ejemplo Arduino oficiales de Waveshare,
revisión consultada el 2026-07-28.

## Pads accesibles

| Pad | Señal |
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

Para el proyecto:

- ADS1115: P2, P3, P6 y P7.
- MTS opcional: P3 y P9 a través de un receptor RS-232.
- Alimentación de coche: 5 V regulados a P1 y GND, nunca 12 V.

## Display y táctil

| Función | GPIO |
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

## Direcciones I²C conocidas

| Dispositivo | Dirección |
|---|---:|
| AXP2101 | 0x34 |
| ES7210 | 0x40 |
| PCF85063 | 0x51 |
| CST9220 | 0x5A |
| QMI8658 | 0x6B |
| ADS1115 del proyecto | 0x48 |

El esquema muestra pull-ups de **2.2 kΩ a 3.3 V** en SDA/SCL. Un breakout
comercial puede traer pull-ups adicionales; se tolerarán en banco si el
bus funciona, pero la PCB final no añadirá otros pull-ups.

## Otros GPIO ocupados

- microSD: GPIO1, 2, 3 y 41;
- audio: GPIO8, 9, 10, 42, 45 y 46;
- botón programable: GPIO18;
- RTC/IMU interrupts: GPIO13, 17 y 21;
- USB nativo: GPIO19 y 20.

No reasignar estos pines sin revisar el esquema.
