# PROGRESS — Civic ESP32 Oil Gauge

> Living state. Read this FIRST in every session. Keep current and compact.

## Project card
- Name / one-line purpose: Civic ESP32 Oil Gauge — replace the Innovate MTX-D display while retaining and safely characterizing its installed oil pressure and temperature sensors.
- Project type: embedded firmware / reusable measurement component
- Stack & target platform(s): C++26 firmware on ESP-IDF 6.0.2, Waveshare ESP32-S3-Touch-AMOLED-2.16 BSP 2.0.1, LVGL 9.5.0; C++17 native measurement tests; ADS1115 planned
- License: public repository visibility; no distribution license selected yet
- Docs language: English
- Security profile: `references/security/library-component.md`, extended with automotive electrical and fail-safe display constraints
- Accessibility: embedded-display adaptation of WCAG 2.2 AA principles; warnings never rely on color alone
- i18n: single-language built product — Spanish status labels, SI/PSI units as specified; source identifiers and documentation in English
- Installed base: fresh prototype; no released firmware, users, migration, or stored user data
- Design system: existing approved baseline — `docs/UI_DESIGN.md` and `docs/design/references/`
- Keel portability: lock + embedded v5.3.2 in `.claude/skills/keel/` and `.agents/skills/keel/`
- Assistant config: rules (tools: Codex); permissions and Git hook deferred by D-008
- Models: n/a — Codex has no project markdown subagents; checks run inline
- Keel baseline: v5.3.2
- Website intent: no
- Client budget: no
- User guide: deferred until the hardware-validated release candidate
- Docs theme: n/a until Phase 6
- Chaining: off

## Phase status
| Phase | Status | Key artifacts |
|---|---|---|
| 1 Discovery | adopted (as-built) | `docs/00-competitive-landscape.md`, `docs/01-discovery.md`, `docs/estimate.md` |
| 2 Functional spec | adopted (as-built) | `docs/02-functional-spec.md`, `docs/03-technical-plan.md`, `docs/flows/`, `docs/threat-model.md` |
| 3 Design handoff | adopted — no-Design branch | `docs/design/DESIGN-BRIEF.md`, `docs/design/design-handoff/` |
| 4 Faithful build | renderer implemented; physical fidelity pending | `docs/BUILD-SPEC.md`, `src/oil_gauge_ui.cpp` |
| 5 Development | Sprint 1 in progress; Sprint 2 complete | [Sprint 1](sprints/sprint-1-fluid-demo.md) physical visual judgment; [Sprint 2](sprints/sprint-2-repository-productization.md) repository productization |
| 6 Documentation | partial | Existing hardware, BOM, calibration, and UI documentation |
| 7 Release | pending | No release or vehicle cutover |
| 8 Website | n/a — no intent | — |

## Current position
- Phase: 5 — Development  Step: Sprint 1 physical visual judgment
- Next action: review the corrected 60 FPS demo physically on the target display.
  Keep demo mode enabled; sensor calibration and vehicle cutover remain separate
  safety-gated work.

## Open items
- Unresolved user questions: exact sensor pinouts/curves, installed connector identities, and final hardware route A vs B
- Open Design Requests: none
- Unverified external steps/assets: corrected commit `8e4c24e` was written and verified
  on the locally recorded exact board. A bounded boot capture proved ESP-IDF 6.0.2,
  demo mode, 16 MB flash, 8 MB PSRAM, 480×480 display/touch initialization, and more
  than 20 seconds without an error, reset, or watchdog. The first physical photo then
  showed corrupted fragments in static and dynamic text while non-text geometry remained
  coherent. The serialized-font correction from commit `9b806cc` is now present in
  flashed app version `701d0b4`; exact serial/MAC identity and every written region
  were verified. The 669,824-byte image SHA-256 is
  `7591c7cf5a02899252ad8404f67fa93d557c52124b93c2f76aeabd2b9f63ffbe`. A bounded
  boot capture proves demo mode, ESP-IDF 6.0.2, 16 MB flash, 8 MB PSRAM, 480×480
  display/touch initialization, and no error/reset/watchdog after startup. Physical
  corrected-text evidence, I²C scan, ADC, MTS, power, thermal, and EMC validation
  remain open. The user accepted proceeding without a complete factory backup; two
  1 MB chunks remain diagnostic evidence only. Sprint 1 app `3e0298a` now uses
  linear interpolation, fractional-pixel bar edges, a 15 ms cadence, completed-frame
  FPS logging, and the dedicated uncompressed 24 px Spanish state font. Its
  676,224-byte image SHA-256 is
  `042942dc254dc1cdb51529c338d716aecedded144edd263097742600b7abb8e5`.
  The local exact-board identifier and all four written regions verified. The bounded
  log records a clean ESP-IDF 6.0.2/demo boot and eight consecutive completed-frame
  windows of 65–67 FPS. Physical visual judgment remains open.
- Forge issues in progress: none
- Repository publication: Marcos selected public visibility for
  `kytos22/esp32-s3-civic-oil-gauge` on 2026-08-04. The public repository is live
  at `https://github.com/kytos22/esp32-s3-civic-oil-gauge`, with `main` created
  from sanitized root `5ad0a4d`; the private development history remains only in
  the ignored local recovery bundle.

### Deferred items (consciously postponed work)
- Direct-sensor calibration and final analog front end — safety-critical — when hardware and reversible harness are present
- Physical 50/50 UI fidelity/glanceability judgment — medium — after an explicitly authorized flash and capture
- CAN/OBD second-display work — separate project/scope; do not merge into the oil gauge firmware

Last updated: 2026-08-04 — Sprint 2 complete; sanitized public repository verified
