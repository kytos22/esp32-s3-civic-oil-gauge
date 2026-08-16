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
| 5 Development | Sprints 1–5 complete; Sprint 6 native-scan presenter candidate built after two failed GPIO-TE architectures | [Sprint 1](sprints/sprint-1-fluid-demo.md) complete; [Sprint 5](sprints/sprint-5-warning-audio.md) loop runtime-proven; [Sprint 6](sprints/sprint-6-settings-menu.md) hardware partially verified |
| 6 Documentation | partial | Existing hardware, BOM, calibration, and UI documentation |
| 7 Release | pending | No release or vehicle cutover |
| 8 Website | n/a — no intent | — |

## Current position
- Phase: 5 — Sprint 6 display synchronization architecture
- Golden Prototype 1: commit `443eb72`, retained as the exact-board regression
  reference in Git history without duplicating a golden firmware tree or claiming
  production readiness.
- Next action: request new exact-board flash authorization for the clean D-054
  candidate. The physical pass must check upright orientation,
  touch mapping, menu scroll, both red transitions, completed-DMA cadence and the
  diagonal. Sensor and vehicle work remain gated.

## Open items
- D-054 follows the method now documented by Espressif for diagonal tearing after
  SPI hardware rotation: the CO5300 returns to native `MADCTL=0x00`, LVGL applies
  the upright 270-degree rotation in PARTIAL mode, and a separate GPIO43-TE task
  presents only immutable full-frame snapshots. One canonical RGB565 framebuffer,
  two 480x120 draw buffers and two aligned direct-PSRAM-DMA snapshots separate
  rendering from scanout. The pure slot policy first failed to compile because it
  did not exist; it now passes three AC-41 regressions inside the 26/26 native
  suite. The revised source contract first failed 15/33 and now passes 34/34. Clean
  commit `50dee93` produces a 734,816-byte ESP-IDF 6.0.2 app with SHA-256
  `d02ba8f1a9a5cb819a7fa63b6d05c6eae859a842a18e2d543b365aeb5b6fabc1`.
  No flash is authorized by this software result.
- D-053 red-first changed the display contract and produced the expected 21/24
  failure against the still-oriented source. Green removes all post-init panel
  `swap_xy()`/`mirror()` calls, explicitly preserves Waveshare `MADCTL=0xA0`, and
  adds a per-frame non-nested draw duration while keeping the remaining D-051
  variables fixed. The contract passes 24/24 and the native suite 23/23. The clean
  ESP-IDF 6.0.2 build at commit `dbdc856` produced a 755,744-byte app with SHA-256
  `7b4ae20345c537cd7a329bc5649153babe43ca1e40bac9a0dcbec471f0e960ce`.
  The separately authorized exact-board identity matched before flash and all four
  written regions passed esptool's hash verification. Runtime measured TE at 59.403 Hz and
  confirmed `MADCTL=0xA0`, but the serialized FULL/single-buffer path normally
  transfers only 14.82–14.85 FPS: roughly 29–31 ms draw plus 32–36 ms synchronous
  flush per 62–66 ms render event. No panic, watchdog or unexpected reset appeared
  in the bounded capture. Marcos confirmed the orientation is correct and the
  diagonal persists. The diagonal therefore does not follow the D-051 `0x60`
  address-direction change; D-053 closes the orientation A/B without fixing
  presentation.
- D-052 corrects the earlier over-broad D-051 conclusion. The failed candidate
  changed Waveshare's working `MADCTL=0xA0` to `0x60`, which is the deterministic
  cause of the observed 180-degree inversion. Adapter `TE_SYNC` independently forces
  LVGL FULL mode and one buffer; the measured render event encloses its 31–35 ms
  synchronous flush, leaving roughly 30–34 ms of drawing/scheduling rather than
  adding 61–65 ms on top. The diagonal remains unisolated because orientation,
  render mode and buffering changed together. That A/B alone did not reject
  hardware orientation; D-054 later resolves the category using Espressif's
  diagonal-tearing guidance plus the repeated exact-board result. Commit `443eb72`
  is Golden Prototype 1.
