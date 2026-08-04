# Diseño de la interfaz de aceite

Estado: **referencia aprobada para implementar más adelante**.

La referencia visual editable está en
[`design/references/oil-gauge-design.fragment.html`](../../design/references/oil-gauge-design.fragment.html).
La versión autónoma para abrir en un navegador está en
[`design/references/oil-gauge-design.html`](../../design/references/oil-gauge-design.html) y la
captura estática en
[`design/references/oil-gauge-design.png`](../../design/references/oil-gauge-design.png).

## Composición

- Lienzo objetivo: 480 × 480 píxeles.
- Fondo negro puro para apagar los píxeles del AMOLED.
- División vertical 50/50:
  - presión de aceite en la mitad superior;
  - temperatura de aceite en la mitad inferior.
- El número queda centrado respecto al eje horizontal completo. El icono a la
  izquierda y la unidad a la derecha no deben desplazarlo.
- Las barras tienen 9 píxeles de grosor en el lienzo de 480 × 480.
- Las cifras permanecen blancas. Etiquetas, unidades y referencias secundarias
  usan blanco atenuado.
- Las RPM no aparecen nunca en la pantalla. Solo se usan como entrada interna
  de la lógica de presión.

## Iconos

Usar símbolos automotrices compactos, de trazo grueso y elementos rellenos:

- presión: aceitera ancha con tapón, pico y gota separada;
- temperatura: termómetro sólido con tres marcas laterales y dos ondas de
  aceite debajo.

Los SVG del prototipo son la referencia geométrica. En firmware se podrán
convertir a paths, polígonos o bitmaps monocromos conservando su silueta.

## Presión de aceite

Escala visual provisional: 0–150 PSI.

| Condición | Estado | Color/comportamiento |
|---|---|---|
| RPM = 0 | `MOTOR PARADO` | sin alarma |
| 0–10 PSI y RPM > 0 | `WARNING` | rojo y parpadeo a 1 Hz |
| 11–14 PSI y RPM > 0 | `PRESIÓN BAJA` | provisional, pendiente de confirmar |
| 15–80 PSI | `OK` | amarillo/ámbar |
| >80 PSI | `PRESIÓN ALTA` | provisional, pendiente de confirmar |

Color normal de icono y barra: `rgb(255, 176, 32)`.
Color de alarma: `rgb(255, 57, 72)`.

Durante `WARNING` parpadean el icono, el texto y la barra; la cifra numérica
permanece fija y legible. Si se deshabilitan animaciones, todos esos elementos
quedan en rojo fijo.

## Temperatura de aceite

El rango mostrado por el MTX-D empieza aproximadamente en 49 °C. Hasta disponer
de una curva propia validada:

- si el valor está por debajo de 50 °C, mostrar `<50 °C`;
- mantener el icono azul y la barra sin rellenar por debajo de 50 °C;
- no presentar un número preciso por debajo del rango validado.

Estados semánticos actuales:

| Temperatura | Estado |
|---|---|
| <70 °C | `FRÍO` |
| 70–74 °C | `CALENTANDO` |
| 75–93 °C | `ÓPTIMO` |
| 94–100 °C | `CALIENTE` |
| >100 °C | `MUY CALIENTE` |

La barra, el icono y el nombre del estado comparten una transición continua:

| Punto | RGB | Uso |
|---:|---|---|
| 50 °C | `30, 132, 255` | azul frío |
| 57 °C | `30, 132, 255` | comienza la transición |
| 75 °C | `174, 205, 167` | verde claro y desaturado |
| 89 °C | `174, 205, 167` | final del verde estable |
| 94 °C | `234, 190, 82` | amarillo/ámbar |
| 100 °C | `255, 118, 28` | naranja |
| 138 °C | `255, 45, 56` | rojo en el límite |

Interpolar linealmente entre esos puntos. La barra se normaliza entre 50 y
138 °C.

## Qué no forma parte de la pantalla final

Los tres deslizadores bajo el reloj existen únicamente para probar el diseño.
El de RPM verifica la lógica oculta de motor parado/encendido. No deben
trasladarse al firmware de conducción.

## Restricción de implementación

Este documento fija el aspecto y la lógica visual, pero no valida las
conversiones eléctricas de los sensores Innovate. Mantener
`OIL_GAUGE_DEMO_MODE=1` hasta caracterizar y contrastar ambos sensores con el
MTX-D.
