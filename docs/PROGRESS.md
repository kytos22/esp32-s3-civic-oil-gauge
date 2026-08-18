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
- Keel portability: lock + embedded v5.15.1 in `.claude/skills/keel/` and `.agents/skills/keel/`; display/project state reconciled, with the new close/Stop automation explicitly pending in D-074
- Assistant config: rules (tools: Codex); permissions and pre-commit hook deferred by D-008; unconditional post-commit courier guard installed by D-074
- Autonomy: manual; issue duty off; Issue sweep interval: n/a; Issue capture: off
- Notify: none
- Test-first policy: pure-logic from D-062; existing tests are not retroactive
- Models: n/a — Codex has no project markdown subagents; checks run inline
- Keel baseline: v5.13.0 (embedded runtime is v5.15.1; D-074 keeps the incomplete automation delta visible)
- Website intent: no
- Client budget: no
- User guide: deferred until the hardware-validated release candidate
- Docs theme: n/a until Phase 6
- Durability: git remote `origin` at `https://github.com/kytos22/esp32-s3-civic-oil-gauge.git`; accepted history published through merged PR #1; public `main` contains the project baseline
- Branches: integration branch `develop`; current work branch
  `codex/post-triple-tasks` starts at `pb5-good-base`. The parked display experiment
  remains isolated on `codex/triple-buffer-pipeline` at `0594dba` and is not merged
- Chaining: off

## Phase status
| Phase | Status | Key artifacts |
|---|---|---|
| 1 Discovery | adopted (as-built) | `docs/00-competitive-landscape.md`, `docs/01-discovery.md`, `docs/estimate.md` |
| 2 Functional spec | adopted (as-built) | `docs/02-functional-spec.md`, `docs/03-technical-plan.md`, `docs/flows/`, `docs/threat-model.md` |
| 3 Design handoff | adopted — no-Design branch | `docs/design/DESIGN-BRIEF.md`, `docs/design/design-handoff/` |
| 4 Faithful build | renderer implemented; indoor physical fidelity accepted | `docs/BUILD-SPEC.md`, `src/oil_gauge_ui.cpp` |
| 5 Development | Sprints 1–5 complete; Sprint 6 visual baseline accepted; Sprint 7 ADS1115 bare-board proof complete; Sprint 8 physically accepted | [Sprint 5](sprints/sprint-5-warning-audio.md) complete; [Sprint 6](sprints/sprint-6-settings-menu.md) D-061 accepted; [Sprint 7](sprints/sprint-7-ads1115-diagnostics.md) AC-42 runtime-proven; [Sprint 8](sprints/sprint-8-warning-threshold-startup.md) Honda/Civic composition and controls accepted |
| 6 Documentation | partial | Existing hardware, BOM, calibration, and UI documentation |
| 7 Release | pending | No release or vehicle cutover |
| 8 Website | n/a — no intent | — |

## Current position
- Phase: 5 — accepted feature baseline; ordinary feature work may continue
- Golden Prototype 1: commit `443eb72`, retained as the exact-board regression
  reference in Git history without duplicating a golden firmware tree or claiming
  production readiness.
- Accepted Physical Baseline 2: D-057 implementation commit `9febd47`. Marcos
  confirmed no tearing, the best menu motion so far, correct warning and colors,
  removal of only the fixed notes, and retention of every dynamic state indicator.
- Accepted Physical Baseline 3: D-059 implementation commit `08b04eb`. Marcos
  confirmed rapid brightness works without blocking, 21 px is optimal and tearing
  remains absent. The temperature colors and vertical group placement move to
  D-060 without reopening the accepted transport, brightness or thickness.
- Accepted Physical Baseline 4: D-061 implementation commit `c0bce25`. Marcos
  accepted both supplied icon silhouettes, enlarged units, dynamic recoloring,
  alignment/spacing and the continued absence of tearing on the exact AMOLED.
