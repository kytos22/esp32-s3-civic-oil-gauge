# Oil Gauge UI Design

Status: **approved binding reference; native LVGL implementation built and indoor physical match accepted**.

Editable reference:
[`design/references/oil-gauge-design.fragment.html`](design/references/oil-gauge-design.fragment.html).
Standalone browser version:
[`design/references/oil-gauge-design.html`](design/references/oil-gauge-design.html).
Static capture:
[`design/references/oil-gauge-design.png`](design/references/oil-gauge-design.png).

## Composition

- Target canvas: 480×480 px.
- Pure black background to switch off AMOLED pixels.
- Vertical 50/50 split: oil pressure top, oil temperature bottom.
- Each numeric value is centered on the complete horizontal axis. The left
  icon and right unit never displace it.
- Bars are 9 px thick at 480×480.
- Numeric values are white; labels/units/references use dimmed white.
- Semantic state labels are right-aligned in 24 px Montserrat for distance
  readability; the physical demo confirms they are unclipped and glanceable at
  handheld distance.
- RPM is never shown. It is only an internal pressure-warning input.

## Icons

Use compact automotive symbols with thick strokes and filled elements:

- pressure: wide oil can with cap, spout, and detached droplet;
- temperature: solid thermometer, three side marks, and two oil waves below.

Prototype SVG geometry is binding. Firmware may convert it to paths, polygons,
or monochrome bitmaps without changing the silhouette.

## Oil pressure

Provisional visual scale: 0–150 PSI.

| Condition | State label | Color/behavior |
|---|---|---|
| RPM = 0 | `MOTOR PARADO` | No alarm |
| 0–10 PSI and RPM > 0 | `WARNING` | Red, flash at 1 Hz |
| 11–14 PSI and RPM > 0 | `PRESIÓN BAJA` | Provisional, validate |
| 15–80 PSI | `OK` | Yellow/amber |
| >80 PSI | `PRESIÓN ALTA` | Provisional, validate |

Normal icon/bar color: `rgb(255, 176, 32)`.
Alarm color: `rgb(255, 57, 72)`.

During `WARNING`, icon, text, and bar flash; the numeric value remains fixed
and readable. With reduced motion, those elements remain fixed red.

## Oil temperature

The MTX-D display starts near 49 °C. Until a wider direct curve is validated:

- below 50 °C render `<50 °C`;
- keep icon blue and bar empty below 50 °C;
- never show a precise number below the validated range.

Semantic states:

| Temperature | State label |
|---|---|
| <70 °C | `FRÍO` |
| 70–74 °C | `CALENTANDO` |
| 75–93 °C | `ÓPTIMO` |
| 94–100 °C | `CALIENTE` |
| >100 °C | `MUY CALIENTE` |

Bar, icon, and label use the same continuous interpolation:

| Point | RGB | Meaning |
|---:|---|---|
| 50 °C | `30, 132, 255` | Cold blue |
| 57 °C | `30, 132, 255` | Begin progressive transition |
| 75 °C | `174, 205, 167` | Light desaturated green |
| 89 °C | `174, 205, 167` | End stable green |
| 94 °C | `234, 190, 82` | Yellow/amber |
| 100 °C | `255, 118, 28` | Orange |
| 138 °C | `255, 45, 56` | Red at sensor display limit |

Interpolate linearly between stops. Normalize the bar from 50–138 °C.

## Prototype-only controls

The three sliders below the gauge exist only to explore the design. The RPM
slider proves hidden engine stopped/running logic. None belongs in the driving UI.

## Implementation restriction

This document fixes appearance and visual logic; it does not validate Innovate
sensor conversion. Keep `CONFIG_OIL_GAUGE_DEMO_MODE=y` until both sensors are
characterized and compared against the MTX-D.

## Firmware implementation

- Fixed-coordinate LVGL renderer: `src/oil_gauge_ui.cpp`.
- Board/display startup and 13 ms continuous demo sequence: `src/main.cpp` and
  `src/demo_sequence.cpp`.
- Embedded Montserrat subsets: `src/fonts/`.
- Framework: ESP-IDF 6.0.2, official Waveshare BSP 2.0.1, LVGL 9.5.0.
- Software/hardware evidence: 14/14 native tests and a complete 676,224-byte
  ESP32-S3 image generated on 2026-08-04 from application version `3e0298a`,
  SHA-256 `042942dc254dc1cdb51529c338d716aecedded144edd263097742600b7abb8e5`.
  Exact-board flash passes and eight consecutive completed-frame windows measure
  65–67 FPS.

The approved HTML/SVG geometry was not redesigned. The user-supplied physical
photo and complete 60 FPS demo video were reviewed on 2026-08-14: the visual match,
glyph integrity, warning blink, semantic labels, centered values, and 50/50 layout
pass indoors at handheld distance. Daylight, night, glare, and in-vehicle motion
remain mandatory before vehicle cutover.
