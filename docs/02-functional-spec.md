# Functional Spec — Civic ESP32 Oil Gauge

> Adopted project — reconstructed as-built. Requirements describe the approved target;
> implementation state is called out explicitly.

## Functional requirements

### F-01 — Safe startup and acquisition mode

- Inputs: boot state, `OIL_GAUGE_DEMO_MODE`, ADS1115 presence, calibration validity.
- Processing: initialize serial, I²C, display, and ADC; scan the bus; refuse engineering-unit conversion without valid calibration.
- Outputs: explicit demo, raw-ADC, missing-ADC, or calibrated operating screen.
- Preconditions: 5 V regulated supply to VBUS/USB-C; all I/O within 3.3 V limits.
- Postconditions: no uncalibrated voltage is presented as PSI or °C.
- Errors: display failure, ADC absence, invalid calibration, invalid/out-of-range input, and math errors are visible in logs and/or the display.

### F-02 — Oil pressure measurement and state

- Inputs: protected A0 ADC voltage, measured excitation, validated pressure calibration, engine-running/RPM state.
- Processing: restore the divider input, convert through the measured calibration, filter, range-check, then evaluate RPM-aware thresholds.
- Outputs: centered PSI value, pressure icon/bar, semantic status.
- Preconditions: the direct sensor curve has passed the calibration gate; otherwise only raw voltage is permitted.
- Postconditions: engine-off state does not generate a low-pressure warning.
- Errors: missing/invalid input produces a fault, never a plausible pressure.

### F-03 — Oil temperature measurement and state

- Inputs: protected thermistor node voltage, selected/measured pull-up, validated R/T calibration.
- Processing: compute resistance, convert through the validated model/table, filter, range-check, then map the approved continuous color stops.
- Outputs: centered °C value or `<50 °C`, temperature icon/bar, semantic status.
- Preconditions: the thermistor curve has passed the calibration gate.
- Postconditions: values below the validated lower bound are not shown with false precision.
- Errors: open, short, missing calibration, out-of-range, or math failure produces a fault.

### F-04 — Approved 50/50 AMOLED interface

- Inputs: converted values, semantic states, animation preference, fault flags.
- Processing: render pressure in the upper half and temperature in the lower half against pure black.
- Outputs: the exact geometry, icons, bar thickness, typography alignment, colors, and state behavior in `docs/UI_DESIGN.md`.
- Preconditions: the reference prototype remains the binding visual source.
- Postconditions: numeric values remain horizontally centered independently of icons and units.
- Errors: warning/fault presentation does not depend on color alone.

### F-05 — Calibration and cutover evidence

- Inputs: synchronized MTX-D/LogWorks or MTS values, raw ADC values, excitation, temperature/pressure reference, RPM/engine state.
- Processing: capture points, fit only justified models, validate on held-out points, compare cold/hot/multiple-speed behavior, retain source data.
- Outputs: calibration coefficients or tables, residual/error report, cutover decision.
- Preconditions: reversible high-impedance harness; MTX-D remains connected.
- Postconditions: direct acquisition is enabled only after the acceptance limits pass.
- Errors: inconclusive/nonlinear data keeps direct acquisition disabled and the
  original MTX-D remains installed; laptop MTS may provide evidence but is not
  an embedded fallback.

### F-06 — Automotive power and wiring safety

- Inputs: switched 12 V, fuse, transient/reverse-polarity protection, automotive-capable 5 V buck, star ground.
- Processing: supply and protect the display and acquisition electronics without back-feeding USB.
- Outputs: stable 5 V rail and protected analog/digital inputs.
- Preconditions: bench verification under load and enclosure isolation.
- Postconditions: no vehicle voltage reaches VBUS, 3V3, or GPIO directly.
- Errors: undervoltage/restart must return to a safe boot state without persistent false values.

## Reference artifacts