- D-051 software candidate is complete. Red-first failed at the former 20 ms
  profile and at 12/18 display invariants; green removes software rotation and the
  custom draw callback, uses CO5300 hardware orientation plus adapter 0.6.3
  `TE_SYNC` on GPIO43, and uses one 480x480 PSRAM draw buffer. Native tests pass
  23/23, the display/audio contract passes 23/23, and the complete ESP-IDF 6.0.2
  build produced a 756,048-byte app with SHA-256
  `a5f81c11f43b8a5bb9cc22d6f27717c2dabe5bf59c6a989fb75ff13d9fba2b23`.
  Effective configuration has demo mode enabled and both application production
  and LVGL refresh set to 15 ms; TE remains the physical presentation gate. The
  two-second summary log now reports completed transfers, idle LVGL refreshes,
  render time, TE-wait-plus-DMA time, and transfer intervals. Marcos authorized and
  the exact board received the rebuilt 756,048-byte image with SHA-256
  `a5463ce6b787f97543049231986abfb36f2c020064fab3a3ba54d852c220f192` on
  2026-08-16. All four write-time hashes and all three immutable post-boot digests
  passed. GPIO43 TE measured 59.483 Hz and the official synchronized path started,
  but 33 retained timing windows show mostly 14.84–14.86 FPS, with periodic windows
  near 9 FPS; render averages are roughly 61–65 ms and flush averages roughly
  31–35 ms. There was no panic, watchdog, or reset. The official single-buffer path
  therefore fails the performance target. Marcos then confirmed the image is rotated
  180 degrees and that the diagonal tearing absent from the prior version returned.
  This rejects the exact combined D-051 configuration. It does not isolate or reject
  hardware orientation generally; D-052 defines the required one-variable A/B.
- D-049 records why an ESP32-S3 request for 50 MHz QSPI still resolves to 40 MHz:
  GPSPI is sourced from 80 MHz APB and uses integer divisors. The effective
  experiment therefore changes application/LVGL cadence to 20 ms and the measured
  target to 50 completed FPS; 80 MHz remains outside the accepted panel boundary.
- The software candidate is complete: native tests pass 23/23 and the full ESP-IDF
  6.0.2 build produced a 754,192-byte application with SHA-256
  `44ba2a5c69fe8963aefaabf94fc9d4ce0293ebfbf5639da9c2d8c51fe674c964`.
  Effective configuration retains `CONFIG_OIL_GAUGE_DEMO_MODE=y` and uses
  `CONFIG_LV_DEF_REFR_PERIOD=20`; QSPI remains 40 MHz. No flash was performed.
- Marcos authorized and flashed that 40 MHz candidate on the exact board. App
  `9fc2977`, 754,192 bytes, SHA-256
  `2f5930076df38e1f946a6489a8dc429c8d217df39d5a35d159045cd40b2d315f`,
  passed all four write-time hashes and all three post-boot immutable-region checks.
  Ignored capture `.artifacts/hardware/2026-08-15/sprint6-50hz-9fc2977.typescript`,
  SHA-256 `73f3077c0165bc297ef8f3979c4b5aa1b0e31e716a37df3313506b62cd0af13d`,
  records 29 dynamic windows at 46–50 FPS in total, then one 5 FPS and 31 retained
  17 FPS reports after one LVGL-lock timeout before later recovery. D-050 authorizes
  the requested 80 MHz comparison while retaining the 20 ms application cadence.
- The project-owned 80 MHz override now passes its 6/6 source contract, all 23
  native tests, and a complete ESP-IDF 6.0.2 build. The effective image still has
  demo mode enabled and `CONFIG_LV_DEF_REFR_PERIOD=20`; the Waveshare BSP compile
  command contains the forced include while its managed source remains unedited.
- Exact-board app `443eb72`, 754,272 bytes, SHA-256
  `f9f8a0e9ba5833a909cca0cbce6e939321edebb248d5c4ae18973a9b9ee7161d`,
  passed all four write-time hashes and all three immutable post-boot digest checks.
  Ignored capture `.artifacts/hardware/2026-08-15/sprint6-80mhz-443eb72.typescript`,
  SHA-256 `37c0595383000c1a1877da1b6bb6491d0baa7f277236bffd95691e389bfcf5c6`,
  confirms the 80 MHz CO5300 request, eight matched static-red pause/resume pairs,
  and 27 dynamic windows: 6 at 48 FPS, 14 at 49 FPS, and 7 at 50 FPS. No LVGL-lock
  failure, panic, watchdog, or reset appeared in the retained 71.5-second run.
