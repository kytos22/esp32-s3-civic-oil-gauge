# Decisions — Civic ESP32 Oil Gauge

> Append-only. A session NEVER re-opens a decision recorded here on its own initiative;
> only the user reverses a decision by appending a superseding entry.

## D-001 — Adopt Keel v5.3.2
- Date / phase: 2026-07-30 / adoption
- Decision: Govern the existing project with Keel v5.3.2 using the portability lock and complete embedded copies.
- Why: Preserve decisions, safety boundaries, evidence, and the exact next action across future sessions.
- Alternatives rejected (and why): Informal chat memory only; it had already produced missing visual files and fragmented project context.
- Supersedes: none

## D-002 — English project documentation
- Date / phase: 2026-07-30 / adoption
- Decision: Keel artifacts and maintained project documentation use English; conversation remains Spanish and the gauge may use Spanish status labels.
- Why: The user accepted Keel's recommended token-economy default.
- Alternatives rejected (and why): Spanish-only documentation; clearer to the owner but more expensive for repeated assistant context.
- Supersedes: none

## D-003 — Private prototype with no client budget or website
- Date / phase: 2026-07-30 / adoption
- Decision: Treat the project as private and not yet licensed for distribution, with no client budget and no project website.
- Why: It is a personal hardware prototype with no installed base.
- Alternatives rejected (and why): Selecting an open-source license or public release before the hardware and safety case are validated.
- Supersedes: none

## D-004 — Route A is the target; Route B remains the reversible fallback
- Date / phase: adopted as-is from existing architecture
- Decision: Target direct sensor acquisition through protected analog front ends and an ADS1115; keep MTX-D MTS/RS-232 acquisition as a development reference and fallback.
- Why: Route A can remove the original gauge, while Route B retains Innovate's unknown conditioning and supplies synchronized reference data.
- Alternatives rejected (and why): Assuming undocumented sensor curves or immediately cutting out the MTX-D.
- Supersedes: none

## D-005 — Calibration-gated output
- Date / phase: adopted as-is from firmware and safety rules
- Decision: Keep `OIL_GAUGE_DEMO_MODE=1`; never show PSI or °C from direct sensors until real calibrations are measured and cross-validated against the MTX-D.
- Why: Innovate does not publish the pressure transfer function or thermistor curve.
- Alternatives rejected (and why): Reusing visually similar 0.5–4.5 V or 10 kΩ NTC curves; they could be wrong or electrically unsafe.
- Supersedes: none

## D-006 — Approved 50/50 oil UI is binding
- Date / phase: adopted as-is from the approved design
- Decision: The final oil screen uses the 480×480 black-background, 50/50 pressure/temperature design in `docs/UI_DESIGN.md`; the numeric values remain horizontally centered.
- Why: The user iteratively approved that exact layout, icon geometry, colors, transitions, line thickness, and warning behavior.
- Alternatives rejected (and why): The current firmware's temporary 60/40 demo renderer and unrequested adaptive layouts.
- Supersedes: none

## D-007 — Automotive warning behavior
- Date / phase: adopted as-is from the approved design
- Decision: Low-pressure warning is conditional on engine-running/RPM state and flashes at 1 Hz; RPM is never displayed. Temperature uses semantic cold/warming/optimal/hot/very-hot states and never reports a precise value below the validated 50 °C floor.
- Why: Avoid false low-pressure alarms with the engine stopped and avoid false precision outside the sensor's known display range.
- Alternatives rejected (and why): Pressure-only warning without RPM state and numeric readings below 50 °C.
- Supersedes: none

## D-008 — Codex-only native configuration
- Date / phase: 2026-07-30 / adoption
- Decision: Materialize compact nested `AGENTS.md` rules for Codex only. Do not add project agents, MCP servers, committed permissions, CI, or a pre-commit gate yet.
- Why: Codex is the currently accepted assistant; the directory does not yet contain a valid Git repository and the verified commands cannot run in this WSL session.
- Alternatives rejected (and why): Empty configuration for unused assistants, or permissions/gates that claim to work without verification.
- Supersedes: none

## D-009 — Chaining remains off
- Date / phase: 2026-07-30 / adoption
- Decision: Keel writes and shows continuation prompts, but does not automatically open or start another session.
- Why: Manual continuation preserves user supervision and Codex automatic chaining is not verified here.
- Alternatives rejected (and why): `prefill` and `start`; neither is necessary, and `start` is not supported on this Windows/WSL setup.
- Supersedes: none

## D-010 — Competitive recommendations accepted as the roadmap default
- Date / phase: 2026-07-30 / adoption
- Decision: Treat dual simultaneous values, clear units, warm-up state, configurable/fail-safe warnings, sensor-fault visibility, black OLED presentation, and calibration logging as v1. Defer peak/min-max recall, user-selectable layouts, touch configuration, and permanent data logging.
- Why: The user approved the recommended Keel package; this keeps v1 focused on reliable oil protection rather than dashboard feature breadth.
- Alternatives rejected (and why): Matching configurable dashboards feature-for-feature before the core measurement chain is proven.
- Supersedes: none

## D-011 — No custom quality rubric
- Date / phase: 2026-07-30 / adoption
- Decision: Use the mechanical specification, safety constraints, accessibility rules, and approved UI reference without an additional subjective rubric.
- Why: This is a single-purpose embedded instrument with a binding design, not a public developer API needing a taste-based review standard.
- Alternatives rejected (and why): A separate extensibility or brand rubric; it adds maintenance without improving the current safety gates.
- Supersedes: none

## D-012 — Approve Remediation Sprint 0 in full
- Date / phase: 2026-07-30 / Phase 5, Sprint 0
- Decision: Apply all seven remediation items proposed in `docs/04-adoption-audit.md`: initialize a valid local Git repository, install PlatformIO in an isolated project environment, create the eleven missing Keel artifacts, normalize the approved design references without changing their bytes, specify and test RPM-aware pressure states, and add deterministic renderer-state logic/tests.
- Why: The software verification and design contracts must be reproducible before the Waveshare board arrives.
- Alternatives rejected (and why): Partial remediation; it would leave the project unable to close gate zero or enforce the approved warning semantics.
- Supersedes: none

## D-013 — Adopt the approved legacy design without a new Design round trip
- Date / phase: 2026-07-30 / Phase 3–4 adoption bridge
- Decision: Use Keel's no-Design branch for Sprint 0. The already approved HTML/PNG reference and `docs/UI_DESIGN.md` are the canonical design evidence; the compact adopted handoff records provenance and `docs/BUILD-SPEC.md` consolidates the implementation contract without redesigning or duplicating the reference files.
- Why: The user already approved the exact 480×480 design through iterative review, so another creative design pass would add risk and cost without resolving an open design question.
- Alternatives rejected (and why): Re-run Design and risk visual drift; duplicate the reference files inside the handoff and create two competing sources.
- Supersedes: none

## D-014 — Isolate and pin the Sprint 0 PlatformIO toolchain
- Date / phase: 2026-07-30 / Phase 5, Sprint 0
- Decision: Use PlatformIO Core 6.1.19 in `.venv-platformio`, with project-specific state under `/home/marcos/.platformio-oil-gauge` and an ephemeral source mirror created by `scripts/pio`.
- Why: The stable Core version is reproducible, the system Python remains untouched, the WSL home cache is writable, and the mirror avoids PlatformIO/SCons collapsing the two adjacent spaces in the repository path.
- Alternatives rejected (and why): Global installation; it mutates the host. Storing package/build caches on the NTFS project path; it was substantially slower. Invoking `pio` directly from the repository path; SCons resolved a different path.
- Supersedes: none

## D-015 — Use the official native ESP-IDF display stack
- Date / phase: 2026-08-04 / Phase 5, graphics slice
- Decision: Build the product firmware with ESP-IDF 6.0.2, the official Waveshare `esp32_s3_touch_amoled_2_16` BSP 2.0.1, and LVGL 9.5.0. Keep PlatformIO 6.1.19 only for the existing native Unity measurement tests.
- Why: The board is now present, Waveshare publishes a first-party ESP-IDF BSP for its CO5300 display/touch hardware, and the user explicitly requested the latest ESP-IDF baseline.
- Alternatives rejected (and why): Continue the temporary Arduino_GFX scaffold; it no longer matches the requested framework and duplicates board initialization already maintained by Waveshare. Track moving dependency ranges; exact versions are required for reproducible display builds.
- Supersedes: D-014 only for the firmware build; D-014 remains active for native tests.

