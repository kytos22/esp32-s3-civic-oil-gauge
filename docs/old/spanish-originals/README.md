# ESP32 Oil Gauge para Honda Civic

Sustitución visual del Innovate Motorsports MTX-D de presión/temperatura de
aceite por una Waveshare **ESP32-S3-Touch-AMOLED-2.16**, conservando los
sensores Innovate ya instalados.

## Estado

- La pantalla todavía no ha llegado.
- El hardware oficial de Waveshare, sus pads y sus medidas ya están
  documentados.
- El firmware arranca deliberadamente en modo demostración.
- No hay ninguna curva de sensor inventada: Innovate no publica la función
  tensión→presión ni la curva resistencia→temperatura.
- La calibración se hará con el MTX-D conectado como referencia antes de
  retirarlo.

## Arquitectura elegida

El objetivo final es:

```text
Sensores Innovate
      │
      ▼
protección + adaptación analógica
      │
      ▼
ADS1115-Q1 (I²C, 0x48)
      │ GPIO15 SDA / GPIO14 SCL
      ▼
Waveshare ESP32-S3-Touch-AMOLED-2.16
```

Durante el desarrollo se mantendrá el MTX-D operativo para obtener pares
de referencia y validar las curvas. También se investigará su salida MTS
RS-232 como alternativa: conservar el acondicionamiento original oculto y
usar el ESP32 únicamente como nueva pantalla.

## Por dónde empezar

1. [Lista de compra](../../BOM.md)
2. [Arquitectura y decisiones](../../ARCHITECTURE.md)
3. [Pinout verificado de Waveshare](../../WAVESHARE_PINOUT.md)
4. [Caracterización y calibración](../../CALIBRATION.md)
5. [Comprobaciones cuando llegue la pantalla](../../ARRIVAL_CHECKLIST.md)
6. [Fuentes y hechos confirmados](../../RESEARCH.md)
7. [Diseño aprobado de la interfaz de aceite](../../UI_DESIGN.md)

## Firmware

El proyecto base usa PlatformIO con Arduino-ESP32, siguiendo el ejemplo
Arduino oficial de Waveshare. En ausencia de calibraciones válidas:

- muestra `DEMO` con valores simulados, o
- muestra únicamente voltajes ADC crudos si detecta un ADS1115.

Nunca presenta voltajes sin calibrar como PSI o grados reales.

Validación realizada el 2026-07-28:

- compilación completa correcta para 16 MB flash / 8 MB PSRAM;
- ocho pruebas unitarias correctas para divisores, termistor, calibración,
  filtrado y estados de alarma;
- compilación estricta del núcleo matemático sin avisos.

Comandos:

```bash
pio run
pio test -e native
```

No se debe flashear ni conectar al coche hasta completar la lista de
comprobación de llegada.

## Advertencia eléctrica

La Waveshare acepta 5 V por VBUS/USB-C, **no 12 V del coche**. La
alimentación final requiere fusible, protección frente a transitorios y
polaridad inversa, y un convertidor 12 V→5 V adecuado. No conectar a la vez
USB y una fuente externa de 5 V sin verificar antes que no exista retorno
de corriente.
