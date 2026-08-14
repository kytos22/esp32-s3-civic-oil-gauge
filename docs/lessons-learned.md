# Lessons Learned — Civic ESP32 Oil Gauge

> Append-only. Add an entry only after a problem's cause and fix are known.

## L-001 — PlatformIO state escaped to a read-only home directory
- Symptom: `pio test -e native` raised `HomeDirPermissionsError` while trying to lock `/home/marcos/.platformio`.
- Cause: the Python venv was project-local, but PlatformIO's core/package state still used its default home path, which this execution environment mounted read-only.
- Fix: `scripts/pio` sets a project-specific state root at `/home/marcos/.platformio-oil-gauge`.
- Where: Phase 5, Sprint 0; `scripts/pio`.
- What failed first: installing only the CLI in `.venv-platformio`.
- Check added: `scripts/keel-verify` requires the explicit isolated state variables.
- Rule for next time: isolate both the executable environment and PlatformIO's mutable core/build/libdeps state.

## L-002 — PlatformIO collapses adjacent spaces in project paths
- Symptom: the native platform first looked for a builder and then `test/` under `Proyectos Code`, while the real path is `Proyectos  Code`.
- Cause: PlatformIO/SCons normalized the two adjacent spaces to one in generated paths.
- Fix: `scripts/pio` copies the exact source inputs to a fresh no-space directory created with `mktemp`, runs there, and removes only that generated directory on exit.
- Where: Phase 5, Sprint 0; `scripts/pio`.
- What failed first: moving only the PlatformIO core/build/libdeps directories to paths without spaces.
- Check added: the canonical wrapper is required by `scripts/keel-verify`; native and firmware commands run exclusively through it.
- Rule for next time: do not invoke PlatformIO directly from this repository path.

## L-003 — LVGL fast-memory IRAM conflicts with ESP-IDF 6.0.2's compiler
- Symptom: LVGL 9.5.0 failed with conflicting `.iram1` section attributes while warnings were treated as errors.
- Cause: `CONFIG_LV_ATTRIBUTE_FAST_MEM_USE_IRAM=y` assigned different IRAM sections to a declaration and definition under the ESP-IDF 6.0.2 GCC 15 toolchain.
- Fix: Leave the optional LVGL fast-memory IRAM attribute disabled; the complete firmware then compiled and linked successfully.
- Where: Phase 5 graphics slice; `sdkconfig.defaults`.
- What failed first: carrying the optimization setting from the vendor example into this newer exact LVGL/ESP-IDF pairing.
- Rule for next time: validate vendor performance options independently when moving to a newer compiler and keep optional placement attributes off unless measured performance requires them.

## L-004 — Full factory-flash read is not stable over the current USB/IP route (provisional)
- Symptom: a 16 MB `esptool read-flash` stopped at 17.7%; a later 1 MB chunked
  attempt read offsets 0 and 0x100000 successfully, then stopped while reading the
  block at 0x200000 with `Packet content transfer stopped`.
- Cause: unresolved. The ESP32-S3, serial number, flash size, and first two blocks
  were read consistently, but the current usbipd/WSL transport did not sustain the
  longer transfer.
- Fix: unresolved; stop after three failed attempts and do not claim that the two
  retained chunks form a restorable factory backup.
- Where: Phase 5 hardware arrival gate; ignored `.artifacts/hardware/2026-08-04/`.
- What failed first: one continuous 16 MB read; the second attempt stopped on a
  local precondition before touching hardware; the third used 1 MB blocks and still
  failed at the third block.
- Check added: none yet; the next approach requires user direction and must verify
  all 16 blocks plus the assembled 16 MB SHA-256 before calling the backup complete.
- Rule for next time: distinguish a readable USB serial endpoint from a transport
  proven stable for full-flash backup, and preserve incomplete reads only as evidence.

## L-005 — Waveshare BSP 2.0.1 reverses its documented LVGL lock result
- Symptom: the display and touch initialized, then `createOilGaugeUi()` stalled inside
  `lv_inv_area`; the task watchdog repeatedly reported CPU0 in the main task.
- Cause: BSP 2.0.1 declares `bsp_display_lock()` as `bool` but directly returns
  `esp_lv_adapter_lock()`'s `esp_err_t`. `ESP_OK` converts to false and
  `ESP_ERR_TIMEOUT` converts to true, so application logic entered LVGL after a failed
  zero-timeout lock attempt.
- Fix: use the adapter's native `esp_err_t` lock/unlock API directly, wait indefinitely
  only for initial UI construction, and use a bounded 100 ms lock for later frames.
- Where: Phase 5 hardware arrival gate; `src/main.cpp`.
- What failed first: the first physical boot after a successful verified flash; native
  tests and the complete firmware build could not exercise this runtime interaction.
- Check added: `scripts/keel-verify` rejects the BSP lock wrapper and requires the
  direct initial adapter lock plus an unlock call.
- Rule for next time: inspect the exact implementation and return type of vendor
  synchronization wrappers before relying on their API comments.

## L-006 — Low-opacity blink-off frames look dotted on the AMOLED
- Symptom: the pressure warning looked as though it were made from dots during the
  attenuated half of its blink on the physical display.
- Cause: the renderer deliberately mapped the off phase to `LV_OPA_20`, leaving a
  sparse-looking 20% composite instead of switching the warning pixels off.
- Fix: map the off phase to full transparency and use a binary 250 ms on / 250 ms
  off cycle; keep the numeric pressure continuously visible.
- Where: Phase 5, Sprint 1 slice 1.5; `src/oil_gauge_ui.cpp` and the editable HTML
  reference.
- What failed first: `scripts/keel-verify` reported 0/6 hard 2 Hz firmware/simulator
  invariants against the former 1 Hz, 20%-opacity implementation.
- Check added: the native AC-06 boundary regression checks every 250 ms transition;
  `scripts/keel-verify` requires full transparency and 2 Hz timing in firmware and
  both simulator representations.
- Rule for next time: warning-off states on this panel are binary; never substitute
  low opacity without a new physical acceptance check.