## D-016 — Ship the first display build in calibration-safe demo mode
- Date / phase: 2026-08-04 / Phase 5, graphics slice
- Decision: Default `CONFIG_OIL_GAUGE_DEMO_MODE=y`. Cycle seven synthetic scenes every four seconds, refresh the UI at 4 Hz, and exercise stopped, cold, low pressure, normal, hot, high, and warning/blink states. When demo mode is disabled before calibration exists, display explicit calibration-missing faults rather than derived oil values.
- Why: The received AMOLED can be visually checked without pretending the Innovate sensor curves are known.
- Alternatives rejected (and why): Read direct sensors now; their pin functions and curves remain unmeasured. Show raw voltages in the driving UI; that is not useful for visual acceptance and could be mistaken for calibrated data.
- Supersedes: none

## D-017 — Route the received board to WSL through usbipd-win
- Date / phase: 2026-08-04 / Phase 5, hardware arrival gate
- Decision: Use signed usbipd-win 5.3.0 on Windows 11 to attach only the locally
  recorded exact Espressif `303a:1001` board to Ubuntu-24.04
  as `/dev/ttyACM0`.
- Why: The native ESP-IDF toolchain and project wrappers live in WSL, while Windows
  initially exposed the board only as `COM8`. Exact VID, PID, product string, serial,
  and BUSID checks prevent another USB serial device from being selected.
- Alternatives rejected (and why): Flash an ambiguous COM port; unsafe. Recreate the
  toolchain on Windows solely for this gate; unnecessary duplication.
- Supersedes: the previously open USB-host-routing choice in the technical plan.

## D-018 — Replace the factory firmware without a complete backup
- Date / phase: 2026-08-04 / Phase 5, hardware arrival gate
- Decision: The user explicitly accepts proceeding without a complete, restorable
  factory-flash backup and authorizes replacing the factory firmware with the
  calibration-safe demo firmware only on the locally recorded exact Espressif board.
- Why: The received display must be exercised with the approved oil-gauge renderer;
  the retained two 1 MB chunks are incomplete diagnostic evidence and cannot restore
  the factory image.
- Alternatives rejected (and why): Retry the unstable backup route now; three read
  attempts already reached the Keel stop condition. Keep the factory firmware and
  defer physical verification; the user explicitly chose to continue.
- Supersedes: the flash-authorization and complete-backup blockers recorded in the
  arrival gate; D-017 remains the binding USB identity route.

## D-019 — Bypass the BSP 2.0.1 LVGL lock wrapper
- Date / phase: 2026-08-04 / Phase 5, hardware arrival gate
- Decision: Call `esp_lv_adapter_lock()` and `esp_lv_adapter_unlock()` directly from
  application code; do not call Waveshare BSP 2.0.1 `bsp_display_lock()`.
- Why: The pinned BSP declares its wrapper as `bool` but returns the adapter's
  `esp_err_t` unchanged. `ESP_OK` therefore becomes false, while a timeout becomes
  true, reversing the documented success semantics and allowing unprotected LVGL
  calls. The first physical boot exposed the resulting watchdog stall.
- Alternatives rejected (and why): Invert the BSP wrapper result in application code;
  it would encode a known vendor defect and become unsafe if the wrapper is corrected.
  Patch the managed component; it would create a local fork for one avoidable wrapper.
- Supersedes: none.

## D-020 — Serialize compressed-font software rendering
- Date / phase: 2026-08-04 / Phase 5, physical renderer correction
- Decision: Set `CONFIG_LV_DRAW_SW_DRAW_UNIT_CNT=1` while the oil-gauge renderer uses
  LVGL's compressed built-in font format.
- Why: The first physical photo shows corrupted fragments in every text class while
  icons, bars, dividers, orientation, and color blocks remain coherent. In pinned
  LVGL 9.5.0, more than one software draw unit renders on multiple threads, while
  compressed-font RLE decoding reads and writes one shared `font_fmt_rle` state in
  the LVGL global. A single draw unit removes that race without changing the approved
  UI, fonts, refresh rate, or demo states.
- Alternatives rejected (and why): Hide the fault by redrawing opaque label
  backgrounds; static labels are also corrupt, so invalidation is not the cause.
  Regenerate every font uncompressed before isolating the concurrency fault; that
  changes flash size and a second variable unnecessarily. Patch LVGL locally; the
  deterministic 4 Hz gauge does not require parallel software rendering.
- Supersedes: none.

## D-021 — Target and measure at least 60 FPS for the demo
- Date / phase: 2026-08-04 / Phase 5, Sprint 1 graphics
- Decision: Replace the four-hertz stepped demo with smooth interpolation between
  the existing seven scenes. Schedule both application updates and LVGL refreshes
  every 16 ms, retain one software draw unit while compressed fonts are present,
  and enable the pinned Espressif adapter's completed-frame FPS counter. Increase
  both semantic state labels from the custom 16 px subset to a dedicated,
  uncompressed 24 px Montserrat subset containing every required Spanish glyph,
  while preserving their right alignment and the 50/50 layout.
- Why: Marcos requires fluid motion at a measured minimum of 60 FPS and needs
  `OK`, `ÓPTIMO`, `PRESIÓN BAJA`, and related states to be readable from farther
  away. The prior 250 ms application cadence and LVGL's 33 ms default refresh
  period made both requirements impossible.
- Alternatives rejected (and why): Only shorten the application loop; LVGL would
  still cap completed display frames near 30 FPS. Restore multiple draw units;
  compressed-font parallel decoding already produced physical glyph corruption.
  Display FPS on the gauge; it would add unrequested driving UI clutter, so FPS is
  logged over USB instead.
- Supersedes: the 4 Hz cadence portion of D-016. D-016's seven scenes, demo safety
  gate, and calibration-missing behavior remain active; D-020 remains active.

## D-022 — Preserve subpixel bar motion and add refresh headroom
- Date / phase: 2026-08-04 / Phase 5, Sprint 1 hardware verification
- Decision: Render each bar as an integer solid fill plus one one-pixel leading
  edge whose opacity represents the fractional pixel. Use linear scene
  interpolation and schedule application and LVGL refresh at 15 ms.
- Why: Exact-board app `7c580fe` booted cleanly, but completed-frame windows ranged
  from 9 to 58 FPS. The adapter counts only LVGL frames that actually flush. Slow
  four-second transitions frequently rounded to the same integer bar width, while
  smooth-step easing made this worst near scene boundaries. Fractional edge opacity
  preserves visible motion every frame; 15 ms provides scheduling headroom above
  the required 60 FPS without changing the approved bar geometry or thresholds.
- Alternatives rejected (and why): Force an unrelated invisible object to redraw;
  it would inflate the counter without improving the gauge. Shorten the seven demo
  scenes; it would alter the approved state-tour timing. Restore parallel draw
  units; D-020 records the resulting font corruption.
- Supersedes: D-021 only for its 16 ms cadence and smooth-step interpolation. Its
  measured 60 FPS requirement, state labels, demo safety, and single draw unit remain.

## D-023 — Productize the repository without a golden version
- Date / phase: 2026-08-04 / Phase 5, Sprint 2 repository productization
- Decision: Use the public organization of the companion boost-gauge repository
  at commit `9ca4340` only as a structural reference. Add bilingual root entry
  points, oil-owned asset and versioned firmware namespaces, practical navigation,
  and a release-hash contract. Do not add `GOLDEN_VERSION.md` or an equivalent.
  Do not copy turbo code, sensor data, settings, diagrams, firmware, generated
  caches, visual assets or bundled libraries.
- Why: The oil project already has stronger internal engineering documentation,
  but it needs the product-facing discoverability and immutable release layout of
  a public firmware repository. A golden snapshot would duplicate the binding
  design, tests, pinned dependencies and measured hardware evidence without adding
  a safety guarantee.
- Alternatives rejected (and why): Mirror the turbo tree exactly; its Arduino
  libraries, Wokwi files, prebaked renderer and sensor packaging do not apply to
  the native ESP-IDF oil gauge. Push the current development history publicly;
  earlier commits contain the exact test-board identifier, so a public remote must
  start from a sanitized snapshot. Create a remote before choosing visibility;
  publication scope is a user decision and is not inferred.
- Supersedes: D-017 and D-018 only for public documentation of the test device:
  exact identifiers remain in ignored local evidence, while maintained tracked
  documentation uses a redacted exact-board reference. Hardware identity checking
  itself remains mandatory.

## D-024 — Publish a sanitized public repository
- Date / phase: 2026-08-04 / Phase 5, Sprint 2 repository productization
- Decision: Publish `kytos22/esp32-s3-civic-oil-gauge` with public visibility.
  Replace the local `main` history with one sanitized root commit after saving a
  recoverable private bundle under ignored local evidence; push only the new
  history. Public visibility does not select or imply an open-source license.
- Why: Marcos explicitly selected public visibility. A fresh root prevents the
  superseded exact test-board identifier in earlier commits from becoming public
  while keeping the continuing local branch aligned with the GitHub repository.
- Alternatives rejected (and why): Push the existing history; it exposes device
  evidence. Maintain unrelated private and public branches; it creates a routine
  risk of pushing the wrong branch. Infer a license; visibility and licensing are
  separate user decisions.