- Accepted Physical Baseline 5: corrected Honda/Civic implementation commit
  `411c793`. Marcos confirmed the complete two-layer startup composition is perfect
  on the exact AMOLED after the verified `pb2-d057-26-g411c793` flash.
- Accepted Physical Baseline 6: tag `pb5-good-base` preserves the complete current
  no-tearing firmware and accepted UI immediately before the triple-buffer display
  experiment. It remains the rollback point until a new exact-board candidate is
  separately authorized, flashed and physically accepted.
- Next action: continue non-triple-buffer work from this restored accepted base;
  await Marcos's next requested task. The display architecture is now current in
  `docs/reference/display-pipeline.md`; if the isolated performance experiment is
  reopened, its first slice is deletion/proof of the sole LVGL refresh scheduler,
  before any compositor or buffer-count change. ADS1115 grounded/divider
  validation remains the next separate electrical step; sensors and vehicle work
  remain gated.

## Open items
- D-075/D-076 publish `codex/post-triple-tasks` and merge public PR #1 into
  `main`. GitHub created merge commit `ef8e86c`; remote verification proves that
  the complete accepted branch through `bb5202b` is an ancestor of public
  `origin/main`.
- D-066 and its successors are parked for later on branch
  `codex/triple-buffer-pipeline` at `0594dba`. They are deliberately absent from
  this branch and must not be merged while ordinary feature work continues. Marcos
  requested restoration of `pb5-good-base`; exact USB/chip/app identity and all
  four write hashes passed. The retained rollback run reports stable 30.493–32.707
  FPS after startup, TE about 59.5 Hz, and `timeouts=0 errors=0 fatal=0`.
- D-073 records the adapter audit and the transferable part of the RGB buffering
  sequence. The accepted branch uses `esp_lvgl_adapter` only for lifecycle,
  locking, timers and touch; display registration, GPIO43 TE, full-frame QSPI DMA
  and buffer release remain project-owned. The future experiment must delete the
  automatic display refresh timer because LVGL 9.5 re-arms a paused timer on
  `LV_EVENT_REFR_REQUEST`. No firmware or hardware changed in D-073.
- D-074 updates both embedded Keel copies and lock stamps to v5.15.1, installs
  and configures the post-commit stale-handoff guard, and adds the shared session
  identity helper. The v5.14/v5.15 `keel-close` and Stop-hook executables remain
  intentionally marked `missing`: Keel publishes behavioral contracts but no
  canonical generated scripts, and Codex Desktop exposes no project Stop-hook
  registration. They require a separate reviewed automation slice rather than
  an invented close/push mechanism in this documentation-only update.
- D-065 adds a persistent 1–30 PSI low-pressure warning threshold (10 PSI default)
  and a persistent 0–10 second Honda startup splash (1 second default; 0 disables).
  The menu presents the canonical threshold in PSI or one-decimal BAR, while the
  engine-state gate and established warning/audio path remain unchanged. The boot
  overlay reuses the boost project's 320×215 Honda artwork on opaque black. Red-first
  native compilation failed for the absent fields/helpers/threshold API; green is
  31/31. The new source contract passes 16/16 and verifies the 206,400-byte
  RGB565A8 payload plus the byte-identical source PNG. The complete ESP-IDF 6.0.2
  build at implementation commit `1cc15f5` produces a 995,856-byte candidate
  (`pb2-d057-23-g1cc15f5`) with SHA-256
  `62af1d287378c0d8f7e3668d266241cd0b72ea093728a49df7f41f95c101d906`.
  Marcos authorized the exact-board flash. The exact ESP32-S3 revision 0.2,
  8 MB PSRAM, 16 MB flash, USB-Serial/JTAG and recorded MAC matched. All four
  written regions passed hash verification. A bounded boot confirmed the exact
  app identity, 59.555 Hz TE, touch, audio and ADS1115 at 0x48; steady telemetry
  reported roughly 29–33 completed FPS with `timeouts=0 errors=0 fatal=0`.
  Physical judgment of the new controls and splash remains pending.
  Marcos then identified that the accepted boost boot composition also contains a
  separate Civic wordmark, omitted from the first candidate. The correction now
  byte-matches and renders both source assets, and the expanded contract passes
  23/23. The complete ESP-IDF 6.0.2 build at commit `411c793` produces corrected
  candidate `pb2-d057-26-g411c793`, 1,034,960 bytes, SHA-256
  `39f114bf3221891b939798d8ca522c9a297d68717959958777d2462ce379505f`.
  Marcos authorized the corrected exact-board flash. The recorded MAC matched,
  all four regions passed write-time hash verification, and bounded boot confirmed
  the exact app, 59.527 Hz TE, touch/audio/ADS1115 startup, 31–33 steady completed
  FPS and `timeouts=0 errors=0 fatal=0`. Marcos confirmed the corrected Honda/Civic
  composition is perfect, physically accepting its appearance.
