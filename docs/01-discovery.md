# Discovery — Civic ESP32 Oil Gauge

> Adopted project — reconstructed as-built. Inferences are marked `as-built, unverified`.

## Problem & outcome

Replace the visible Innovate MTX-D oil pressure/temperature gauge with a
Waveshare ESP32-S3-Touch-AMOLED-2.16 while retaining the already-installed
Innovate sensors and a reversible harness. The most important outcome is a
readable, fail-safe oil display whose values agree with the MTX-D before the
original instrument is removed.

## Competitive landscape & opportunity

- Scan status: done; see `docs/00-competitive-landscape.md`.
- Table stakes: simultaneous values, readable units, configurable/fail-safe warnings, cold/warm state, fault visibility, dimming, and a calibration/logging path.
- Differentiators: RPM-gated pressure warning, the approved OLED-first 50/50 layout, and a reversible acquisition route that refuses invented sensor curves.
- AI/MCP proposal: forced filler, dropped. A deterministic embedded instrument does not benefit from an AI control layer.

## Project type

- Primary: embedded firmware / reusable measurement component
- Secondary: automotive instrument UI
- Security profile loaded: `references/security/library-component.md`, extended with the project's automotive electrical rules.

## Feature list

| Feature | What it does | User | Priority | Why in v1 | Constraint |
|---|---|---|---|---|---|
| Dual oil display | Shows pressure and temperature simultaneously | Driver | must | table stakes and user's idea | Approved 50/50 UI |
| Fail-safe acquisition | Reads raw channels but withholds engineering units until calibrated | Developer/driver | must | safety | No invented curves |
| Pressure state | Distinguishes engine stopped, warning, low, OK, and high | Driver | must | engine protection | RPM is internal only |
| Temperature state | Shows `<50`, cold, warming, optimal, hot, and very hot | Driver | must | engine protection | Continuous approved colors |
| Sensor fault state | Exposes missing ADC, missing calibration, open/short/out-of-range, or math failure | Driver/developer | must | fail-safe baseline | Never substitute a plausible value |
| Calibration workflow | Captures ADC/MTS/MTX-D reference pairs | Developer | must | unpublished curves | Reversible harness and reference instrument retained |
| Dimming input | Adapts brightness for vehicle lighting | Driver | should | competitor baseline | Protected 12 V input; not yet validated |

## Scope

- v1: the seven features above, a reversible bench harness, direct-sensor calibration evidence, the approved renderer, native tests, and a documented vehicle-validation gate.
- Later: peak/minimum recall, persistent logging, final automotive PCB, enclosure, automated brightness, and maintenance diagnostics.
- Separate scope: the second identical display fed by WiCAN Pro/OBD/CAN.
- Never unless explicitly reversed: touch interaction while driving, guessed sensor curves, cutting Innovate harnesses, or connecting vehicle 12 V directly to the display.

## Honest assessment

The display replacement is technically feasible, but the screen is not the hard
part. The undocumented pressure transfer function, thermistor curve, automotive
power transients, harness identification, and RPM source dominate the risk. Route
B (keeping MTX-D conditioning and receiving MTS) may be the safer practical
endpoint if direct sensor characterization is inconclusive. A custom gauge is
justified by the exact UI and two-screen plan, but it must not claim better
accuracy than the MTX-D without traceable calibration.

- Verdict: proceed with staged scope and mandatory comparison gates.
- User decision: proceed; the user accepted the recommended Keel adoption package on 2026-07-30.

## Constraints & non-negotiables

- Never connect 12 V to VBUS, 3V3, or a GPIO.
- Keep `OIL_GAUGE_DEMO_MODE=1` until calibration evidence exists.
- Do not assume a 0.5–4.5 V pressure sensor or a 10 kΩ NTC.
- Do not infer pin functions from wire colors.
- Do not cut Innovate harnesses.
- Do not remove the MTX-D until cold/hot/multiple-speed comparisons pass.
- Do not flash hardware or test in the vehicle without explicit user authorization.

## License

- At adoption the project was private and not licensed for distribution; D-024
  made the repository public and D-028 later selected PolyForm Noncommercial 1.0.0.
- Every dependency license must be reviewed before any public distribution decision.

## Installed base / upgrade

- Fresh prototype; no installed firmware base or persistent user data.
- The MTX-D remains the installed reference instrument.

## External dependencies

| Dependency | Exact version/reference | Source | Fail-safe behavior if absent/incompatible |
|---|---|---|---|
| pioarduino ESP32 platform | 55.03.311 | `platformio.ini` | Build stops; no fallback binary is claimed |
| Arduino-ESP32 | 3.3.11, provided by platform | platform package | Build stops |
| Arduino_GFX | 1.6.7 | PlatformIO registry | Display failure is logged; no sensor value claim |
| Adafruit ADS1X15 | 2.6.2 | PlatformIO registry | Demo mode or explicit ADC-missing screen |
| Waveshare board files | official repository commit recorded in `docs/RESEARCH.md` | Waveshare GitHub | Pinout is not re-inferred |
| Innovate MTX-D | P/N 39130 | installed hardware/manual | Remains reference until cutover gate passes |

## Internationalization & output language

- Built product: single-language, Spanish semantic state labels with numeric PSI and °C.
- Source identifiers/comments and all maintained documentation: English.
- A later public/multilingual release requires a recorded i18n mechanism; none is claimed now.

## Accessibility and glanceability

- Target: fixed 480×480 embedded automotive display.
- WCAG 2.2 AA principles are applied where meaningful: adequate contrast, no color-only warning, stable centered values, readable fault text, reduced-motion fallback, and no required touch interaction.
- A pressure warning combines red color, semantic text, and flashing; reduced-motion mode uses fixed red.
- Physical legibility, sunlight, night glare, color perception, and motion must be verified on the real display.

## Project website intent

- No project website.

## Design

- Design is required and an approved baseline already exists in `docs/UI_DESIGN.md` plus `docs/design/references/`.
- Target surface: the Waveshare 480×480 AMOLED only.
- Pure black background, approved 50/50 composition, thick automotive icons, 9 px bars, horizontally centered values.
- The existing reference predates Keel. It is binding; no retroactive redesign is authorized.

## Environment & test drivers

- This session can read and write the project and run shell commands in WSL.
- `pio`/PlatformIO is missing from this WSL PATH; `pio test -e native` returned command-not-found on 2026-07-30.
- The project records a successful build and eight passing native tests from 2026-07-28, but that historical result is not a current pass.
- Hardware drivers cannot run because the Waveshare board has not arrived.
- Browser prototype: headless-capable in principle; no browser test harness exists.
- Physical display, touch, power, ADC, MTS, CAN, thermal, and vehicle legs are tagged `HARDWARE`; vehicle operation is also `PRODUCTION-RISK`.
- No software will be installed automatically. PlatformIO restoration is a remediation item.

## Preliminary estimate

- See `docs/estimate.md`.
- Client budget: no.
- Chaining: off.

## Open questions

- Exact sensor pinout, excitation, transfer functions, connector types, MTS
  channel order, board current draw, and enclosure temperature remain unknown
  until measured. D-030 resolves the architecture choice: ADS1115-only in the
  gauge, with MTS available only as a laptop calibration reference.
- The current empty `.git` directory is not a valid repository; initialization remains an explicit structural action.