- Supersedes: D-023 only where remote visibility was unresolved; all structural,
  no-copy and no-golden-version constraints remain binding.

## D-025 — Link the README preview to a GitHub Pages simulator
- Date / phase: 2026-08-04 / Phase 5, Sprint 3 README simulator
- Decision: Publish the approved standalone oil-gauge HTML from `docs/` through
  GitHub Pages. Make the preview image and an explicit bilingual call-to-action in
  each root README open the live simulator. Keep one simulator source rather than
  copying its markup into a second public page.
- Why: GitHub sanitizes README content and cannot execute inline JavaScript,
  sliders or iframes. A linked Pages surface preserves the expected interaction
  while making the limitation clear to readers.
- Alternatives rejected (and why): Embed an iframe or script in README; GitHub
  removes or disables it. Replace the preview with an animation; it cannot expose
  adjustable pressure, RPM and temperature. Duplicate the simulator under a new
  path; two binding visual sources would drift.
- Supersedes: none.

## D-026 — Generate the README GIF from the approved simulator
- Date / phase: 2026-08-04 / Phase 5, Sprint 3 README simulator
- Decision: Replace the static README preview with one looping GIF generated by
  headless Edge from deterministic synthetic states of the approved standalone
  HTML. Keep the GIF in `assets/`, use it in both languages, retain the live
  simulator link and commit the generator for reproducibility.
- Why: The README gains visible motion while the frames remain faithful to the
  binding design. A generator prevents manual pixel edits and language variants
  from drifting.
- Alternatives rejected (and why): AI-generate or redraw the gauge; it would not
  be implementation evidence. Screen-record manually; it is not reproducible.
  Duplicate separate English/Spanish GIFs; the simulator itself intentionally uses
  the approved Spanish gauge labels.
- Supersedes: D-025 only for the preview media type; Pages remains the interactive
  destination because README JavaScript is still unavailable.

## D-027 — Render the README GIF continuously at 50 FPS
- Date / phase: 2026-08-04 / Phase 5, Sprint 3 README simulator
- Decision: Generate every GIF frame from an interpolated simulator state using
  the same smooth-step path concept as the firmware demo. Encode at 20 ms per
  frame (50 FPS), the practical broadly reproduced GIF cadence, with no cross-fade
  or duplicated static-only tour.
- Why: The nine-frame preview communicates states but looks stepped and does not
  represent the physical demo's continuous motion. Real per-frame HTML renders
  preserve numbers, bars, colours and semantic thresholds during movement.
- Alternatives rejected (and why): Claim 60/65 FPS in GIF; browser GIF timers are
  quantized to centiseconds and playback above 50 FPS is inconsistent. Cross-fade
  key screenshots; values and text would blur rather than transition correctly.
- Supersedes: D-026 only for frame cadence and count; its source, ownership,
  reproducibility and live-link decisions remain binding.

## D-028 — License improvements for noncommercial use
- Date / phase: 2026-08-04 / Phase 5, Sprint 4 repository licensing
- Decision: Apply the unmodified PolyForm Noncommercial License 1.0.0 to
  project-authored code, documentation and assets, with the required notice
  `Copyright 2026 Marcos Vidal`. Permit study, modification and redistribution of
  improvements for the license's noncommercial purposes. Commercial use requires
  separate permission from the copyright holder. Third-party content retains its
  own license.
- Why: Marcos explicitly wants others to improve the project but not use it
  commercially. PolyForm Noncommercial is a standardized software license whose
  official permissions include use, changes and distribution for noncommercial
  purposes.
- Alternatives rejected (and why): MIT, Apache-2.0 and GPL; all permit commercial
  use. Creative Commons BY-NC-SA; Creative Commons does not recommend its content
  licenses for software. A custom license; standardized terms are clearer and less
  likely to omit important grants, notices or remedies.
- Supersedes: D-001, D-024 and the technical-plan license status only where they
  record that no public distribution license had yet been selected.

## D-029 — Update Keel copies before deferred reconciliation
- Date / phase: 2026-08-10 / Phase 5 maintenance of project workflow
- Decision: Update the installed and both embedded Keel copies from v5.3.2 to v5.13.0, keep the project baseline at v5.3.2, and record the required post-update reconciliation as pending until the new user-choice rows are answered in one batch.
- Why: Keel's update check found v5.13.0. Its mandatory reconciliation introduces user-owned choices that must not be inferred, while the immediate sensor-wiring question is safety-critical and can be answered read-only without connecting hardware.
- Alternatives rejected (and why): Silently infer the new choices; they control pushes and public issue activity. Block the electrical safety guidance; withholding it would increase the risk of an unsafe direct connection.
- Supersedes: none.

## D-030 — Use ADS1115 only in the replacement gauge
- Date / phase: 2026-08-10 / Phase 5 direct-sensor characterization
- Decision: Implement production oil acquisition through the ADS1115 only. Do
  not include MAX3232E/TRS3232E or an embedded MTX-D serial receiver. If MTS is
  useful during calibration, connect the existing MTX-D to the laptop through
  its Innovate cable and a real RS-232 or USB-to-RS-232 interface.
- Why: Marcos will use the laptop for any serial capture and wants the new gauge
  to acquire the installed sensors directly.
- Alternatives rejected (and why): An embedded RS-232 receiver; it duplicates
  calibration equipment and would require retaining the powered MTX-D.
- Supersedes: the embedded Route B option in the prior architecture; laptop MTS
  remains a non-production calibration reference.

## D-031 — Use a non-blocking entry cue for demo pressure warning
- Date / phase: 2026-08-11 / Phase 5, Sprint 5
- Decision: Use the Waveshare board's ES8311 codec and integrated speaker to play
  one 2.2 kHz double beep (120 ms on, 90 ms off, 120 ms on) when the demo enters
  the pressure-warning state. Re-arm only after warning clears, run playback in a
  separate FreeRTOS task, and treat audio initialization failure as silent degraded
  operation rather than blocking the display.
- Why: Marcos requested an audible indication when the demo passes through warning;
  an entry-only cue exercises the speaker without continuously alarming during the
  synthetic warning scene or disturbing the measured 60 FPS renderer path.
- Alternatives rejected (and why): Continuous tone; too intrusive for a demo.
  Blocking PCM writes in the UI loop; they would stall rendering. Driving GPIO46
  as a buzzer; it is the power-amplifier enable, while audio data belongs on the
  onboard ES8311/I²S path.
- Supersedes: none.

## D-032 — Add refresh headroom for warning-audio playback
- Date / phase: 2026-08-11 / Phase 5, Sprint 5 hardware verification
- Decision: Schedule both application updates and LVGL refreshes every 14 ms while
  retaining one software draw unit, the existing seven-scene timing, and the
  approved visual behavior.
- Why: The first exact-board audio build normally measured 62–67 completed FPS,
  but periodic warning entries produced 58–59 FPS windows. Fourteen milliseconds
  adds headroom for codec playback without altering the gauge states or layout.
- Alternatives rejected (and why): Accept the average; Marcos requires a measured
  minimum of 60 FPS. Restore parallel draw units; D-020 records physical glyph
  corruption. Remove the audio cue; it is the accepted Sprint 5 behavior.
- Supersedes: D-022 only for its 15 ms cadence. Its fractional-pixel bars, linear
  interpolation, measured minimum, and single-draw-unit constraints remain active.

## D-033 — Isolate warning-audio work from UI state updates
- Date / phase: 2026-08-11 / Phase 5, Sprint 5 hardware verification
- Decision: Pin the warning-audio worker to CPU1 at priority 4 and write PCM in
  512-sample chunks. Keep the application/UI update loop on its configured CPU0
  and retain LVGL's higher priority 6 worker.
- Why: D-032 improved two warning entries to 61–64 FPS, but a longer exact-board
  run exposed a third 58 FPS window. The unpinned priority-4 audio task could run
  on CPU0 ahead of the priority-1 application loop; isolating it removes that
  contention while preserving the non-blocking double beep.
- Alternatives rejected (and why): Keep reducing the global refresh period; that
  raises constant display load instead of isolating a periodic task. Lower audio
  to priority 1 without affinity; it can still time-slice on CPU0. Remove audio;
  it is the accepted Sprint 5 behavior.
- Supersedes: D-031 only for worker affinity and PCM chunk size; its tone, timing,
  entry gate, volume policy, and silent-degradation behavior remain active.

## D-034 — Use a 13 ms cadence for repeated warning headroom
- Date / phase: 2026-08-11 / Phase 5, Sprint 5 hardware verification
- Decision: Schedule application updates and LVGL refresh every 13 ms while
  retaining D-033 audio isolation and every approved visual/audio behavior.
- Why: The first long D-033 exact-board run completed three tones. Normal windows
  were 65–72 FPS, but the third tone still produced one 59 FPS window. A 13 ms
  cadence raises nominal headroom to about 77 completed frames per second.