- D-064 replaces only the two full-screen red-warning message lines from 24 px
  Montserrat Medium to a dedicated 36 px Montserrat Bold subset. `PELIGRO` moves
  to y=210 and both labels use 404 x 48 centered boxes; the pressure number,
  warning cadence, red background and all other UI remain unchanged. The simulator
  mirrors the 36 px/700-weight typography. Red-first source verification failed
  0/9 before implementation and now passes 11/11; complete ESP-IDF 6.0.2 build
  passes. Clean commit `bf3faa0` builds app `pb2-d057-19-gbf3faa0`, 787,968 bytes,
  SHA-256 `95a8a245f8d02f535b12c4f7306c48a35269e79b6cbd8b43f51eb0d0f0cb62e4`.
  Marcos authorized the exact-board flash. The flashed app is
  `pb2-d057-20-gff4c17c`, 787,968 bytes, SHA-256
  `f7928e6bac7177ccb3e7226f117f297fa26f66b8aae80f797970978cf0952fb4`;
  exact ESP32-S3/MAC identity matched and all four written regions passed hash
  verification. A bounded boot capture confirmed ESP-IDF 6.0.2, demo mode,
  usable 59.516 Hz TE, touch/audio initialization, ADS1115 at `0x48`, and display
  telemetry with `timeouts=0 errors=0 fatal=0`. Marcos subsequently confirmed
  that all visually inspected aspects appear correct, accepting the larger/bolder
  warning typography and spacing without an apparent regression.
- D-063 adds a bench-only ADS1115 path without changing D-061 rendering or enabling
  sensor conversions. The pure protocol creates single-shot A0–A3 configurations
  at PGA ±4.096 V / 128 SPS and converts signed counts at 125 µV/LSB. Its red-first
  native run failed on the deliberately absent `ads1115_protocol.h`; green is
  29/29. The ESP-IDF path reuses the Waveshare BSP bus, probes `0x48`, registers a
  100 kHz device and logs four explicitly labelled floating-input readings once per
  second. Clean commit `d770bb5` builds app `pb2-d057-17-gd770bb5`, 782,416 bytes,
  SHA-256 `1a472ab5fb757c8ed4c5e6146e01a9b7e8f92c28b204ea394a647d8bb53eda86`.
  Marcos authorized the exact-board flash. VID/PID `303a:1001`, ESP32-S3/8 MB PSRAM,
  USB-Serial/JTAG and MAC matched; all four written regions passed hash verification.
  Boot found the ADS1115 at `0x48` and repeatedly captured A0–A3 at about
  0.552–0.562 V while floating. Touch registered, audio opened, demo remained enabled,
  and the bounded display reports retained `timeouts=0 errors=0 fatal=0`. No sensors
  are attached. Before enumeration, SDA/SCL had accidentally occupied adjacent exposed
  USB D−/D+ pads P4/P5; moving them to I²C P6/P7 immediately restored the exact USB
  device. This wiring fault is resolved and did not require a firmware change.
- D-062 reconciles the project from Keel v5.3.2 to v5.13.0 after Marcos approved
  the recommended one-time choices: manual autonomy, no automatic forge issue
  activity, no external notifications and pure-logic test-first going forward.
  Both portability locks now byte-match the v5.13.0 canonical block; chaining
  remains off and no external publication behavior changes.