| Path | Kind | Binding behavior |
|---|---|---|
| `docs/design/references/oil-gauge-design.fragment.html` | HTML visual reference | Exact 480×480 approved oil layout and interactive state simulator |
| `docs/design/references/oil-gauge-design.html` | standalone HTML reference | Portable rendering of the same design |
| `docs/design/references/oil-gauge-design.png` | static capture | Visual comparison baseline; HTML remains editable source |

## Data model

No persistent user data exists.

| Entity | Fields | Validation |
|---|---|---|
| RawChannel | channel, ADC code, volts, timestamp | finite, ADC range, channel known |
| Calibration | kind, coefficients/table, valid flag, source dataset, validation error | explicit valid flag; finite values; evidence reference |
| ConvertedSample | pressure PSI, temperature °C, engine state, faults, timestamp | range and fault state carried with values |
| DisplayState | pressure state, temperature state, blink phase, reduced-motion flag | deterministic mapping from sample |

Calibration values are compile-time constants today. Persistent calibration storage is out of v1
unless introduced by a recorded scope change.

## Integrations

| Integration | Method | Limits / trust boundary | Failure handling |
|---|---|---|---|
| ADS1115 | I²C 0x48 at 3.3 V | Inputs must never exceed VDD + 0.3 V; board bus already has pull-ups | Explicit ADC-missing/raw-only state |
| MTX-D direct sensors | protected analog front end | Curves and pins are unknown until measured | Calibration remains invalid |
| MTX-D MTS | Innovate cable to laptop RS-232/USB-to-RS-232 | Calibration reference only; initial baud/channel order is a hypothesis | Reject malformed/unidentified frames |
| LogWorks | PC-side reference logging | External software and serial adapter required | `HARDWARE` guided leg |
| Vehicle RPM state | source not yet selected | Must be evidence-backed; may come from CAN/WiCAN or another protected input | Pressure warning remains unarmed if engine state is unknown; show fault/pending state |

## Permissions matrix

There are no user roles. The only operational modes are:

| Actor/mode | Allowed |
|---|---|
| Driver | Read display only; no required touch interaction |
| Developer, bench | Demo/raw acquisition, logs, controlled calibration tools |
| Developer, vehicle | Read-only supervised capture after explicit authorization |
| Firmware | Never enable direct calibrated output without valid calibration evidence |

## Flows index

- `docs/flows/boot-and-display.md`
- `docs/flows/calibration-and-cutover.md`

## Technical plan

See `docs/03-technical-plan.md`.

## Design split

- Needs design/faithful implementation: the single 480×480 oil screen and all its normal, warning, cold/warm, hot, very-hot, fault, and demo states.
- No new design: measurement math, filters, I²C, MTS decoding, calibration fitting, power/harness documentation.
- External manual setup: reversible sensor harness, LogWorks/MTS reference capture, bench power, physical board tests, supervised vehicle measurements.
- External assets: final enclosure/52 mm mount and any photographed connector references.
- Accessibility/glanceability: no color-only state, stable centered numbers, readable status/fault labels, reduced-motion fallback, adequate night/day brightness.
- Rich references: the three `docs/design/references/oil-gauge-design.*` files.
- Target device: Waveshare 480×480 only; no responsive breakpoints.

## Acceptance criteria

