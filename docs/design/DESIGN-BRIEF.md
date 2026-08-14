# Design Brief: Civic ESP32 Oil Gauge

> Adopted design brief. The user approved the complete design before Keel adoption.
> This brief records that contract; it does not authorize creative changes.

## 0. Working rules

1. Preserve the approved 480×480 design exactly.
2. Treat `docs/UI_DESIGN.md` as the semantic contract and the three files in
   `docs/design/references/` as the visual evidence.
3. Ask before changing any geometry, icon silhouette, color stop, text, threshold,
   motion, or alignment.
4. The firmware adapts to the design. The design never adapts to implementation
   convenience.
5. Sensor conversion remains outside this design contract and stays calibration-gated.

## 1. Context

- Product: a glanceable oil pressure and oil temperature instrument for a 2017 Honda
  Civic Sport 1.5.
- Target: Waveshare ESP32-S3-Touch-AMOLED-2.16, 480×480 CO5300 AMOLED.
- Audience: the driver; no touch interaction is required while driving.
- Purpose: show both oil values simultaneously, make warming/fault/warning states
  immediately legible, and never imply validity when calibration is missing.
- Host constraints: pure embedded C++ renderer, fixed 480×480 viewport, black AMOLED
  background, no browser/runtime assets in firmware.

## 2. Existing design system

- Status: existing, one-screen automotive instrument.
- Canonical sources: `docs/UI_DESIGN.md` and `docs/design/references/`.
- Surface: this Waveshare 480×480 AMOLED only.
- Background: `#000000`.
- Primary numeric text: `#F7F9FB`.
- Secondary text: `#9AA4AF`.
- Pressure normal: `#FFB020`.
- Pressure warning: `#FF3948`.
- Temperature stops:
  `50 #1E84FF`, `57 #1E84FF`, `75 #AECDA7`, `89 #AECDA7`,
  `94 #EABE52`, `100 #FF761C`, `138 #FF2D38`.
- Typography: the approved prototype's condensed sans-serif treatment; firmware must
  match its measured placement and weight using build-native glyphs.
- Geometry: 480×480, equal 240 px regions, values centered at x=240, 9 px bars.
- Icons: the exact pressure-can and thermometer/oil-wave silhouettes in the editable
  reference. They are path geometry, not external image assets.
- Motion: pressure warning uses a binary 2 Hz flash (250 ms fully visible, 250 ms
  fully transparent); reduced-motion mode holds the warning red.

## 3. Screen inventory

One unique screen: `oil-gauge`.

It shows pressure in the upper half and temperature in the lower half. It accepts
converted samples, engine/RPM state, fault state, blink phase, and reduced-motion
preference. RPM is never displayed.

## 4. Required states

- Pressure: engine state unknown, engine stopped, warning 0–10 PSI while running,
  provisional low 11–14 PSI, OK 15–80 PSI, provisional high above 80 PSI, sensor fault.
- Temperature: below-range `<50`, cold 50–69, warming 70–74, optimal 75–93,
  hot 94–100, very hot above 100, sensor fault.
- System: demo-labelled, ADC missing, calibration missing/raw-only, valid calibrated,
  and per-channel fault.
- Fixed viewport only: no responsive breakpoints.

## 5. Accessibility and safety

- State meaning never relies on color alone; every state has a text label or fault cue.
- The white, amber, red, blue, green, yellow, orange, and final red tokens all exceed
  4.5:1 contrast against black.
- Numeric values remain visible and stable during pressure-warning flashing.
- Reduced motion removes flashing while retaining fixed red plus `WARNING`.
- `<50 °C` prevents false precision below the validated display floor.
- No touch target, focus order, or screen-reader surface exists on the driving display.
  Physical glanceability, color perception, daylight/night brightness, glare, and motion
  remain `HARDWARE`/`JUDGMENT` verification.

## 6. References and delivery

- Editable source: `docs/design/references/oil-gauge-design.fragment.html`.
- Standalone simulator: `docs/design/references/oil-gauge-design.html`.
- Approved capture: `docs/design/references/oil-gauge-design.png`.
- Adopted handoff orientation: `docs/design/design-handoff/README.md`.
- Consolidated implementation contract: `docs/BUILD-SPEC.md`.
- External setup: none for design. Hardware/calibration setup is governed by
  `docs/ARRIVAL_CHECKLIST.md` and `docs/CALIBRATION.md`.
- External assets: none.
- Open design questions: none.
