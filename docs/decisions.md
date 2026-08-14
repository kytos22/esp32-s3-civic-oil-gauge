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