- Alternatives rejected (and why): Accept 59 FPS as measurement noise; the explicit
  requirement is a measured minimum of 60. Jump directly to 12 ms; 13 ms should
  provide sufficient margin with less constant display work and must be measured.
- Supersedes: D-032 for the active cadence only; D-032 remains the evidence-backed
  record of why 15 ms and then 14 ms were attempted.

## D-035 — Accept the exact-board warning-audio result
- Date / phase: 2026-08-11 / Phase 5, Sprint 5 acceptance
- Decision: Accept app `bf5c932` as the completed Sprint 5 warning-audio result
  and close AC-32 after Marcos confirmed the integrated-speaker double beep is
  physically audible.
- Why: All four flash regions were hash-verified, the retained bounded log records
  three completed tone paths and 29 consecutive 66–77 FPS windows without a panic,
  watchdog, or audio error, and the remaining physical judgment is now confirmed.
- Alternatives rejected (and why): Keep Sprint 5 open; no acceptance criterion
  remains outstanding. Treat this as calibrated vehicle-alarm proof; sensor curves
  and vehicle alarm semantics remain separately safety-gated.
- Supersedes: none.

## D-036 — Accept the indoor physical UI result
- Date / phase: 2026-08-14 / Phase 5, Sprint 1 acceptance
- Decision: Accept the flashed demo's indoor 480×480 visual match and close AC-20,
  AC-23, and Sprint 1 from the user-supplied photo and complete 60 FPS video.
- Why: The 28.423-second physical cycle shows every long and short pressure and
  temperature state, intact Spanish accents, centered values, binding icons, 9 px
  bars, pure-black presentation, and the exact 50/50 split without glyph corruption,
  clipping, or overlap. Sequential warning frames prove that the intermittent
  warning label/icon/bar are the intended 1 Hz blink while the numeric pressure
  remains visible.
- Alternatives rejected (and why): Treat blink-off frames as renewed rendering
  corruption; adjacent frames show a regular on/off sequence and all static text
  remains intact. Treat the desk capture as daylight/night or vehicle-motion proof;
  those environmental judgments were not exercised and remain open.
- Supersedes: none.

## D-037 — Use a hard 2 Hz pressure-warning blink
- Date / phase: 2026-08-14 / Phase 5, Sprint 1 visual correction
- Decision: The pressure warning icon, label, and bar use a binary 2 Hz blink:
  250 ms fully visible and 250 ms fully transparent. The numeric pressure remains
  continuously visible. Reduced-motion mode remains fixed red.
- Why: Marcos observed that the former low-opacity phase looks dotted on the
  physical AMOLED and requested a faster, cleaner warning animation. A fully
  transparent off phase avoids panel/dithering artifacts instead of presenting a
  dimmed warning as intentional content.
- Alternatives rejected (and why): Keep the 20% opacity phase; it is the reported
  visual defect. Keep the 1 Hz cycle; Marcos explicitly selected 2 Hz. Blink the
  numeric value; it would reduce the continuously readable measurement.
- Supersedes: D-007 for warning frequency and D-036 only for acceptance of the
  former warning-off appearance; their other safety and visual conclusions remain.

## D-038 — Add a long-press settings surface and separate the thermometer marks
- Date / phase: 2026-08-14 / Phase 3 design extension
- Decision: Add a settings surface opened by a 700 ms press-and-hold anywhere on
  the gauge. Its required controls are AMOLED brightness, warning-sound volume,
  and data-source mode. Redraw the oil-temperature icon with a taller thermometer
  stem and all three horizontal marks raised so no mark intersects the oil waves.
- Why: Marcos requested on-device adjustment without adding visible controls to the
  driving screen, and the current lowest thermometer mark reaches the wave stroke
  in the native geometry.
- Safety constraint: A runtime `SENSORES`/normal choice may be shown but must remain
  disabled with an explicit `CALIBRACIÓN PENDIENTE` reason until ADS1115 acquisition,
  sensor calibrations, and AC-13 through AC-18 pass. It must never turn missing or
  assumed sensor data into normal-looking values.
- Undefined by this decision: exact menu composition beyond the three required
  controls, persistence/reset behavior, interruption by an active warning, exit
  behavior, and exact revised icon coordinates. These are registered in DR-001 and
  block implementation until Marcos approves them.
- Supersedes: the `no touch UI` assumption in the adopted design for the new settings
  surface only; the driving gauge remains touch-control-free and visually unchanged
  except for the thermometer icon.

## D-039 — Accept the clean binary 2 Hz warning on the physical AMOLED
- Date / phase: 2026-08-14 / Phase 5, Sprint 1 acceptance
- Decision: Accept flashed app `002581d` as the corrected physical warning result.
  Its current icon, label, and bar blink cleanly at 2 Hz with no dotted dim phase,
  while the numeric pressure remains visible.
- Why: Marcos explicitly confirmed that the current warning now looks clean. The
  exact-board run already recorded 16 consecutive completed-frame windows at
  64–77 FPS through repeated warning entries.
- Alternatives rejected (and why): Keep the physical judgment open; the designated
  user judgment is now present. Reintroduce an opacity fade; that was the artifact
  corrected by D-037.
- Supersedes: D-036 and D-037 only where they left physical acceptance pending.

## D-040 — Approve settings v1 choices and defer manual day/night profiles
- Date / phase: 2026-08-14 / Phase 3 design extension
- Decision: Approve the DR-001 menu hierarchy and interaction proposal, including
  PSI/bar units. Keep the manual brightness slider, but defer manual `DÍA`/`NOCHE`
  profiles while automatic ambient-light sensing is evaluated. The thermometer
  stem must extend above the top of all three raised marks, and the lowest mark must
  retain clear separation from the oil waves.
- Why: Marcos approved the proposed menu, selected units, specified the final icon
  relationship, and preferred investigating automatic light detection before adding
  redundant manual profiles.
- Open boundary: Marcos also requested an optional full-screen 2 Hz warning. Whether
  its off phase may hide the pressure number remains unresolved in DR-001 and blocks
  the consolidated design handoff.
- Alternatives rejected (and why): Ship manual day/night presets now; they are
  deliberately deferred. Treat the stem and marks as merely non-overlapping; the
  stem must visibly protrude above them.
- Supersedes: D-038 where it left these menu and icon details undefined.

## D-041 — Keep pressure continuously visible in full-screen warning mode
- Date / phase: 2026-08-14 / Phase 3 design extension
- Decision: In selectable `PANTALLA 2 HZ` mode, alternate an opaque red full-screen
  field for 250 ms with the normal black gauge for 250 ms. During the red phase,
  redraw the centered pressure number in white above the field at its normal
  position. The pressure number is never hidden in either phase.
- Why: Marcos explicitly required that the number never disappear. Continuous
  pressure readability is also the existing safety and accessibility invariant.
- Alternatives rejected (and why): Blink or black out the complete screen including
  the number; this would temporarily remove the primary measurement. Make the red
  field translucent; the physical AMOLED already exposed an unwanted dotted
  attenuation artifact.
- Resolves: the final open boundary in D-040 and DR-001.

## D-042 — Accept Sprint 6 as software-complete without flashing
- Date / phase: 2026-08-14 / Phase 5, Sprint 6
- Decision: Accept commit `65ebbfa` as the software-complete settings/menu build.
  It passes 19/19 native tests, the complete Keel verifier, and an ESP-IDF 6.0.2
  clean build producing a 750,720-byte app with SHA-256
  `f2de29c39b3cf7bdc4b06e24c85f98f02b55e17f05fb3fdeecc0743e1afd65fa`.
  Do not mark the sprint physically complete and do not flash without a new explicit
  authorization.
- Why: The pure logic, integration contracts, and target compilation are evidenced,
  while touch feel, visual fidelity, NVS across reboot, sound controls, and sustained
  FPS require the exact display.
- Safety boundary: Demo mode stays enabled; `SENSORES` remains disabled as
  `CALIBRACIÓN PENDIENTE`; no ADS1115, Innovate harness, 12 V, or vehicle action is
  authorized by this decision.

## D-043 — Revise the physical Sprint 6 menu, full-screen warning, and audio edges
- Date / phase: 2026-08-14 / Phase 5, Sprint 6 physical review
- Decision: Allow `SENSORES` to be selected before acquisition/calibration, but show
  an explicit no-data state and no numeric oil values. Keep the settings menu open
  until the user closes it or a warning interrupts it; remove the inactivity
  timeout. Change only `PANTALLA` warning mode to a 0.5 Hz complete cycle: one second
  of the normal gauge and one second of a solid red warning carrying the always-
  visible pressure number plus `PELIGRO` / `PRESIÓN MUY BAJA`. Present the red phase
  as one prebuilt atomic overlay to remove the observed transition tearing. Ramp the
  warning audio waveform to and from digital zero before muting/unmuting the codec
  so the physical speaker does not produce the reported start/end puff.
