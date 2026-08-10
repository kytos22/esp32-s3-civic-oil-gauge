# Reloj de aceite para Honda Civic con ESP32-S3

**Español** | [English](README.md)

Pantalla de presión y temperatura de aceite para la Waveshare
ESP32-S3-Touch-AMOLED-2.16 de 480×480 píxeles. El proyecto pretende sustituir
únicamente el reloj visible Innovate Motorsports MTX-D Oil Pressure/Temperature,
manteniendo los sensores ya instalados y un adaptador reversible en el vehículo.

> [!WARNING]
> Las curvas de los sensores todavía no están calibradas. El firmware actual
> funciona únicamente con valores sintéticos claramente identificados como
> `DEMO`. No debe utilizarse como protección del motor ni sustituir todavía al
> MTX-D instalado.

[![Demo animada 50/50 del reloj de aceite — abrir el simulador interactivo](assets/oil-gauge-demo.gif)](https://kytos22.github.io/esp32-s3-civic-oil-gauge/design/references/oil-gauge-design.html)

### [▶ Abrir el simulador interactivo del reloj de aceite](https://kytos22.github.io/esp32-s3-civic-oil-gauge/design/references/oil-gauge-design.html)

La vista previa representa estados continuos con interpolación suave a 50 FPS.
Ajusta la presión de aceite, las RPM del motor y la temperatura mediante los
sliders. GitHub no permite ejecutar JavaScript dentro del README, por lo que el
GIF y este enlace abren el simulador publicado con GitHub Pages.

## Estado actual

- Firmware nativo ESP-IDF 6.0.2 con el BSP oficial de Waveshare 2.0.1 y
  LVGL 9.5.0.
- Inicialización de pantalla y panel táctil comprobada en el hardware real.
- Demo determinista y continua medida durante ocho ventanas consecutivas entre
  65 y 67 FPS completados en la placa de destino.
- Zonas iguales 50/50 para presión y temperatura sobre fondo AMOLED negro puro.
- Estados semánticos en español de 24 px, valores principales centrados, barras
  de nueve píxeles y aviso parpadeante de presión baja.
- Catorce pruebas Unity independientes del hardware superadas.
- La calibración directa, el adaptador reversible de Innovate y la validación en
  el vehículo siguen intencionadamente pendientes.

La situación detallada y las barreras de seguridad pendientes se mantienen en
[`docs/PROGRESS.md`](docs/PROGRESS.md).

## Características

- Presión de aceite en PSI y temperatura de aceite en grados Celsius.
- Estados explícitos de frío, calentando, óptimo, caliente y muy caliente.
- El aviso de presión baja depende del estado del motor; un motor parado no
  genera una falsa alarma.
- Un doble pitido no bloqueante por el altavoz integrado cuando la demo entra
  en aviso de presión baja; no se repite mientras el aviso siga activo.
- Por debajo del rango útil del sensor se muestra `<50` en lugar de inventar
  precisión.
- El significado de los avisos nunca depende únicamente del color.
- El modo demo seguirá siendo el valor predeterminado hasta disponer de una
  calibración medida.
- Una calibración ausente o inválida produce un fallo visible, nunca valores de
  ingeniería falsos.

## Hardware

- Waveshare ESP32-S3-Touch-AMOLED-2.16 con AMOLED de 480×480 píxeles.
- Instalación existente Innovate MTX-D Oil Pressure/Temperature.
- ADC externo compatible con ADS1115-Q1 a 3,3 V previsto para la ruta de lectura
  directa.
- Fuente protegida de 12 V a 5 V para automoción y mazo reversible antes de usar
  el sistema en el vehículo.

El mapa de pines está en [`docs/WAVESHARE_PINOUT.md`](docs/WAVESHARE_PINOUT.md),
la lista de compra en [`docs/BOM.md`](docs/BOM.md) y las dos rutas de señal
propuestas en [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).

## Seguridad y estado del cableado

- Nunca conectes los 12 V del vehículo directamente a VBUS, 3V3 o un GPIO.
- No supongas que el sensor de presión de Innovate entrega 0,5–4,5 V ni que el
  sensor de temperatura es un NTC de 10 kΩ.
- No asignes funciones a partir del color de los cables.
- No cortes el mazo de Innovate.
- Mantén el MTX-D conectado mientras se recopilan referencias en frío, caliente
  y a distintas RPM.

El bus I²C expuesto de la placa utiliza SDA GPIO15 y SCL GPIO14. Ya incorpora
resistencias pull-up de 2,2 kΩ a 3,3 V; el ADS1115 previsto utiliza la dirección
`0x48`. Estos datos no identifican ningún cable de los sensores Innovate. Sigue
el procedimiento de [`docs/CALIBRATION.md`](docs/CALIBRATION.md) antes de
conectar señales de los sensores.

## Descargas de firmware

Todavía no existe una versión de producción. Las versiones validadas se
guardarán en [`firmware/<versión>/`](firmware/README.md) con imágenes de
aplicación y flash completo, instrucciones bilingües, evidencias visuales y
sumas SHA-256.

Los binarios de desarrollo de `build/` se ignoran intencionadamente y no deben
publicarse como versiones.

## Compilación

Utiliza los envoltorios del proyecto desde WSL; fijan la cadena de herramientas
conocida y aíslan la caché de pruebas de PlatformIO de otros proyectos.

Compilar el firmware ESP-IDF completo:

```bash
./scripts/idf build
```

Ejecutar las pruebas nativas de medición y demo:

```bash
./scripts/pio test -e native
```

Ejecutar las comprobaciones del repositorio y de seguridad:

```bash
./scripts/keel-verify
```

El flasheo es una operación de hardware independiente. Verifica la placa exacta
y obtén autorización explícita antes de escribirla.

## Organización del repositorio

| Ruta | Finalidad |
| --- | --- |
| `src/` | Entrada ESP-IDF, renderizador LVGL, fuentes y lógica de medición |
| `include/` | Interfaces públicas de medición, calibración, pines y demo |
| `test/` | Pruebas nativas Unity |
| `docs/` | Arquitectura, seguridad, calibración, diseño y evidencias |
| `assets/` | Recursos visuales propios y vistas previas aprobadas |
| `firmware/` | Únicamente paquetes versionados y validados |
| `scripts/` | Puntos de entrada fijados para compilación, pruebas y consistencia |

La organización adopta las convenciones públicas útiles del repositorio
complementario del reloj de turbo, pero no copia firmware, datos de sensores,
ajustes, diagramas, cachés generadas ni recursos del turbo. Intencionadamente no
se utiliza un mecanismo de «golden version»; la reproducibilidad se basa en
dependencias fijadas, pruebas, evidencias de aceptación documentadas y hashes de
cada versión.

## Guía de desarrollo

Los colaboradores y agentes de programación deben leer [`AGENTS.md`](AGENTS.md)
antes de modificar el renderizador, los tiempos de pantalla, las barreras de
calibración, los pines, el empaquetado o las pruebas de hardware. El índice de la
documentación mantenida está en [`docs/INDEX.md`](docs/INDEX.md).

## Licencia

El código, la documentación y los recursos propios del proyecto se ofrecen bajo
la [licencia PolyForm Noncommercial 1.0.0](LICENSE.md). Puedes estudiar,
modificar y redistribuir el proyecto y tus mejoras para los usos no comerciales
permitidos. El uso comercial requiere un permiso independiente del titular de
los derechos. Es software con código disponible, no «open source» según OSI.

Conserva el aviso obligatorio incluido en [`NOTICE`](NOTICE). Las dependencias y
herramientas de terceros mantienen sus propias licencias.

## Proyecto independiente

Los nombres y marcas Honda, Civic, Innovate Motorsports y Waveshare pertenecen a
sus respectivos propietarios. Este es un proyecto independiente de aficionado y
no está afiliado ni respaldado por dichas empresas.