- D-061 embeds the supplied temperature and pressure silhouettes as 92 x 72 LVGL
  A8 masks and uses matching transparent PNG masks in the simulator. This retains
  D-060's state-driven icon colors instead of baking the source red/white pixels
  into the UI. PSI, BAR, °C and °F now use Montserrat 28 px. Source references:
  `oil temp black.png` SHA-256
  `6fb79a549825379055a942b454fd5c0c248e6027329640fb189daf5b1a5fb06b` and
  `oil pressure black.png` SHA-256
  `77d869eadf8ffab7e8f360ee0e996c85449a4a5112460d481b9ad75a03a11954`.
  Native tests pass 28/28 and display/audio and asset invariants pass 74/74.
  Clean implementation commit `c0bce25` builds a 780,368-byte ESP-IDF 6.0.2 app
  identified as `pb2-d057-13-gc0bce25`, with SHA-256
  `61d93b65a481a3712babef21f2afdf528645f3f4c77c0a25d48744feb4a4443f`.
  Marcos authorized the exact-board flash. VID/PID `303a:1001`, ESP32-S3/8 MB
  PSRAM and the recorded MAC matched; all four written regions passed hash
  verification. Boot confirms exact app `pb2-d057-13-gc0bce25`, TE 59.620 Hz,
  DMA about 13.2–14.8 ms and `timeouts=0 errors=0 fatal=0` throughout the bounded
  capture. Marcos then accepted every requested physical check: both silhouettes,
  unit readability, alignment/spacing, dynamic recoloring and no tearing. D-061 is
  therefore Accepted Physical Baseline 4.
- D-059 raises both bars to 21 px and changes temperature semantics to cold below
  60 °C (still displaying `<50` below measurable range), warming 60–75 °C,
  optimal 76–95 °C, hot 96–100 °C, and very hot above 100 °C. It also fixes the
  rapid-brightness lock: UI events only replace one atomic pending value, and the
  display presenter sends the newest command `0x51` after frame DMA completion,
  never concurrently from the main task. Red evidence was the old boundary test,
  prior 18 px geometry and missing serialization contract; the implementation now
  passes 27/27 native tests and 55/55 display/audio invariants. Clean commit
  `08b04eb` builds a 730,672-byte ESP-IDF 6.0.2 app identified as
  `pb2-d057-6-g08b04eb`, with SHA-256
  `64f04f9a9ef40fa02b57e75593c19ee0d1f5f5fd0475b2c863f6f694258efb43`.
  Marcos authorized the exact-board flash. VID/PID `303a:1001`, serial/MAC and
  binary identity matched before writing; all four regions passed write-hash
  verification. Boot confirms `pb2-d057-6-g08b04eb`, TE 59.511 Hz, DMA about
  13.1–14.6 ms and `timeouts=0 errors=0 fatal=0` throughout a bounded 45-second
  capture. Marcos then confirmed rapid brightness works without blocking, 21 px is
  optimal and tearing remains absent. The old color-stop mapping is rejected because
  it did not align with the new semantic bands.
- D-060 aligns color progression with the refined physical request: blue through
  59 °C, green at 76, gradual light amber through 90, intense orange at 100,
  red at 120 and fixed red through 140. From 120 °C the dynamic state becomes a
  2 Hz blinking red `WARNING` while number, icon and bar remain visible. It moves each
  icon/value/unit/bar group down 4 px while leaving headings and dynamic states in
  place. Red evidence was one native color mismatch plus 55/63 display invariants;
  group down 4 px while leaving headings and dynamic states in place. The first
  clean D-060 candidate `476cf4d` is superseded before flash by this refined color
  and warning contract. Red was a missing enum/visibility contract and 61/68
  invariants; green is native 28/28 and display/audio 68/68. Clean commit `c15e63f`
  builds a 730,640-byte ESP-IDF 6.0.2 app identified as
  `pb2-d057-11-gc15e63f`, with SHA-256
  `8705132d455e5eba09f6cc1e087a471d21975afcdbbcb266d58712e1c12e92d0`.
  Exact-board flash and judgment remain pending fresh authorization.
