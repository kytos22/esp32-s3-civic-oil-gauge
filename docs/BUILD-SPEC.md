# BUILD-SPEC: Civic ESP32 Oil Gauge

> Source handoff: `docs/design/design-handoff/`
> Canonical evidence: `docs/UI_DESIGN.md` and `docs/design/references/`
> This document is the implementation contract. Code adapts to it; it never adapts to code.

## 1. Adopted-handoff audit

| Item | Checked | Evidence | Result |
|---|---|---|---|
| No-Design branch authorized | yes | D-013 | pass |
| Unique screen resolves | yes | `docs/UI_DESIGN.md`; reference HTML and PNG | pass |
| Tokens have one source | yes | token tables below match `docs/UI_DESIGN.md` | pass |
| Reference bytes preserved during canonical move | yes | before/after SHA-256 values matched on 2026-07-30 | pass |
| States and behavior resolved | yes | pressure/temperature tables below | pass |
| Assets resolved | yes | icons are binding inline SVG geometry in the editable HTML; no external files | pass |
| Fonts resolved | yes | no external font asset is required by the embedded target | pass |
| External assets | yes | none | pass |
| External setup | yes | none for the visual design | pass |
| Accessibility specified | yes | §4 and `docs/design/DESIGN-BRIEF.md` | pass indoors at handheld distance; environmental checks remain |
| Open design questions | yes | DR-001 answered and consolidated; pressure number never hides | pass |
| Foreign delivery files | yes | handoff contains only its orientation file | pass |

The compact adopted handoff is an explicit legacy exception, not a general weakening
of Keel's Design delivery contract. The approved references are not duplicated.

## 2. Resolved screen

| Screen | Type | Source artifact | Contract | Notes |
|---|---|---|---|---|
| Oil gauge | unique, fixed 480×480 | `docs/design/references/oil-gauge-design.fragment.html` | `docs/UI_DESIGN.md` | Pressure top, temperature bottom; 700 ms hold opens settings |
| Settings | full-screen, fixed 480×480 | `docs/UI_DESIGN.md` | DR-001 | Black touch surface; warning interrupts it |

## 3. Canonical tokens

| Token | Value | Role |
|---|---:|---|
| canvas.width / height | `480 px / 480 px` | fixed AMOLED viewport |
| region.height | `240 px` | exact 50/50 split |
| value.centerX | `240 px` | both values centered on full axis |
| background | `#000000` | AMOLED pixels off |
| text.primary | `#F7F9FB` | numeric values; contrast 19.90:1 on black |
| text.secondary | `#9AA4AF` | labels/units; contrast 8.30:1 |
| pressure.normal | `#FFB020` | normal icon/bar; contrast 11.48:1 |
| pressure.warning | `#FF3948` | warning; contrast 5.93:1 |
| bar.thickness | `9 px` | both bars |
| warning.period | `500 ms`, step-end | binary 2 Hz flash; 250 ms on / 250 ms off |
| refresh.period | `13 ms` | application and LVGL target cadence with repeated-audio margin above 60 FPS |
| state.font | `Montserrat 24 px` | pressure/temperature semantic state |

Temperature colors are linearly interpolated between:

| °C | Color | Contrast on black |
|---:|---|---:|
| 50 | `#1E84FF` | 5.80:1 |
| 57 | `#1E84FF` | 5.80:1 |
| 75 | `#AECDA7` | 12.09:1 |
| 89 | `#AECDA7` | 12.09:1 |
| 94 | `#EABE52` | 11.99:1 |
| 100 | `#FF761C` | 7.86:1 |
| 138 | `#FF2D38` | 5.68:1 |

## 4. State matrix

### Pressure

| State | Condition | Label | Color/motion | Sprint 0 core |
|---|---|---|---|---|
| engine unknown | engine state unavailable | pending/fault cue | non-alarming | implemented/tested |
| engine stopped | known RPM = 0 | `MOTOR PARADO` | amber, fixed | implemented/tested |
| warning | running and 0–10 PSI | `WARNING` | red, binary 2 Hz; reduced motion fixed | implemented/tested; physical pass |
| low | running and 11–14 PSI | `PRESIÓN BAJA` | provisional amber | implemented/tested |
| OK | 15–80 PSI | `OK` | `#FFB020` | implemented/tested |
| high | >80 PSI | `PRESIÓN ALTA` | provisional amber | implemented/tested |
| fault | invalid pressure | explicit fault | non-color cue | implemented/tested |

### Temperature

