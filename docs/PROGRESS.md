# PROGRESS — Civic ESP32 Oil Gauge

> Living state. Read this FIRST in every session. Keep current and compact.

## Project card
- Name / one-line purpose: Civic ESP32 Oil Gauge — replace the Innovate MTX-D display while retaining and safely characterizing its installed oil pressure and temperature sensors.
- Project type: embedded firmware / reusable measurement component
- Stack & target platform(s): C++26 firmware on ESP-IDF 6.0.2, Waveshare ESP32-S3-Touch-AMOLED-2.16 BSP 2.0.1, LVGL 9.5.0; C++17 native measurement tests; ADS1115 planned
- License: PolyForm Noncommercial 1.0.0 for project-authored content; required
  notice names Marcos Vidal; commercial use requires separate permission
- Docs language: English
- Security profile: `references/security/library-component.md`, extended with automotive electrical and fail-safe display constraints
- Accessibility: embedded-display adaptation of WCAG 2.2 AA principles; warnings never rely on color alone
- i18n: single-language built product — Spanish status labels, SI/PSI units as specified; source identifiers and documentation in English
- Installed base: fresh prototype; no released firmware, users, migration, or stored user data
- Design system: existing approved baseline — `docs/UI_DESIGN.md` and `docs/design/references/`
- Keel portability: lock + embedded v5.13.0 in `.claude/skills/keel/` and `.agents/skills/keel/`; lock refresh pending reconciliation
- Assistant config: rules (tools: Codex); permissions and Git hook deferred by D-008
- Models: n/a — Codex has no project markdown subagents; checks run inline
- Keel baseline: v5.3.2
- Website intent: no
- Client budget: no
- User guide: deferred until the hardware-validated release candidate
- Docs theme: n/a until Phase 6
- Durability: git remote `origin` at `https://github.com/kytos22/esp32-s3-civic-oil-gauge.git`
- Branches: integration branch `develop` / current work branch `develop`; reconciliation changes remain local until the autonomy decision is recorded
- Chaining: off

## Phase status
| Phase | Status | Key artifacts |
|---|---|---|
| 1 Discovery | adopted (as-built) | `docs/00-competitive-landscape.md`, `docs/01-discovery.md`, `docs/estimate.md` |
| 2 Functional spec | adopted (as-built) | `docs/02-functional-spec.md`, `docs/03-technical-plan.md`, `docs/flows/`, `docs/threat-model.md` |
| 3 Design handoff | adopted — no-Design branch | `docs/design/DESIGN-BRIEF.md`, `docs/design/design-handoff/` |
| 4 Faithful build | renderer implemented; physical fidelity pending | `docs/BUILD-SPEC.md`, `src/oil_gauge_ui.cpp` |
| 5 Development | Sprint 5 in progress; Sprint 1 physical judgment remains open; Sprints 2–4 complete | [Sprint 5](sprints/sprint-5-warning-audio.md) onboard warning audio; [Sprint 1](sprints/sprint-1-fluid-demo.md) physical visual judgment |
| 6 Documentation | partial | Existing hardware, BOM, calibration, and UI documentation |
| 7 Release | pending | No release or vehicle cutover |
| 8 Website | n/a — no intent | — |

## Current position
- Phase: 5 — Development  Step: Sprint 5 onboard warning audio
- Next action: after separate authorization, flash the exact board and verify the
  double beep plus consecutive completed-frame FPS windows. Keep demo mode enabled;
  sensor calibration and vehicle cutover remain separate safety-gated work.

## Open items
- Unresolved user questions: exact pressure excitation/signal pin assignment,
  confirmed pressure transfer function, temperature curve/electrical range,
  installed connector identities,
  and the Keel v5.13.0 reconciliation choices
  (autonomy, forge issue duties, issue capture, and test-first policy). The
  user-supplied MTX-D diagram confirms only the physical topology: two
  temperature conductors (gauge and ground) and three pressure conductors (two
  gauge and ground). Route A/ADS1115 is selected for the gauge; MTX-D serial is
  laptop-only calibration equipment. Innovate `11-0161A` makes 5 V excitation
  and 0.5–4.5 V for 0–150 PSI the leading pressure hypothesis, but no official
  source found equates that document's sensor explicitly with P/N `12-0074`.
- Keel reconciliation: pending v5.3.2 → v5.13.0. Embedded copies are updated and byte-identical; the lock refresh, full conformance sweep, card additions, red-first migration, and verifier changes await one batched user decision.
- Open Design Requests: none
- Sprint 5 software result: the warning-entry gate failed first with `Expected
  TRUE Was FALSE`, then the 15/15 native suite passed. The full ESP-IDF 6.0.2
  firmware builds with demo warning audio enabled at 35%; physical sound and
  post-change FPS remain unverified because no flash was authorized.
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
- Interactive simulator: both bilingual README previews and CTAs point to the
  HTTPS GitHub Pages simulator. Pages serves the byte-identical approved HTML
  from `main:/docs` (47,509 bytes; SHA-256
  `b96bddee2599f8fd4ddfcc233e6ba87aec415dcbc7c38567554197cded62e9e1`).
  Browser-driven slider movement remains `PLATFORM-IMPOSSIBLE` in this session
  because Browser bootstrap rejects the WSL workspace path containing spaces.
- Animated preview: both READMEs use the same 736×700 looping GIF generated from
  205 smooth-step HTML samples at 50 FPS. Optimized storage retains 142 unique
  frames (132 at 20 ms), a 4.1-second loop, 1,212,439 bytes and SHA-256
  `1764a74750fcc07b492df7a9b1dbde7994e96181d40dfeab3faffad819e60f1c`.
- Repository license: project-authored content uses the byte-matched official
  PolyForm Noncommercial 1.0.0 text plus `Required Notice: Copyright 2026 Marcos
  Vidal`. Remote Git blob IDs for `LICENSE.md`, `NOTICE` and `README.md` match the
  local commit. GitHub labels the nonstandard license family as `Other`; the full
  terms and bilingual summaries are visible in the repository.

### Deferred items (consciously postponed work)
- Direct-sensor calibration and final analog front end — safety-critical — when hardware and reversible harness are present
- Physical 50/50 UI fidelity/glanceability judgment — medium — after an explicitly authorized flash and capture
- CAN/OBD second-display work — separate project/scope; do not merge into the oil gauge firmware

Last updated: 2026-08-11 — Sprint 5 warning audio complete in software; physical proof awaits separate flash authorization