- **AC-01:** With demo mode enabled, the display labels all simulated data as demo and never implies sensor validity.
- **AC-02:** With missing calibration, pressure and temperature conversion functions return `calibrationMissing`.
- **AC-03:** Missing ADS1115 is visible and never yields a retained last-known or fabricated value.
- **AC-04:** Invalid ADC/thermistor input returns an explicit fault and cannot become an engineering-unit value.
- **AC-05:** Engine stopped/RPM zero never triggers low-pressure warning.
- **AC-06:** At engine-running state and 0–10 PSI, the pressure warning text/icon/bar flash at 1 Hz while the numeric value remains stable; reduced motion uses fixed red.
- **AC-07:** Pressure 15–80 PSI maps to the approved amber OK state; 11–14 and >80 remain explicitly provisional until threshold validation.
- **AC-08:** Temperature below 50 °C renders `<50 °C`, starts at blue, and does not show a precise number.
- **AC-09:** Temperature color interpolation follows the approved stops at 50, 57, 75, 89, 94, 100, and 138 °C.
- **AC-10:** Temperature semantic states are cold below 70, warming at 70–74, optimal at 75–93, hot at 94–100, and very hot above 100 °C.
- **AC-11:** Both numbers are horizontally centered on the complete 480 px axis and the pressure/temperature regions are equal height.
- **AC-12:** The display background is pure black and bars are 9 px thick in the 480×480 reference coordinate system.
- **AC-13:** Direct pressure agrees with the MTX-D within 2 PSI in the normal range on held-out cold/hot points before cutover.
- **AC-14:** Direct temperature agrees with the MTX-D within 2 °C from 60–130 °C on held-out points before cutover.
- **AC-15:** Open/short faults for both sensors are detected before cutover.
- **AC-16:** Cold/hot validation is repeated over at least three drives and startup/restart produces no persistent false value.
- **AC-17:** Vehicle 12 V never reaches VBUS, 3V3, or GPIO directly; the protected 5 V rail is bench-verified under load.
- **AC-18:** No Innovate harness is cut and every pin/color assignment is backed by measurements/photos.
- **AC-19:** Unit tests and the complete firmware build run successfully in the current environment before any flash.
- **AC-20:** Real display, ADC, MTS, power, thermal, and vehicle results remain `unverified` until their evidence is stored.
- **AC-21:** Demo pressure, temperature, and RPM interpolate continuously between
  the seven synthetic scenes at a 13 ms application cadence; native tests cover
  midpoint, scene boundary, forward progress, and sequence wrap.
- **AC-22:** The physical demo sustains at least 60 completed display frames per
  second over consecutive adapter one-second windows; scheduled cadence alone is
  not accepted as FPS evidence.
- **AC-23:** Both semantic state labels use 24 px Montserrat, remain right-aligned,
  and are physically readable without overlap or clipping at 480×480.
- **AC-24:** English and Spanish root READMEs provide truthful oil-gauge-only
  status, features, hardware, safety, build, documentation and firmware navigation.
- **AC-25:** Future validated releases use immutable `firmware/<version>/`
  directories with application and complete images, bilingual instructions, visual
  proof and SHA-256 checksums; no development binary is presented as a release.
- **AC-26:** The publishable snapshot contains no golden-version file, turbo-gauge
  project data, tracked generated build tree, local log or exact device identifier.
- **AC-27:** No GitHub remote is created or pushed until Marcos explicitly chooses
  visibility; a public remote starts from a sanitized snapshot rather than the
  development history that previously recorded the test-board identifier.
- **AC-28:** Both root README previews link to the live HTTPS simulator. The
  GitHub Pages copy exposes working pressure, RPM and temperature range controls
  backed by the approved standalone HTML, without implying that GitHub can run
  JavaScript inline inside README content.
- **AC-29:** Both root READMEs use the same looping GIF generated from synthetic
  pressure, RPM and temperature states of the approved standalone simulator; the
  GIF remains clickable and does not replace or fork the interactive source.
- **AC-30:** The README GIF renders continuous smooth-step transitions from the
  approved simulator at 50 FPS, using real per-frame gauge values rather than
  cross-fades between a small set of static screenshots.
- **AC-31:** Project-authored code, documentation and assets are covered by the
  unmodified PolyForm Noncommercial License 1.0.0 plus a required copyright
  notice; both READMEs state that improvements and redistribution are permitted
  only for noncommercial purposes and that third-party licenses remain separate.
- **AC-32:** In demo mode, each transition from non-warning to the engine-running
  pressure-warning state requests exactly one non-blocking double beep through the
  onboard ES8311 speaker path; remaining in warning does not retrigger it, leaving
  warning re-arms it, and audio failure never stops the visual gauge.

## Estimate

See `docs/estimate.md`. Client budget is not applicable.

## Open questions

All unresolved items require measurement, not an idea-level user decision. They are tracked in
`docs/PROGRESS.md` and the adoption audit.