- Authorized exact-board app `728c4de` passed all four write-time region hashes and
  booted cleanly, but the retained run exposed repeated 44–56 FPS windows exactly
  during the full-screen warning and no warning-loop start. NVS confirms warning
  sound enabled at 77%. Source inspection found that the already-visible 480×480
  red layer was cleared from `LV_OBJ_FLAG_HIDDEN` on every 13 ms update, forcing a
  continuous full-panel invalidation and starving the lower-priority audio worker.
  A red regression now requires edge-triggered overlay visibility before correction.
- Edge-gated exact-board app `7c3a7c5` proves the audio behavior: six captured
  warning episodes each log one loop start and one loop idle roughly 6.7 seconds
  later, with sound enabled. Normal gauge windows measure 66–77 FPS. Full-screen
  warning windows remain 45–57 FPS because the renderer still updates the hidden
  gauge underneath the opaque red layer. A second red regression now requires the
  same no-background-render rule already used by the resident settings menu and a
  truthful pause of the FPS counter while the intentional 1-second static red frame
  is displayed.
- Final exact-board app `9d49ead`, 754,192 bytes, SHA-256
  `f0975d82b45ba927a3fccc2ffe6937ed46b0e0487a12789e6517d36e9f34a699`, passed all
  four write-time hashes and post-boot verification of bootloader, partition table,
  and application. Its ignored capture
  `.artifacts/hardware/2026-08-15/sprint6-warning-final-9d49ead.typescript`, SHA-256
  `51f326b213544032cbc8aea54ae07f278f86ab622da01df94afbc419d5c3178e`, records 44
  dynamic-gauge windows at 63–76 FPS, 12 matched static-red pause/resume pairs, and
  no runtime fault. Final NVS had sound disabled, so audible judgment remains. The
  preceding ignored `7c3a7c5` capture (SHA-256
  `9030bd30efb6c8880c2670793b7a1996a9a57d34a46548450fddd6ce98080439`) records six
  matched warning-loop start/idle pairs with sound enabled; the later change did not
  modify audio code.
- Sprint 6 second-review artifact: the red native fixture failed to compile before
  warning-loop commands existed and the new display/audio contract passed only 4/9.
  The implementation now keeps settings resident, returns from the renderer before
  touching gauge widgets, loops the ramped double beep until warning exit/disable,
  and uses a project-owned ESP LCD/LVGL registration with two 480×480 RGB565 PSRAM
  draw buffers. Native 22/22, settings 9/9, display/audio 9/9, all 41 acceptance
  rows, and the complete ESP-IDF 6.0.2 build from clean implementation commit
  `1b28eac` pass. The 753,856-byte app has SHA-256
  `56ea0cddcf76b7079619489cdaf2814899d0a328db346be7cabddb73a2304f4e`.
  No flash was performed; lack of a verified CO5300 TE GPIO leaves final tearing,
  continuous-audio FPS, and audible stop/edge quality as HARDWARE/JUDGMENT.
- Sprint 6 review-extension artifact: clean ESP-IDF 6.0.2 build from implementation
  commit `8c2cc46`, 752,960-byte app, SHA-256
  `6b854240f99d4edf92e8bdf0507ca83b26b0ad0fb69161d3146961c5d7543127`.
  Marcos authorized and completed the exact-board flash on 2026-08-15. The
  Espressif USB identity and chip MAC both matched the locally recorded exact
  display; all four write regions passed hash verification, and bootloader,
  partition table, and the
  complete application passed independent post-boot digest verification. Ignored
  capture `.artifacts/hardware/2026-08-15/sprint6-fahrenheit-bar-8c2cc46.typescript`,
  SHA-256 `53d431227357bf5f3eaab1f5ac8467a6b08769f06919e73897877232ed40ea93`,
  records a clean demo boot and 19 consecutive 61–77 FPS windows with no panic,
  watchdog, reset, or application error. Guided visual/touch/audio judgment remains.
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
  is enabled. Clean commit `da7cbfb` builds a 751,472-byte application with SHA-256
  `995a743bb3b3e3153381669bc88fefa8b209ca96c22e6481656ec0e2d9af40f9`.
  This correction is included in the exact-board app `8c2cc46`; guided physical
  proof of the revised behaviors remains pending.
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

Last updated: 2026-08-16 — native-scan software-rotation presenter built; exact-board proof requires new authorization