- Why: Marcos physically tested app `65ebbfa` and reported that sensor mode cannot be
  selected, the menu appears to close on inactivity, the full-screen warning is too
  sparse and tears during entry, and the speaker pops at both tone edges. He also
  confirmed that settings persist across reboot.
- Safety boundary: Selecting `SENSORES` does not enable ADS1115 acquisition, assumed
  calibration, or real-looking values. It is an explicit unavailable-data screen.
  Element-only warning remains the already accepted clean 2 Hz behavior.
- Alternatives rejected (and why): Continue disabling `SENSORES`; Marcos explicitly
  requested an off/demo state now. Keep the 10-second menu timeout; it conflicts with
  the physical preference. Blink the complete red warning at 2 Hz or show only the
  number; Marcos selected 0.5 Hz and requested the danger message. Hide the pressure
  number; D-041 and the current request both require continuous visibility.
- Supersedes: D-038/D-042 for the locked sensor selector, D-040/DR-001 for automatic
  menu exit, and D-041 for full-screen warning cadence/content only. D-041's
  continuously visible pressure invariant remains.

## D-044 — Keep the codec active between warning tones
- Date / phase: 2026-08-15 / Phase 5, Sprint 6 physical-review correction
- Decision: Open and settle the ES8311 output once during audio initialization,
  leave it unmuted at digital zero, and wrap every double beep with 40 ms zero-filled
  segments in addition to the existing sample envelope. Do not toggle codec mute at
  the start or end of an individual warning tone.
- Why: The physical puff coincided with the former per-tone mute/unmute transitions.
  A continuous zero-level stream avoids abrupt analogue state changes while keeping
  the non-blocking warning worker and one-shot gate unchanged.
- Verification boundary: The source contract and complete firmware build pass, but
  absence of the physical puff still requires the exact board and explicit flash
  authorization.
- Supersedes: D-043 only where it described per-tone codec mute/unmute; its required
  zero-amplitude tone edges and all other menu/warning decisions remain unchanged.

## D-045 — Add Fahrenheit as a display unit and repair the BAR decimal glyph
- Date / phase: 2026-08-15 / Phase 5, Sprint 6 review extension
- Decision: Add a persistent Celsius/Fahrenheit temperature-unit preference. Keep
  acquisition, state bands, colors, bar normalization, thresholds, and alarms in
  canonical degrees Celsius; convert only the displayed temperature, unit, reference
  labels, and simulator readout. Below the validated 50 °C floor, Fahrenheit mode
  displays `<122 °F`. Regenerate the 96 px numeric font with the decimal-point glyph
  required by one-decimal BAR values so LVGL never substitutes its missing-glyph box.
- Why: Marcos requested Fahrenheit mode and reported a visible rectangle between the
  BAR digits on the prior physical build. Inspection found that the renderer emits a
  decimal point while the dedicated numeric font contains `-`, digits, and `<` but no
  U+002E glyph.
- Alternatives rejected (and why): Convert internal temperature thresholds to
  Fahrenheit; it would duplicate safety logic and introduce rounding boundaries.
  Draw the decimal with a smaller fallback font; it would not match the baseline or
  preserve centered numeric typography.
- Supersedes: none.

## D-046 — Keep settings resident, loop warning audio, and use full-frame QSPI draw buffers
- Date / phase: 2026-08-15 / Phase 5, Sprint 6 physical-review correction
- Decision: A pressure warning never closes an already-open settings menu. While
  settings owns the screen, the gauge renderer performs no background widget or
  warning-overlay updates; it catches up from the current sample after `VOLVER`.
  Warning audio repeats the existing ramped double beep for as long as the warning
  remains active and stops when the warning clears or warning sound is disabled.
  Keep the sound-test action as one isolated double beep. Use the BSP's public panel
  and touch primitives from a project-owned display runtime, registering two 480x480
  RGB565 PSRAM draw buffers instead of `bsp_display_start()`'s two 480x50 buffers,
  so full-screen menu and red-warning invalidations are rendered and submitted as
  one full-frame area rather than ten horizontal bands.
- Why: Marcos physically confirmed the previous checks but observed tearing on both
  the red warning and settings menu, requested a continuous warning-sound loop, and
  required settings to remain resident without rendering the gauge behind it. Code
  inspection confirms that the CO5300 is driven through `esp_lcd_panel` and
  Espressif's LVGL adapter, but the pinned Waveshare BSP selects
  `ESP_LV_ADAPTER_TEAR_AVOID_MODE_NONE` with double 50-line partial buffers.
- Hardware boundary: The board BSP and pin map expose no CO5300 TE GPIO. Therefore
  firmware cannot claim scan-synchronous tear elimination: full-frame buffering
  removes the observed banded partial refresh mechanism, but the exact AMOLED still
  requires physical judgment after a separately authorized flash. TE synchronization
  would require evidence of a physically connected panel TE output.
- Safety boundary: The warning is still evaluated while settings is visible and its
  audio loop remains non-blocking. Demo mode stays enabled; no sensor, ADS1115, 12 V,
  harness, or vehicle action is enabled.
- Supersedes: D-043/AC-33/AC-38 only where warning closed settings, and D-044/AC-32
  only where one double beep was requested per warning episode.

## D-047 — Invalidate the full-screen warning only at visibility edges
- Date / phase: 2026-08-15 / Phase 5, Sprint 6 exact-board correction
- Decision: Keep the two full-height LVGL draw buffers, but change the 480×480 red
  warning layer only when its visible/hidden state actually changes. Opening settings
  explicitly hides that layer once, then the resident-menu early return prevents
  background rendering. Log the persisted warning mode, sound state, and volume at
  boot so physical audio evidence is self-contained.
- Why: Exact-board app `728c4de` passed the write and boot gates but measured 44–56
  FPS during every full-screen warning and never scheduled the lower-priority audio
  loop. The renderer was clearing the already-clear hidden flag every 13 ms, causing
  a complete 480×480 QSPI invalidation on every frame while red was visible. NVS
  independently confirmed that warning sound was enabled at 77%, excluding a muted
  setting as the cause.
- Alternatives rejected (and why): Return to 50-line partial buffers; that restores
  the reported band tearing. Raise the panel clock above the component's 40 MHz
  default; no panel-limit evidence justifies overclocking. Raise only audio priority;
  it would mask the unnecessary display work and leave the FPS failure.
- Supersedes: D-046 only where it implied full-height buffers alone were sufficient;
  its menu, audio-loop, demo, and no-TE boundaries remain active.

## D-048 — Freeze the obscured gauge during each static red warning phase
- Date / phase: 2026-08-15 / Phase 5, Sprint 6 exact-board correction
- Decision: While the opaque full-screen warning is visible, update only its pressure
  number and do not render the completely obscured gauge underneath. Pause the
  completed-frame FPS assertion during that intentional one-second static red frame,
  resume it after two seconds of dynamic gauge rendering, and log both transitions.
- Why: Edge-gated app `7c3a7c5` proves five complete warning-audio loops, but still
  measures 45–57 FPS while red because changing bars, labels, and values behind an
  opaque 480×480 object make LVGL redraw their covered regions. A static warning has
  no 60 FPS motion to measure; the meaningful requirement is at least 60 completed
  FPS whenever the gauge is dynamically interpolating, plus physical judgment of
  each 0.5 Hz red transition.
- Alternatives rejected (and why): Keep rendering hidden content; it wastes both
  cores and lowers display/audio headroom. Count an invisible animation to preserve
  a nominal FPS number; D-022 already rejects counters that do not improve visible
  motion. Report static frames as a failure; zero content changes do not define a
  frame-rate requirement.
- Supersedes: D-047 only where edge-gating alone was expected to remove all hidden
  rendering; its 40 MHz panel-clock and no-overclock boundary remains active.

## D-049 — Evaluate the gauge at a real 50 Hz cadence without a fictitious 50 MHz QSPI clock
- Date / phase: 2026-08-15 / Phase 5, Sprint 6 performance experiment
- Decision: Change the application and LVGL refresh period from 13 ms to 20 ms and
  evaluate the physical gauge against a 50 completed-FPS target. Keep the CO5300
  QSPI request at the BSP's 40 MHz. Do not configure a nominal 50 MHz value that
  the ESP32-S3 GPSPI peripheral cannot generate.
- Why: Marcos requested a 50 MHz / 50 Hz experiment to reduce tearing. ESP-IDF
  6.0.2 shows that ESP32-S3 GPSPI uses the 80 MHz APB clock with integer divisors;
  a requested 50 MHz clock resolves to 40 MHz, while the next realizable step is
  80 MHz. Recording or logging 50 MHz would therefore be false, and 80 MHz would
  exceed the current CO5300 safety boundary. The 20 ms cadence is independently
  testable and reduces renderer scheduling pressure while matching the requested
  50 Hz presentation target.
