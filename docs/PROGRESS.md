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
| 4 Faithful build | renderer implemented; indoor physical fidelity accepted | `docs/BUILD-SPEC.md`, `src/oil_gauge_ui.cpp` |
| 5 Development | Sprints 1–5 complete; Sprint 6 exact-board runtime passed, guided touch/visual proof pending | [Sprint 1](sprints/sprint-1-fluid-demo.md) complete; [Sprint 5](sprints/sprint-5-warning-audio.md) complete; [Sprint 6](sprints/sprint-6-settings-menu.md) hardware partially verified |
| 6 Documentation | partial | Existing hardware, BOM, calibration, and UI documentation |
| 7 Release | pending | No release or vehicle cutover |
| 8 Website | n/a — no intent | — |

## Current position
- Phase: 5 — Sprint 6 physical-review corrections complete in software
- Next action: request explicit authorization to flash the revised image, then verify
  selectable/persisted no-data `SENSORES`, a menu that remains open, the atomic 0.5 Hz
  full-screen danger warning, puff-free audio edges, and sustained completed-frame
  FPS. NVS reboot persistence is already physically confirmed. Keep compile-time
  demo support enabled; sensor calibration and vehicle cutover remain separate
  safety-gated work.

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
- Sprint 1 warning correction: the renderer-contract check failed first at 0/6,
  then the new 250 ms boundary regression passed in the 16/16 native suite and
  `keel-verify` passed 6/6 warning invariants. The complete ESP-IDF 6.0.2 build
  produced clean app `002581d`, a 724,336-byte demo image with SHA-256
  `4443c6c7675e17abc105a316004530963569275e03eaba23f9fa0f2d2287e6e3`.
  The icon, label, and bar now switch from full red to full transparency; the
  numeric pressure stays visible. The exact authorized board now runs app `002581d`:
  all regions passed write-time verification, immutable regions passed post-boot
  verification, and the retained log SHA-256
  `6251e71a1df6f8fd38447216e068198d7b3bf9004691d9a33bd4776f26893b63`
  records a clean demo boot, two warning tones, and 16 consecutive 64–77 FPS windows.
  Marcos confirmed that the current warning looks clean without the dotted phase,
  closing AC-06, AC-23, slice 1.5, and Sprint 1 on 2026-08-14.
- Open Design Requests: none. [DR-001](design/design-requests/DR-001.md) is answered
  and consolidated through D-043. In full-screen 0.5 Hz mode the prebuilt opaque-red
  phase contains the white pressure number and danger message; the number is never
  hidden.
- Sprint 6 software result: the new settings tests first failed at link time because
  the model did not exist, then the native suite passed 19/19. The firmware now has
  a 700 ms full-screen settings menu, live/persisted safe settings, runtime warning
  audio controls, PSI/bar display, three warning presentation modes, immediate alarm
  interruption, protected reset, and the raised thermometer marks. The full-screen
  red mode redraws the pressure number above its opaque field in every active phase.
  Keel passes 9/9 menu and 11/11 warning invariants. Clean commit `65ebbfa` builds a
  750,720-byte app with SHA-256
  `f2de29c39b3cf7bdc4b06e24c85f98f02b55e17f05fb3fdeecc0743e1afd65fa`.
  Marcos authorized the exact-board test on 2026-08-14. App `65ebbfa` passed the
  USB/chip identity gate, write-time verification, and post-boot verification of
  the three immutable regions. Ignored capture
  `.artifacts/hardware/2026-08-14/sprint6-first-boot.typescript`, SHA-256
  `78391016db981bda1f6284d51b4b56260c8ccf5b9e86232fb01ad22d2a430da8`, records
  one completed warning tone and 24 consecutive 63–76 FPS windows, with no window
  below 60 and no panic, watchdog, reset, or runtime error. Physical touch,
  geometry, all three warning appearances, and sound-volume judgment remain in the
  guided hardware pass; Marcos subsequently confirmed reboot persistence.
