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