- Alternatives rejected (and why): Set `pclk_hz` to 50 MHz; the physical clock
  remains 40 MHz. Jump to 80 MHz; it is outside the presently accepted panel limit
  and requires a separate explicit overclock decision. Retain the 13 ms cadence;
  it does not perform the requested 50 Hz comparison.
- Supersedes: D-021, D-022, D-032, and D-034 only for the active cadence/FPS target;
  historical evidence and the 40 MHz/no-TE boundaries remain unchanged.

## D-050 — Run the exact-board CO5300 QSPI bus at 80 MHz for comparison
- Date / phase: 2026-08-15 / Phase 5, Sprint 6 performance experiment
- Decision: At Marcos's explicit request, keep the application and LVGL cadence at
  20 ms / 50 FPS but replace the Waveshare BSP's 40 MHz QSPI IO configuration with
  a project-owned 80 MHz compile-time override. Test it only on the locally recorded
  exact display and retain the 40 MHz implementation as a reversible Git baseline.
- Why: The first exact-board 20 ms run spent most dynamic windows at 46–50 FPS and
  later encountered one LVGL-lock timeout followed by a long low-refresh interval.
  Doubling the physically realizable panel bus clock isolates transfer bandwidth
  from the application cadence and directly tests whether it improves the visible
  transitions. Marcos explicitly requested the 80 MHz hardware experiment after
  seeing the 40 MHz result.
- Alternatives rejected (and why): Request 50 MHz QSPI; ESP32-S3 GPSPI resolves it
  to 40 MHz. Modify the downloaded managed component; a clean build would restore
  it and invalidate its registry checksum. Return immediately to 13 ms/40 MHz; it
  would not test the newly authorized bus-speed hypothesis.
- Supersedes: D-049 only for the active QSPI clock boundary; its 20 ms/50 FPS target
  and historical 40 MHz evidence remain valid.

## D-051 — Evaluate the official CO5300 hardware-rotation and GPIO-TE path
- Date / phase: 2026-08-15 / Phase 5, Sprint 6 synchronization experiment
- Decision: At Marcos's request, replace the experimental full-frame software
  rotation and custom TE gate with the pinned CO5300 driver's hardware orientation
  API and `ESP_LV_ADAPTER_TEAR_AVOID_MODE_TE_SYNC`. Keep QSPI at the already tested
  80 MHz, configure GPIO43 as the measured TE input, use a single full-frame PSRAM
  buffer as required by adapter 0.6.3, and feed the approximately 60 Hz panel from a
  15 ms application/LVGL cadence. Add summary timing telemetry before any flash.
- Why: The current menu path serializes an approximately 15.5 ms software rotation
  with an ideal 11.52 ms full-frame QSPI transfer, so it cannot deliver a new frame
  inside every measured 16.82 ms panel period. The official path removes that CPU
  rotation and makes TE govern every full-frame transfer instead of only exact
  480x480 custom callbacks.
- Reversible boundary: This is an exact-board diagnostic candidate. The 80 MHz
  pre-TE commit remains the rollback point. No sensors, vehicle wiring, 12 V, or
  flash action is authorized by this decision.
- Alternatives rejected (and why): Change only 20 ms to 17 ms; it remains
  unsynchronized with the measured 59.46 Hz TE signal. Keep the custom rotation
  and add more timing patches; rotation plus DMA already exceeds one panel period.
  Build the four-buffer custom presenter first; the simpler official path must be
  measured before accepting that additional ownership and concurrency complexity.
- Supersedes: D-049 and D-050 only for the active application/LVGL cadence and
  display synchronization implementation. Their 40 MHz and 80 MHz hardware evidence
  remains historical rollback evidence.
- Outcome (2026-08-16): the exact authorized display received app `c09589f` and
  passed write-time plus immutable post-boot digest verification. GPIO43 TE was
  usable at 59.483 Hz and the intended official path started without a runtime
  fault, but it presented mostly 14.84–14.86 FPS with average full-frame render
  times around 61–65 ms and average flush times around 31–35 ms. This rules out the
  adapter's serialized single-buffer `TE_SYNC` path for the product target. Marcos's
  subsequent physical A/B showed the image rotated 180 degrees and restored the
  diagonal tearing that was absent from the previous version. D-051 is rejected;
  changing mirror flags alone cannot rescue a path that also fails scan-order
  presentation and throughput. The next experiment must retain native panel scan
  order, or the prior `443eb72` baseline must be restored.

## D-052 — Preserve `443eb72` as Golden Prototype 1 and isolate the D-051 failures
- Date / phase: 2026-08-16 / Phase 5, Sprint 6 synchronization investigation
- Decision: Keep commit `443eb72` as **Golden Prototype 1**, the first exact-board
  regression reference for subsequent display work. This is a Git-history reference,
  not a duplicated golden firmware tree, production release, or claim that every
  menu/red transition is tear-free. Do not discard CO5300 hardware orientation or
  GPIO-TE based only on the combined D-051 result. No new candidate or flash is
  authorized by this decision.
- Why: Source tracing proves that D-051 changed more than one independent variable.
  Waveshare initializes this exact panel with `MADCTL=0xA0`; the candidate's
  `swap_xy(true)` plus `mirror(true, false)` sequence leaves the CO5300 driver at
  `0x60`, which Waveshare maps 180 degrees opposite to `0xA0`. The observed inverted
  image therefore diagnoses the selected orientation, not hardware rotation as a
  category. Separately, adapter 0.6.3 maps `TE_SYNC` to LVGL FULL mode with one
  buffer. Every small invalidation redraws 480x480, then the bridge byte-swaps the
  complete RGB565 buffer, waits for TE, starts QSPI and waits for DMA completion
  before `flush_ready`. The recorded 61–65 ms render event includes the nested
  31–35 ms flush; it is not an additional pure-render interval. These serialized
  costs explain the roughly 67 ms transfer interval and 14.85 FPS without invoking
  hardware rotation overhead.
- Remaining uncertainty: The returned diagonal is not isolated. Changing
  `MADCTL` from `0xA0` to `0x60` reverses row and column increment directions and
  can make the host writer cross the panel reader after the TE edge, but D-051 also
  changed PARTIAL/double-buffered asynchronous presentation into FULL/single-buffer
  synchronous presentation. The CO5300 datasheet marks MADCTL D5 as don't-care,
  while driver 2.1.0 implements `swap_xy()` using the generic D5/MV mask; its QSPI
  rotation test checks only API success and does not draw/verify orientation.
- Next diagnostic, only after authorization: retain D-051's 80 MHz, GPIO43 TE,
  FULL/single-buffer and timing variables but preserve the Waveshare `0xA0`
  orientation. Add separate timestamps for pure draw, RGB565 swap, TE wait and DMA,
  plus an ISR-level physical TE-period counter and UI-state tag. This one-variable
  A/B can determine whether the diagonal follows memory write direction while the
  expected approximately 15 FPS FULL/single-buffer limit is measured independently.
- Supersedes: D-051 only where its outcome ruled out hardware orientation as a
  category or treated mirror changes as unable to isolate the failure. D-051 remains
  the valid record of the exact failed `c09589f` candidate and its hardware evidence.

## D-053 — Run the orientation-only `MADCTL=0xA0` GPIO-TE diagnostic
- Date / phase: 2026-08-16 / Phase 5, Sprint 6 synchronization investigation
- Decision: With Marcos's explicit authorization, create and exact-board test the
  first D-052 isolation candidate. Retain D-051's 80 MHz QSPI, GPIO43 rising-edge
  TE, 15 ms producer/LVGL cadence, FULL render mode, one PSRAM draw buffer and
  synchronous adapter `TE_SYNC` path. Remove only the post-initialization
  `swap_xy()`/`mirror()` calls so the Waveshare sequence remains at
  `MADCTL=0xA0`. Keep demo mode and the existing touch mapping.
- Why: This is the smallest A/B that can determine whether the 180-degree inversion
  and diagonal follow the memory-address direction without conflating that result
  with a new buffering architecture. The source contract must reject any panel
  orientation call and require an explicit `0xA0` boot statement.
- Telemetry boundary: Add a non-nested `draw` duration derived per LVGL render by
  subtracting the enclosed flush duration. Do not patch the managed adapter to split
  RGB565 swap, TE wait and DMA in this first candidate, because doing so would add
  another implementation variable. The existing boot TE probe remains the physical
  59 Hz reference; deeper adapter instrumentation follows only if this A/B cannot
  decide the diagonal.
- Safety: USB/demo-only exact board. No sensors, ADS1115, MTX-D, 12 V or vehicle.
  A build does not authorize a different board or any wiring change.