- Sprint 6 physical review: Marcos confirmed that settings survive a reboot. He
  requested selectable `SENSORES` with an explicit no-data screen, no automatic menu
  timeout, a tearing-free 0.5 Hz full-screen warning that adds `PELIGRO` / `PRESIÓN
  MUY BAJA` while retaining the pressure number, and removal of the speaker puff at
  tone start/end. D-043 records the exact revision; the current flashed app remains
  the evidence baseline and will not be overwritten without new authorization.
- Sprint 6 physical-review software correction: the new source/cadence regressions
  first failed to compile because `DataSource` and the independent full-screen phase
  did not exist; the source/menu/warning/audio contract then failed 0/12. The revised
  implementation now passes 21/21 native tests, 9/9 menu invariants, 11/11 warning
  invariants, 12/12 physical-review invariants, and a complete ESP-IDF 6.0.2 build.
  `SENSORES` persists but renders `--` / `SIN DATOS`; no ADS1115 path or calibration
  is enabled. Exact-board proof remains pending new flash authorization.
- Ambient-light research: the Waveshare has no onboard light sensor. A future
  `OPT4001-Q1` can share the 3.3 V I²C bus with the ADS1115 at a selected free address;
  `VEML7700` is an easier non-automotive bench option. Protected A3 illumination
  sensing is binary, not ambient lux. Automatic brightness remains deferred.
- Sprint 5 software result: the warning-entry gate failed first with `Expected
  TRUE Was FALSE`, then the 15/15 native suite passed. Exact-board app `0c33fe6`
  flashed with all four regions hash-verified and booted with demo warning audio
  enabled at 35%. The first bounded run was stable but periodic audio-load windows
  measured 58–59 FPS. D-032 improved the first two warning entries to 61–64 FPS,
  but a longer run found a third 58 FPS window. D-033 pinned 512-sample audio work
  to CPU1 and improved that repeated-warning window to 59 FPS, still below target.
  D-034 app `bf5c932` at 13 ms passes exact-board runtime verification: all four
  regions hash-verified; the retained bounded log contains three completed tone
  paths and 29 consecutive 66–77 FPS windows with no panic, watchdog, or audio
  error. Marcos confirmed the physical double beep is audible, completing AC-32
  and Sprint 5 on 2026-08-11.
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
  corrected-text evidence passed on 2026-08-14. I²C scan, ADC, MTS, power, thermal,
  environmental visual, and EMC validation remain open. The user accepted proceeding
  without a complete factory backup; two
  1 MB chunks remain diagnostic evidence only. Sprint 1 app `3e0298a` now uses
  linear interpolation, fractional-pixel bar edges, a 15 ms cadence, completed-frame
  FPS logging, and the dedicated uncompressed 24 px Spanish state font. Its
  676,224-byte image SHA-256 is
  `042942dc254dc1cdb51529c338d716aecedded144edd263097742600b7abb8e5`.
  The local exact-board identifier and all four written regions verified. The bounded
  log records a clean ESP-IDF 6.0.2/demo boot and eight consecutive completed-frame
  windows of 65–67 FPS. The user-supplied 28.423-second 60 FPS video and photo were
  reviewed on 2026-08-14: all semantic states, Spanish accents, centered values,
  icons, bars, black background, and 50/50 geometry pass without corruption,
  clipping, or overlap. The same evidence exposed the warning's dotted low-opacity
  phase; app `002581d` corrected it and Marcos subsequently accepted the clean result.
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
  The local canonical fragment and standalone wrapper now also expose PSI/bar and
  all three warning modes, including the always-visible full-screen pressure value;
  the deployed Pages copy remains unchanged until an authorized push.
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
- Daylight/night/glare/in-vehicle visual assessment — medium — before vehicle cutover
- CAN/OBD second-display work — separate project/scope; do not merge into the oil gauge firmware

Last updated: 2026-08-14 — Sprint 6 exact-board runtime passed; guided touch/visual/persistence proof pending
