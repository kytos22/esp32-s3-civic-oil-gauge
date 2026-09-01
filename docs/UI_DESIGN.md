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
- Bars are 21 px thick at 480×480.
- Numeric values are white; headings and units use dimmed white.
- Semantic state labels are right-aligned in 24 px Montserrat for distance
  readability; the physical demo confirms they are unclipped and glanceable at
  handheld distance.
- Preserve every dynamic semantic state label. Remove only the four small fixed
  threshold notes that previously sat below the bars; their space belongs to the
  thicker indicator bars.
- RPM is never shown. It is only an internal pressure-warning input.
- A stationary 700 ms press-and-hold opens the settings page; ordinary taps,
  dragging, and scrolling do not alter the driving display.

## Icons

Use compact automotive symbols with thick strokes and filled elements:

- pressure: the user-supplied oil-pressure silhouette;
- temperature: the user-supplied oil-temperature silhouette.

The supplied references are binding. Firmware and simulator convert them to
alpha masks so the existing live semantic colors can recolor either silhouette.

The earlier hand-built thermometer geometry is superseded by D-061.

## Oil pressure

Visual bar scale: 0–85 PSI. Values above 85 PSI remain readable while the bar
clamps at full width.

| Condition | State label | Color/behavior |
|---|---|---|
| RPM = 0 | `MOTOR PARADO` | No alarm |
| 0–10 PSI and RPM > 0 | `WARNING` | Red, binary flash at 2 Hz |
| 11–14 PSI and RPM > 0 | `PRESIÓN BAJA` | Provisional, validate |
| 15–80 PSI | `OK` | Yellow/amber |
| >80 PSI | `PRESIÓN ALTA` | Provisional, validate |

Normal bar/label color: `rgb(255, 176, 32)`; alarm color:
`rgb(255, 57, 72)`. The icon has its own palette: red in warning, white in
`OK`, and amber for the remaining valid states.

During `WARNING`, icon, text, and bar alternate between fully visible and fully
transparent every 250 ms; the numeric value remains fixed and readable. No dimmed
intermediate state is allowed. With reduced motion, those elements remain fixed red.

The selectable full-screen warning alternates an opaque red 480×480 field and the
normal black gauge once per second (0.5 complete cycles per second). The red phase
contains the centered pressure number in white plus `PELIGRO` and `PRESIÓN MUY
BAJA`. The number never blinks or disappears. This is one prebuilt top-level layer;
runtime changes only its hidden flag and never reorders or rebuilds it.

## Oil temperature

The demo retains the approved lower-floor presentation:

- below 50 °C render `<50 °C`, or `<122 °F` when Fahrenheit is selected;
- keep icon blue and bar empty below 50 °C;
- in `SENSORES`, show the provisional measured number below 50 °C so resistor
  points down to 10 °C can be checked.

Semantic states:

| Temperature | State label |
|---|---|
| <60 °C | `FRÍO` (`<50` when below measurable range) |
| 60–75 °C | `CALENTANDO` |
| 76–100 °C | `ÓPTIMO` |
| 101 °C to (warning − 0.1) | `MUY CALIENTE` |
| warning to 140 °C | blinking red `WARNING` |

Bar and dynamic label use continuous interpolation:

| Point | RGB | Meaning |
|---:|---|---|
| 50 °C | `30, 132, 255` | Cold blue |
| 59 °C | `30, 132, 255` | End cold blue |
| 76 °C | `174, 205, 167` | Light desaturated green at optimal entry |
| 90 °C | `234, 190, 82` | Light amber after gradual transition from 76 °C |
| 100 °C | `255, 118, 28` | Orange |
| selected warning | `255, 45, 56` | Red warning entry; 120 °C by default |
| 140 °C | `255, 45, 56` | Fixed red at display limit |

Interpolate linearly between stops, with the final orange-to-red segment ending
at the selected 110–140 °C warning threshold. Normalize the bar from 50–140 °C.
At and above that threshold the icon and dynamic `WARNING` label blink together
at 2 Hz; number and bar remain continuously visible in red. The temperature
icon palette is blue below 60 °C, white from 60 °C until warning, and red in
warning.

The pressure and temperature threshold marks remain embedded in their bars; only
the explanatory fixed text beneath them is removed.

## Settings page

A full-screen black settings shell opens after a stationary 700 ms hold. Its
non-scrolling home page contains buttons for `BRILLO`, `DATOS`, `AVISOS`,
`SONIDO`, `UNIDADES`, `ARRANQUE`, `IDIOMA`, and `SISTEMA`, each with a compact
current-value summary. A button opens one independent 480×480 subsection; all
other pages are hidden with `LV_OBJ_FLAG_HIDDEN` and are not rendered.

- `BRILLO`: full-width AUTO/MANUAL mode buttons, numeric 5–100% manual/fallback
  value, two-handle AUTO limits, a persistent −30…+30-point AUTO curve offset,
  and live hub lux/state/target/applied telemetry. Telemetry labels update only
  while this subsection is visible.
- `DATOS`: persistent `DEMO`/`SENSORES` source and the provisional A1/pending
  pressure notices.
- `AVISOS`: pressure and temperature thresholds plus `ELEMENTOS 2 HZ`,
  `PANTALLA 0,5 HZ`, or `FIJO`. Pressure remains canonical PSI and temperature
  canonical Celsius, while both controls follow the selected display units.