- Result: Commit `dbdc856` passed the display/audio contract 24/24, native tests
  23/23 and a clean ESP-IDF 6.0.2 build. Its 755,744-byte application has SHA-256
  `7b4ae20345c537cd7a329bc5649153babe43ca1e40bac9a0dcbec471f0e960ce`.
  The separately authorized exact-board MAC was verified before writing; esptool
  verified the hash of every written region. The boot capture confirms Waveshare `0xA0`, usable
  GPIO43 TE at 59.403 Hz and the intended adapter path. Stable dynamic windows were
  normally 14.82–14.85 FPS, with about 29–31 ms of non-nested draw and 32–36 ms of
  synchronous flush inside each 62–66 ms render event. This proves orientation was
  not the only performance problem: FULL/single-buffer `TE_SYNC` remains serialized
  even after software rotation is absent. Marcos confirmed that `0xA0` restores
  correct orientation while the diagonal remains. The address-direction change to
  `0x60` caused the inversion but was not the cause of the diagonal. Preserve this
  distinction: do not reject hardware orientation broadly, and do not continue
  shifting TE timing without measuring transfer start/completion against scanout.

## D-054 — Present immutable native-scan frames and rotate with LVGL
- Date / phase: 2026-08-16 / Phase 5, Sprint 6 synchronization correction
- Decision: Replace the rejected FULL/single-buffer adapter `TE_SYNC` path with a
  project-owned QSPI presenter. Restore the CO5300 to native `MADCTL=0x00`, retain
  the approved upright appearance with LVGL `LV_DISPLAY_ROTATION_270`, and render
  through two 120-row PARTIAL draw buffers. Rotate dirty areas into one canonical
  native-order RGB565 framebuffer. At the last LVGL flush, copy that coherent
  frame into one of two independently owned transmit snapshots. A dedicated task
  may start only the newest READY snapshot on a GPIO43 TE rising edge, may never
  modify an IN_FLIGHT snapshot, and may release it only from the ESP LCD
  `on_color_trans_done` signal. Allocate both snapshots as 64-byte-aligned,
  external-DMA-capable PSRAM and enable ESP LCD's `psram_dma_direct` path so no
  hidden full-frame bounce copy begins after TE. Keep 80 MHz QSPI and the 15 ms producer cadence so
  this correction changes scan order and ownership rather than the already measured
  bus profile.
- Why: D-053 proves `0xA0` restores orientation but not the diagonal, while the
  official Espressif LCD FAQ identifies diagonal tearing after SPI hardware
  rotation by 90/270 degrees and prescribes LVGL software rotation. LVGL 9.5
  documents PARTIAL rendering plus `lv_display_rotate_area()` and
  `lv_draw_sw_rotate()` for that case. The pinned adapter cannot provide the needed
  combination: its QSPI `TE_SYNC` bridge forces FULL, one buffer, in-place RGB565
  swap, TE wait and DMA completion before `flush_ready`. ESP-IDF 6.0.2 separately
  requires the color buffer to remain alive until `on_color_trans_done`.
- Ownership invariant: LVGL writes only the canonical framebuffer. The presenter
  reads only a READY snapshot after atomically changing it to IN_FLIGHT. A new
  complete render may overwrite an older READY snapshot but never an IN_FLIGHT one;
  the presenter drops every older READY generation before sending the newest. This
  prevents partial-frame snapshots, use-after-DMA-buffer reuse and out-of-order
  presentation.
- Timing boundary: GPIO43 remains the only physical presentation clock. The panel
  measured about 59.4 Hz, so the honest physical ceiling is about 59.4 unique
  frames/s rather than a literal 60. Telemetry must separately report completed
  LVGL frames, snapshots, presented frames, overwritten/dropped generations, TE
  edges, DMA duration and presentation interval. No LVGL FPS counter alone may be
  accepted as proof.
- Safety and verification: USB/demo-only; `CONFIG_OIL_GAUGE_DEMO_MODE=y` remains
  mandatory. Add a deterministic slot-ownership regression, source contract,
  native suite and complete ESP-IDF build. Clean commit `50dee93` produces a
  734,816-byte app with SHA-256
  `d02ba8f1a9a5cb819a7fa63b6d05c6eae859a842a18e2d543b365aeb5b6fabc1`.
  A build does not authorize flashing;
  exact-board orientation, touch mapping, diagonal removal and menu/red-transition
  smoothness remain HARDWARE/JUDGMENT and require a new explicit authorization.
- Alternatives rejected (and why): Keep shifting the TE phase with `MADCTL=0xA0`;
  it does not correct the orthogonal hardware write/scan directions. Send each
  rotated PARTIAL strip directly; those become native vertical strips and can expose
  multiple updates within one scan. Reuse one framebuffer for render and DMA; ESP
  LCD explicitly forbids recycling it before transfer completion. Keep adapter
  `TE_SYNC`; D-051/D-053 measured its serialized approximately 14.8 FPS result.
- Supersedes: D-051 and D-053 for the active display implementation. Their exact
  hardware evidence remains the reason for this architecture; `443eb72` remains
  Golden Prototype 1 and the rollback reference.

## D-055 — Stage 80 MHz QSPI through bounded internal DMA buffers
- Date / phase: 2026-08-16 / Phase 5, Sprint 6 hardware-failure correction
- Decision: Preserve D-054's native `MADCTL=0x00`, LVGL 270-degree PARTIAL
  software rotation, canonical frame, two immutable snapshots and GPIO43 TE
  presenter. Replace only the unsafe direct PSRAM-to-GPSPI leg: set
  `psram_dma_direct=false`, cap ESP LCD transfers to eight RGB565 rows (7,680
  bytes), and allow three queued transactions (23,040 bytes of temporary internal
  DMA data at most). Keep QSPI at 80 MHz for the controlled A/B. If draw start or
  completion fails, latch `fatal=1` and stop the presenter; do not retry a panel-IO
  queue after `ESP_ERR_INVALID_STATE`.
- Evidence: the exact authorized D-054 flash passed all four write-time hashes and
  booted app `50dee93`, native scan and 59.434 Hz TE. The first color transfer then
  logged ESP-IDF's `DMA TX underflow detected`, ESP LCD returned
  `ESP_ERR_INVALID_STATE`, and every captured window remained at 0 completed FPS.
  ESP-IDF's SPI Master guide states that direct PSRAM DMA shares MSPI bandwidth
  and can lose data when GPSPI bandwidth is too high; its own ESP32-S3 test limits
  the direct path and checks the TX-fail flag. Pointer capability therefore did
  not validate D-054's throughput assumption.
- Verification: red was the absent transfer-profile compile failure plus 6/13
  source contract. Green is native 27/27, QSPI 13/13, display/audio 34/34 and a
  clean ESP-IDF 6.0.2 build. Commit `aa38f5f` produces a 734,896-byte app with
  SHA-256
  `92392058e67e0dde440f805f159e98c60754dca4c83164ddf87aa03dc3d6065a`.
  The separately authorized exact-board run passed all four write-time hashes and
  booted app `aa38f5f`. Its 30-second capture maintained
  `timeouts=0 errors=0 no_slot=0 fatal=0`, with 13.0–13.5 ms DMA transfers and
  load-dependent completed presentation at about 17–35 FPS. This validates the
  bounded transport correction but does not decide the diagonal, orientation,
  touch mapping or perceived scroll smoothness.
