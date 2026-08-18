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
- Audience: the driver; no touch interaction is required while driving. A deliberate
  700 ms hold opens a separate settings surface while stationary.
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
  `50 #1E84FF`, `59 #1E84FF`, `76 #AECDA7`, `90 #EABE52`,
  `100 #FF761C`, `120 #FF2D38`, `140 #FF2D38`.
- Typography: the approved prototype's condensed sans-serif treatment; firmware must
  match its measured placement and weight using build-native glyphs.
- Geometry: 480×480, equal 240 px regions, values centered at x=240, 21 px bars;
  both icon/value/unit/bar groups sit 4 px below D-059; no fixed threshold notes
  below the bars.
- Icons: the exact pressure and temperature silhouettes derived from Marcos's
  supplied PNG references. Firmware embeds A8 masks and the editable reference
  uses matching transparent PNG masks.
- Motion: pressure warning uses either a binary 2 Hz element flash, a binary 2 Hz
  opaque full-screen red field, or fixed red. The full-screen field always redraws
  the pressure number above it; the number never disappears.

## 3. Screen inventory

Two unique surfaces: `oil-gauge` and its full-screen `settings` page.

It shows pressure in the upper half and temperature in the lower half. It accepts
converted samples, engine/RPM state, fault state, blink phase, and reduced-motion
preference. RPM is never displayed.

The settings page provides brightness, warning audio, demo/calibration-gated source,
PSI/bar units, warning presentation, diagnostics, and protected reset. Warning
activation interrupts settings and restores the gauge immediately.

## 4. Required states

- Pressure: engine state unknown, engine stopped, warning 0–10 PSI while running,
  provisional low 11–14 PSI, OK 15–80 PSI, provisional high above 80 PSI, sensor fault.
- Temperature: below-range `<50` shown as cold, cold 50–59, warming 60–75,
  optimal 76–95, hot 96–100, very hot 101–119, blinking red warning 120–140,
  sensor fault.
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
- The driving display has no visible touch target; a stationary 700 ms hold is the
  sole entry to large settings controls.
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