- `SONIDO`: enable, numeric 5–100% volume, and the real double-beep test.
- `UNIDADES`: separate PSI/BAR and °C/°F selectors.
- `ARRANQUE`: Honda/Civic logo duration, 0–10 seconds; 0 disables it and 1 second
  is the default.
- `IDIOMA`: persistent Spanish/English selection applied immediately to the
  gauge, warning overlay, summaries, dialog, and every settings subsection.
- `SISTEMA`: confirmation-protected settings reset.

The 24 px UI and 36 px warning fonts contain the complete uppercase Spanish and
English glyph set used by these surfaces. Both locales were measured against
their fixed label widths; the tightest dynamic gauge state retains more than
7 px of horizontal margin before the LVGL clip boundary.

`ATRÁS` returns from a subsection to the home page without closing settings.
`CERRAR` on the home page saves dirty preferences and returns to the gauge. There
is no inactivity timeout and no long scrolling settings canvas.

An active pressure warning does not close the menu. Warning evaluation and the
configured repeating double beep continue, but the gauge and red overlay are not
rendered behind settings; the gauge catches up after `CERRAR`. Safe preferences
persist in NVS when the menu closes; missing/corrupt NVS falls back to compile-time
defaults.
The source choice persists. `SENSORES` enables only the provisional A1 bench
temperature conversion; A0 pressure remains behind its calibration gate.

Automatic brightness affects only the panel request path; it adds nothing to the
approved main gauge. AUTO starts and recovers only after two consecutive usable
hub samples. MANUAL ignores lux for output. Stale or invalid reception returns
smoothly to the saved slider value without automatic NVS writes. A brightness
request wakes the serialized panel presenter even when the gauge has no visual
damage, so static data cannot delay a physical brightness change.

AUTO ignores mapped target changes smaller than 2 percentage points and ramps
the physical output at 40 percentage points/s upward and 25 downward. The
default 20–100% span therefore takes about 2.0 s to brighten and 3.2 s to dim,
without changing MANUAL slider response. Settings shows the stabilized AUTO
target separately from the currently applied ramp value.

Unit conversion is presentation-only. Temperature states, colors, bar position,
and warnings always use canonical degrees Celsius. The large numeric font must
contain `-`, `.`, digits, and `<`; BAR's decimal point uses the same 96 px face and
must never fall back to a missing-glyph rectangle.

This review extension is recorded in [DR-002](design/design-requests/DR-002.md)
and D-045.

## Browser controls

The three sliders below the gauge exist only to explore the design. The RPM
slider proves hidden engine stopped/running logic. None belongs in the driving UI.
The editable simulator additionally exposes the warning threshold and startup-logo
duration so these settings can be reviewed without adding controls to the gauge.

## Implementation restriction

This document fixes appearance and visual logic; it does not validate Innovate
sensor conversion. Keep `CONFIG_OIL_GAUGE_DEMO_MODE=y` until both sensors are
characterized and compared against the MTX-D.

## Firmware implementation

- Fixed-coordinate LVGL renderer: `src/oil_gauge_ui.cpp`.
- Board/display startup and 15 ms producer target: `src/main.cpp` and
  `src/demo_sequence.cpp`.
- Embedded Montserrat subsets: `src/fonts/`.
- Framework: ESP-IDF 6.0.2, official Waveshare BSP 2.0.1, LVGL 9.5.0.
- Baseline software/hardware evidence: 14/14 native tests and a complete 676,224-byte
  ESP32-S3 image generated on 2026-08-04 from application version `3e0298a`,
  SHA-256 `042942dc254dc1cdb51529c338d716aecedded144edd263097742600b7abb8e5`.
  Exact-board flash passed; the historical 65–67 counter was an LVGL/software
  metric and is not used as physical-presentation evidence for the current path.

The PARTIAL v2 plus CivicAux base has run on the exact display and received real
hub ambient-light frames. The bilingual sectioned-menu follow-up passes 70/70
native tests and produces a complete 1,096,800-byte ESP-IDF 6.0.2 image. Its new
home/subsection typography, language switch, curve slider, touch targets, panel
errors and tearing still require the next exact-board acceptance run.

The Sprint 6 physical-review revision passes 21/21 native tests, 9/9 settings
invariants, 11/11 split-cadence warning invariants, 12/12 review invariants, and a
complete ESP-IDF 6.0.2 build: clean app `da7cbfb`, 751,472 bytes, SHA-256
`995a743bb3b3e3153381669bc88fefa8b209ca96c22e6481656ec0e2d9af40f9`.
Marcos confirmed NVS persistence on app `65ebbfa`.
Exact-board proof of the revised no-data source, persistent menu, full-screen
transition/message, puff-free audio edges, and post-change FPS remains pending.

The approved HTML/SVG geometry was not redesigned. The user-supplied physical
photo and complete 60 FPS demo video were reviewed on 2026-08-14: the visual match,
glyph integrity, warning blink, semantic labels, centered values, and 50/50 layout
pass indoors at handheld distance. Marcos subsequently confirmed that app `002581d`
removes the dotted warning phase and looks clean at 2 Hz. Daylight, night, glare,
and in-vehicle motion remain mandatory before vehicle cutover.