- Sources: [ESP-IDF 6.0 SPI Master — transactions with data on PSRAM](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/spi_master.html#transactions-with-data-on-psram),
  [ESP-IDF v6.0.2 direct-PSRAM transaction test](https://github.com/espressif/esp-idf/blob/v6.0.2/components/esp_driver_spi/test_apps/master/main/test_spi_master.c#L2091-L2168),
  and the exact-board capture at
  `.artifacts/hardware/2026-08-16/d054-50dee93-runtime.typescript` and
  `.artifacts/hardware/2026-08-16/d055-aa38f5f-runtime.typescript`.
- Alternatives rejected (and why): lower the whole QSPI bus to 40 MHz; one
  480x480 RGB565 frame then needs at least 23.04 ms of payload time and cannot fit
  the measured 16.82 ms TE period. Keep direct PSRAM DMA and merely reduce chunk
  size; the bandwidth-limited path and its data-loss mode remain active. Allocate
  a full 460,800-byte internal frame; the board does not have that internal SRAM
  budget.
- Supersedes: D-054 only for its direct PSRAM DMA transport. D-054's scan-order,
  software-rotation and immutable-buffer ownership decisions remain active;
  `443eb72` remains Golden Prototype 1.

## D-056 — Keep display and LVGL in native orientation
- Date / phase: 2026-08-16 / Phase 5, Sprint 6 orientation simplification
- Decision: Preserve D-055's native `MADCTL=0x00`, GPIO43 TE presenter, canonical
  canvas, two immutable snapshots, bounded 8-row transfers and completion-based
  ownership. Remove all LVGL display/area/pixel rotation. Copy each PARTIAL dirty
  row directly into its native canvas coordinates and perform only the panel-endian
  RGB565 byte swap. Keep touch untransformed so display and input share native
  coordinates.
- Why: Marcos physically confirmed that D-055 has no tearing or diagonal. Its UI
  is rotated 180 degrees relative to the previous desired mounting direction, but
  physical display orientation is unconstrained. Rotation therefore has no product
  value and consumes composition time; the simplest controlled A/B is no rotation
  anywhere above the controller's native scan.
- Verification: the source contract failed 31/35 before implementation and passes
  35/35 after it. Native tests pass 27/27. Clean commit `9b59722` produces a
  733,232-byte ESP-IDF 6.0.2 app with SHA-256
  `a4ef30f5c0dd974cb02360dabf537fd2d6a2575e5c4e37a61e9e6ada3dc5ebd3`.
  The separately authorized exact-board run passed all four write hashes and
  booted app `9b59722` with TE at 59.522 Hz. Its 30-second capture maintained
  `timeouts=0 errors=0 no_slot=0 fatal=0`; composition averaged about 0.3–1.1 ms,
  snapshot copies about 16–21 ms, DMA about 13.1 ms and completed presentation
  about 17–33 FPS. Marcos confirmed native orientation with USB-C on the right,
  correct touch, no tearing or diagonal, and smoother menu motion than D-055;
  menu FPS remain visibly low. The 16–21 ms full-frame snapshot copy, not native
  area composition or QSPI DMA, is the next measured performance bottleneck.
- Safety: USB/demo-only. Keep `CONFIG_OIL_GAUGE_DEMO_MODE=y`; do not connect
  sensors, ADS1115, MTX-D, 12 V or the vehicle.
- Supersedes: D-054/D-055 only for logical rotation. D-055's physically proven
  scan order, no-tearing result, bounded transport and ownership remain active;
  `443eb72` remains Golden Prototype 1.

## D-057 — Render directly into two complete panel-endian framebuffers
- Date / phase: 2026-08-16 / Phase 5, Sprint 6 performance correction
- Decision: Keep D-056's native CO5300 scan, native touch coordinates, GPIO43 TE,
  80 MHz QSPI, bounded eight-row internal DMA staging, and completion-owned
  framebuffer lifetime. Replace the PARTIAL canvas plus two copied snapshots with
  two complete PSRAM draw buffers in LVGL `FULL` mode and
  `RGB565_SWAPPED`. The flush callback queues the complete rendered buffer without
  copying it; the presenter starts it on TE and calls `lv_display_flush_ready()`
  only after `on_color_trans_done` has completed the transfer. In the same visual
  candidate, remove only the four fixed threshold notes below the bars, retain all
  live state labels, and increase both bars from 9 px to 15 px.
- Why: D-056 measured the 460,800-byte snapshot copy at 16–21 ms, longer than one
  59.5 Hz panel period. Pinned LVGL's double-buffered `DIRECT` mode copies every
  previous invalid area into the next buffer before rendering; its triple-buffer
  branch copies those areas into two off-screen buffers. Menu scroll invalidates
  the full 480×480 object, so `DIRECT` would preserve or multiply the measured
  full-frame copy. `FULL` redraws instead, allows LVGL to render the second buffer
  while the first is transferred, and removes both the snapshot copy and byte-swap
  pass.
- Verification: the revised source contract first failed 34/46, then passes 46/46.
  Native tests pass 27/27 and the complete ESP-IDF 6.0.2 build succeeds. The
  Clean commit `9febd47` produces a 731,104-byte candidate with SHA-256
  `625715cffaeca5c12100aad6754979e24e7bba5c163950b3e837e85dd9160e33`.
  Marcos authorized the exact-board flash. VID/PID, serial and ESP32-S3 MAC
  matched before every region passed esptool's write hash verification. Boot
  identifies app `9febd47`, native scan, the FULL double-buffer path and GPIO43
  TE at 59.554 Hz. A bounded capture records 13.2–14.6 ms DMA with
  `timeouts=0 errors=0 fatal=0`, but only about 26.5–29.0 completed presentations
  per second during the dynamic gauge while LVGL produces about 53–58 frames per
  two-second window; a lower-activity demo interval falls to about 16 FPS. The
  ownership and transport are valid, but the 50–60 FPS objective is not met.
  Tearing, color order, menu smoothness and the 15 px visual weight still require
  Marcos's physical judgment.
- Safety: demo-only. No sensor, ADS1115, MTX-D, 12 V, or vehicle connection. No
  flash without exact-board authorization.
- Supersedes: D-056 only for its canvas/snapshot/render-mode pipeline and 9 px bar
  geometry. D-056's accepted native orientation, touch mapping, no-tearing baseline,
  and D-055's bounded QSPI transport remain binding; `443eb72` remains Golden
  Prototype 1.

## D-058 — Preserve D-057 and render one clean rounded bar endpoint
- Date / phase: 2026-08-16 / Phase 5, Sprint 6 physical-review correction
- Decision: Mark implementation commit `9febd47` as Accepted Physical Baseline 2.
  Keep its display synchronization, buffer ownership, native orientation, warning,
  colors, labels and menu behavior unchanged. Increase both indicator bars from
  15 px to 18 px. Remove the separate one-pixel square leading-edge object and
  round the single rounded fill object's width to the nearest physical pixel.
- Why: Marcos confirmed D-057 has no tearing, the best menu motion yet, correct
  warning and colors, correct removal of only the fixed notes, and every dynamic
  indicator intact. The remaining visible defect is a halo at the moving endpoint,
  caused by D-022's fractional-opacity square overlapping a rounded fill. A 404 px
  travel already provides fine spatial steps; one rounded object gives a coherent
  antialiased cap without a square/curved transparency seam.
- Verification: the source contract rejected the old 15 px/separate-edge renderer
  at 45/48 and now passes 48/48. Native tests pass 27/27. Clean commit `c0be6df`
  produces a 730,496-byte ESP-IDF 6.0.2 app identified as
  `pb2-d057-3-gc0be6df`, with SHA-256
  `3ad8e75542bb25dbb27b3b8685e0ce9f2557a4bf5da3c0118e1d22f3d7ae415c`.
  Marcos authorized the exact-board flash. Image and exact device identity matched,
  all four write hashes passed, and boot confirmed `pb2-d057-3-gc0be6df`, native
  scan, FULL double buffering and TE at 59.491 Hz. The bounded capture reports DMA
  around 13.1–14.6 ms with `timeouts=0 errors=0 fatal=0`; cadence stays within
  D-057's already accepted exception. Physical weight and endpoint cleanliness
  remain Marcos's exact-board judgment.
- Safety: demo-only. Do not connect sensors, ADS1115, MTX-D, 12 V, or the vehicle.
- Supersedes: D-022 only for the fractional leading-edge object and D-057 only for
  15 px bar geometry. All D-057 display-pipeline and accepted physical behavior
  remains binding; `443eb72` remains Golden Prototype 1.

## D-059 — Serialize brightness with frame DMA and revise temperature bands
- Date / phase: 2026-08-16 / Phase 5, Sprint 6 physical-review correction
- Decision: Keep D-058's accepted clean single-object rounded endpoint and
  no-tearing display pipeline. Increase both bars from 18 px to 21 px. Change
  semantic temperature bands to cold below 60 °C, warming 60–75 °C, optimal
  76–95 °C, hot 96–100 °C, and very hot above 100 °C; temperatures below the
  measurable 50 °C floor still display `<50` and use the cold label. Coalesce
  rapid brightness changes into one atomic newest value and send CO5300 command
  `0x51` only from the display presenter after frame DMA completion.
- Why: Marcos accepted D-058's endpoint and no-tearing result but requested 21 px
  bars and the revised state ranges. He also reproduced intermittent lock-up when
  dragging brightness quickly. The prior path called the synchronous BSP panel-IO
  command from the main task for every collected slider update while the presenter
  could own the same QSPI IO for frame DMA. Serializing the newest-only command at
  the already-proven DMA completion boundary removes that concurrent ownership and
  avoids an arbitrary debounce timer.
- Verification: red evidence includes the old temperature-boundary native failure,
  the prior 18 px geometry, and absence of the brightness serialization contract.
  The implementation passes 27/27 native tests, 55/55 display/audio invariants and
  the complete Keel verifier. A clean ESP-IDF build and exact-board rapid-slider
  stress still remain; hardware testing requires fresh flash authorization.
- Safety: demo-only. Brightness is clamped to the existing 5–100% safe range. No
  sensors, ADS1115, MTX-D, 12 V, or vehicle connection.
- Supersedes: D-058 only for 18 px thickness, the prior semantic portions of the
  temperature bands, and the main-task live-brightness apply path. D-058's clean
  endpoint and D-057's physical display baseline remain binding.