| State | Condition | Label/value | Color | Sprint 0 core |
|---|---|---|---|---|
| below range | <50 °C | `FRÍO`, `<50` | blue, empty bar | implemented/tested |
| cold | 50–69 °C | `FRÍO` | interpolated | implemented/tested |
| warming | 70–74 °C | `CALENTANDO` | interpolated | implemented/tested |
| optimal | 75–93 °C | `ÓPTIMO` | interpolated | implemented/tested |
| hot | 94–100 °C | `CALIENTE` | interpolated | implemented/tested |
| very hot | >100 °C | `MUY CALIENTE` | interpolated to 138 °C | implemented/tested |
| fault | invalid temperature | explicit fault | non-color cue | implemented/tested |

## 4a. Accessibility

- All semantic states include text, not color alone.
- Pressure numeric text never blinks.
- Full-screen warning redraws the pressure number in white above the opaque red
  field; neither warning phase may hide it.
- Reduced motion holds the warning icon, label, and bar red.
- All specified foreground tokens exceed 4.5:1 against black.
- Indoor physical fidelity, text integrity, warning motion, and handheld-distance
  readability passed from the user-supplied full demo video on 2026-08-14.
- AMOLED daylight/night, glare, in-vehicle motion, and environmental color
  assessment remain `⚠ unverified — HARDWARE/JUDGMENT` until supervised tests.

## 5. Interactions and logic

| Trigger | Behavior | Condition |
|---|---|---|
| sample update | recompute deterministic display state | valid converted samples |
| RPM becomes zero | suppress low-pressure warning | engine state known |
| RPM unavailable | do not arm pressure warning | explicit unknown state |
| warning blink phase changes | toggle icon/label/bar only | warning and motion allowed |
| reduced motion enabled | hold warning elements red | warning |
| 700 ms stationary hold | open full-screen settings | gauge visible; no active warning |
| menu idle for 10 s or `VOLVER` | save changed settings and return to gauge | settings visible |
| pressure warning while menu open | close settings immediately and show warning | warning active |
| warning mode `PANTALLA 2 HZ` | alternate opaque red field and normal gauge; redraw pressure above red | warning active |
| units changed | convert the displayed pressure and labels from canonical PSI | never changes calibration or alarm math |
| temperature below 50 | render `<50`, empty temperature bar | valid sample |
| demo frame | linear interpolation plus fractional-pixel bar edge between adjacent synthetic scenes | every 13 ms |

## 6. Asset map

There are no external assets. The two binding icons are inline SVG geometry in
`docs/design/references/oil-gauge-design.fragment.html`; firmware ports the same
silhouettes into native paths/polygons without changing them.

## 7. External manual setup

No external visual-design setup is required. Hardware and calibration are separate
gated workflows and are not authorized by this document.

## 8. Externally generated assets

None.

## 9. Target-stack integration plan

- State and color decisions live in native-testable `gauge_core`.
- `src/oil_gauge_ui.cpp` consumes those decisions in a fixed 480×480 LVGL renderer;
  `src/main.cpp` and `src/demo_sequence.cpp` supply deterministic continuous demo
  or calibration-gate frames.
- Icon geometry is ported from the editable reference without transformation of
  silhouette or proportion; DR-001 raises the thermometer marks and extends its stem.
- The fixed coordinate system is used directly; no responsive or adaptive layout.
- `CONFIG_OIL_GAUGE_DEMO_MODE=y` remains mandatory until real calibration passes.

## 10. Faithfulness checklist

- [x] Physical firmware capture visually matches the approved 480×480 reference indoors.
- [x] Every documented semantic state has a deterministic core representation.
- [x] Every value traces to this token table or `docs/UI_DESIGN.md`.
- [x] Every behavior traces to §5.
- [x] One driving layout plus its separate full-screen settings surface; one state source.
- [x] No external logo, icon, font, or image requires transformation.
- [x] No placeholder copy is presented as calibrated data.
- [x] No external visual setup or generated asset is pending.
- [x] Code-side state-model adaptation preserves design intent.
- [x] Indoor handheld-distance accessibility/glanceability pass completed on the AMOLED.
- [ ] Daylight, night, glare, and in-vehicle-motion visual assessment completed.
- [x] Zero unresolved Design Requests.

The renderer compiles into a complete ESP32-S3 image and its indoor physical match
is accepted. The remaining environmental visual assessment requires a separately
authorized supervised run; the supplied desk video does not prove daylight, night,
glare, or in-vehicle motion performance.

The Sprint 6 settings extension compiles cleanly at commit `65ebbfa` and passes its
software contracts. Its touch flow, revised icon, full-screen warning, persistence,
and post-change FPS are not yet physically accepted because the image was not flashed.