- D-058 changes only bar geometry/rendering above the accepted D-057 pipeline. It
  increases both bars from 15 px to 18 px and removes the separate square,
  fractional-opacity leading-edge object that Marcos saw as a transparency halo
  against the rounded fill. Width now rounds to the nearest physical pixel and a
  single LVGL rounded object owns the visible endpoint. The red-first contract
  failed 45/48 and now passes 48/48; native tests pass 27/27. Clean commit
  `c0be6df` builds a 730,496-byte ESP-IDF 6.0.2 app identified as
  `pb2-d057-3-gc0be6df`, with SHA-256
  `3ad8e75542bb25dbb27b3b8685e0ce9f2557a4bf5da3c0118e1d22f3d7ae415c`.
  Marcos authorized the exact-board flash. The app image and exact board identity
  matched before all four write hashes passed. Boot confirmed
  `pb2-d057-3-gc0be6df`, native scan, FULL double buffering and GPIO43 TE at
  59.491 Hz. The bounded runtime has DMA about 13.1–14.6 ms and
  `timeouts=0 errors=0 fatal=0`; its cadence remains in D-057's known sub-target
  range. Marcos confirmed the endpoint is now clean and tearing remains absent;
  18 px remains visually too thin and is superseded by D-059's 21 px candidate.
- D-057 removes the measured full-frame snapshot copy with two complete
  `RGB565_SWAPPED` PSRAM buffers in LVGL `FULL` mode. The project-owned presenter
  queues the rendered pointer directly on GPIO43 TE and releases it only after LCD
  DMA completion. Pinned LVGL `DIRECT` was rejected because its buffer-sync path
  copies full invalidated menu areas (twice with three buffers). The corrected
  visual scope removes only the four small fixed threshold notes beneath the bars,
  preserves every live state label, and raises both bars from 9 px to 15 px. The
  source contract failed first at 34/46 and now passes 46/46; native tests pass
  27/27; clean commit `9febd47` produces a 731,104-byte ESP-IDF 6.0.2 app with
  SHA-256 `625715cffaeca5c12100aad6754979e24e7bba5c163950b3e837e85dd9160e33`.
  Marcos authorized the exact-board flash. VID/PID, USB serial and the ESP32-S3
  MAC matched before writing; all four write hashes passed. Boot confirmed app
  `9febd47`, native scan, two FULL `RGB565_SWAPPED` buffers and GPIO43 TE at
  59.554 Hz. The bounded capture has `timeouts=0 errors=0 fatal=0` and DMA around
  13.2–14.6 ms, but dynamic presentation is only about 26.5–29.0 FPS and the
  demo's lower-activity interval is about 16 FPS. D-057 therefore proves correct
  ownership/transport, but fails the intended 50–60 FPS cadence. Marcos physically
  accepted it as Physical Baseline 2: no tearing; best menu motion to date; warning,
  colors, fixed-note removal and dynamic indicators all correct. The only rejected
  details were the 15 px weight and the square fractional endpoint artifact now
  isolated in D-058.
- D-056 follows Marcos's acceptance that physical mounting direction can absorb
  orientation: remove `lv_display_set_rotation()`, `lv_display_rotate_area()` and
  `lv_draw_sw_rotate()`. Dirty PARTIAL areas now copy row-for-row into the native
  canvas with only the required RGB565 byte swap; TE, snapshots, bounded 8-row
  transfers and ownership remain unchanged. The revised display contract first
  failed 31/35 and now passes 35/35; native tests pass 27/27. Clean commit
  `9b59722` produces a 733,232-byte ESP-IDF 6.0.2 app with SHA-256
  `a4ef30f5c0dd974cb02360dabf537fd2d6a2575e5c4e37a61e9e6ada3dc5ebd3`.
  The separately authorized exact board matched VID/PID and MAC before all four
  writes passed hashes. Boot confirmed app `9b59722`, native no-rotation scan and
  TE at 59.522 Hz. A 30-second capture kept
  `timeouts=0 errors=0 no_slot=0 fatal=0`, measured compose averages about
  0.3–1.1 ms, snapshot copies about 16–21 ms, DMA about 13.1 ms and completed
  presentation about 17–33 FPS. Marcos confirmed USB-C-right native orientation,
  correct touch, no tearing or diagonal and visibly smoother menu motion than
  D-055, although menu FPS remain low. The full snapshot copy is now the measured
  performance bottleneck.
