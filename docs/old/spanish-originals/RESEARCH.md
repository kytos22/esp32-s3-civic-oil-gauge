# Investigación y fuentes

Fecha de corte: **2026-07-28**.

## Hechos confirmados

### Innovate MTX-D

- Modelo actual: P/N 39130.
- Formato: 2 1/16" (52 mm).
- Rango mostrado: 0–145 PSI / 10 bar y 120–280 °F / 49–138 °C.
- Sensor de presión de recambio: P/N 12-0074, 0–150 PSI / 10 bar.
- Termistor: P/N 15-0049.
- Mazo de presión: P/N 08-0256C.
- Cable de programación: P/N 38400.
- Alimentación: 12 V conmutados, fusible mínimo de 2 A y masa limpia.
- La entrada blanca se conecta a 12 V de luces, no al reóstato.
- El sensor de presión y el de temperatura son 1/8 NPT.
- Innovate advierte no montar el sensor de presión directamente junto a
  bomba/interruptor por las pulsaciones.
- OUT permite registro MTS/LogWorks.

Fuentes:

- https://www.innovatemotorsports.com/mtx-d-oil-pressure-temperature.html
- https://www.innovatemotorsports.com/wp/content/uploads/2022/05/MTX-D-Oil-Press-Temp.pdf
- https://www.innovatemotorsports.com/sensor-pressure-0-150-psi-10-bar-for-mtx-d-ecf-1.html
- https://www.innovatemotorsports.com/program-cable-mtx-series-gauges-lm-2-lc-2-scg-1-psb-1-and-psn-1.html
- https://www.innovatemotorsports.com/wp/content/uploads/2022/08/LogWorks3_Manual.pdf

### Waveshare ESP32-S3-Touch-AMOLED-2.16

- ESP32-S3R8, 8 MB PSRAM y 16 MB flash.
- AMOLED 480 × 480, CO5300 QSPI y CST9220 táctil I²C.
- AXP2101, QMI8658, PCF85063, ES8311 y ES7210 integrados.
- Pads expuestos VBUS, 3V3, GND, USB, I²C y UART.
- Carcasa: 46 × 46 mm; grosor aproximado 8.3 mm.
- Repositorio oficial consultado: commit
  `713f8bdcc0fc2356ac22335ed4f381096e45ceea`.
- Matriz oficial de compilación: ESP-IDF 5.5.4/6.0.2 y
  Arduino-ESP32 3.3.10.
- El proyecto usa Arduino_GFX 1.6.7 porque la distribución pública 1.6.4
  no compila con Arduino-ESP32 3.3.11; Waveshare mantiene correcciones
  equivalentes en su copia empaquetada de 1.6.4.
- BSP oficial disponible en el registro Espressif, versión 2.0.1 al
  consultar.

Fuentes:

- https://www.waveshare.com/esp32-s3-touch-amoled-2.16.htm
- https://docs.waveshare.com/ESP32-S3-Touch-AMOLED-2.16
- https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-2.16
- https://files.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-2.16/ESP32-S3-Touch-AMOLED-2.16-Schematic.pdf
- https://components.espressif.com/components/waveshare/esp32_s3_touch_amoled_2_16

### ADC y alimentación

- ADS1115-Q1: cuatro canales single-ended, 16 bits, 8–860 SPS, PGA,
  referencia interna, I²C, AEC-Q100 grado 1.
- Con VDD=3.3 V, una entrada analógica no puede superar VDD+0.3 V aunque
  el PGA muestre una escala mayor.
- Una red de 12 V de automóvil requiere contemplar sobretensión,
  sobrecarga, polaridad inversa, arranque con pinzas y load dump.

Fuentes:

- https://www.ti.com/product/ADS1115-Q1
- https://www.ti.com/lit/ds/symlink/ads1115-q1.pdf
- https://www.ti.com/tool/TIDA-01167
- https://www.ti.com/product/LM76003-Q1
- https://www.littelfuse.com/products/overvoltage-protection/tvs-diodes/automotive-tvs-diodes/sld8s/sld8s24a

## Datos que siguen abiertos

1. Pinout real y tensión de excitación del 12-0074.
2. Función de transferencia de presión.
3. Curva R/T y tolerancia del 15-0049.
4. Tipo exacto de los conectores del mazo instalado.
5. Contenido y orden de canales MTS del MTX-D concreto.
6. Consumo pico de la Waveshare con el UI definitivo.
7. Temperatura real dentro del soporte del reloj.

Ninguno de estos puntos se resolverá por suposición; todos tienen una
medición prevista en `CALIBRATION.md` o `ARRIVAL_CHECKLIST.md`.

## Validación del proyecto base

El 2026-07-28 se compiló correctamente con PlatformIO/pioarduino
`55.03.311`, Arduino-ESP32 `3.3.11`, Arduino_GFX `1.6.7` y
Adafruit ADS1X15 `2.6.2`, seleccionando explícitamente la variante
ESP32-S3 N16R8. Las ocho pruebas nativas del núcleo de conversión pasaron.
Esta validación demuestra que el software construye; la pantalla, el
consumo y las entradas analógicas siguen pendientes de prueba física.