- D-055 keeps D-054's native scan, software rotation and immutable ownership, but
  removes direct PSRAM-to-GPSPI DMA after the exact board returned the documented
  `DMA TX underflow` / `ESP_ERR_INVALID_STATE` failure at 80 MHz QSPI. ESP LCD now
  stages a full snapshot through three queued internal-DMA chunks of eight rows
  each (23,040 bytes maximum) while retaining the 80 MHz bus. A transfer-start
  error or completion timeout latches `fatal=1` and stops the presenter rather
  than leaving a poisoned transaction queue blocked invisibly. Red evidence is
  the missing-profile native compile plus a 6/13 source contract; green is 27/27
  native, 13/13 QSPI contract and 34/34 display/audio contract. Clean commit
  `aa38f5f` produces a 734,896-byte ESP-IDF 6.0.2 app with SHA-256
  `92392058e67e0dde440f805f159e98c60754dca4c83164ddf87aa03dc3d6065a`.
  The separately authorized exact board matched VID/PID and MAC before all four
  regions passed write-time hashes. A 30-second boot/runtime capture confirmed
  app `aa38f5f`, TE at 59.438 Hz, completed presentation at about 17–35 FPS,
  13.0–13.5 ms DMA duration and `timeouts=0 errors=0 no_slot=0 fatal=0` throughout.
  Marcos confirmed no tearing or diagonal; the visible UI is rotated 180 degrees
  relative to the prior desired mounting direction, which is acceptable because
  the display can be mounted accordingly. Observed cadence remains about 17–35 FPS.
- D-054 followed the method documented by Espressif for diagonal tearing after
  SPI hardware rotation: the CO5300 returns to native `MADCTL=0x00`, LVGL applies
  the upright 270-degree rotation in PARTIAL mode, and a separate GPIO43-TE task
  presents only immutable full-frame snapshots. One canonical RGB565 framebuffer,
  two 480x120 draw buffers and two aligned snapshots separate
  rendering from scanout. The pure slot policy first failed to compile because it
  did not exist; it now passes three AC-41 regressions inside the 26/26 native
  suite. The revised source contract first failed 15/33 and now passes 34/34. Clean
  commit `50dee93` produces a 734,816-byte ESP-IDF 6.0.2 app with SHA-256
  `d02ba8f1a9a5cb819a7fa63b6d05c6eae859a842a18e2d543b365aeb5b6fabc1`.
  The exact authorized board and MAC matched before flash and all four written
  regions passed esptool hash verification. Boot confirmed app `50dee93`, native
  `MADCTL=0x00`, TE at 59.434 Hz and the intended buffers, but the first direct
  PSRAM transfer underflowed. ESP LCD returned `ESP_ERR_INVALID_STATE`, the queue
  then stopped, and every telemetry window remained at `presented=0.000 fps`.
  D-054 therefore fails before any diagonal/orientation judgment; D-055 supersedes
  only its transfer path. A post-boot readback retry stopped at 512,000 bytes and
  was not repeated, per L-004.
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
  hardware orientation. D-054 applied Espressif's diagonal-tearing guidance but
  failed at its direct DMA leg before visual judgment; D-055 retains that scan-order
  experiment with bounded staging. Commit `443eb72` is Golden Prototype 1.
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

Last updated: 2026-08-16 — D-061 accepted as Physical Baseline 4
