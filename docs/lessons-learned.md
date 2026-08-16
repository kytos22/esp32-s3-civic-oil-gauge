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

## L-007 — OTA selection data changes on the first boot
- Symptom: post-boot `verify-flash` matched bootloader, partition table, and app but
  reported a digest mismatch for `ota_data_initial.bin`.
- Cause: `ota_data_initial.bin` is an empty initialization image; the bootloader
  updates the mutable `otadata` partition when it selects app0 on first boot.
- Fix: require write-time verification for all four images, then verify the immutable
  bootloader, partition table, and app after boot. Record the expected `otadata`
  mutation instead of reflashing it in a loop.
- Where: Phase 5, Sprint 1 slice 1.5 exact-board flash.
- What failed first: a post-boot four-region comparison after every region had
  already passed esptool's write-time hash verification.
- Check added: the AC-06/AC-22 evidence rows now distinguish write-time verification
  from post-boot immutable-region verification.
- Rule for next time: never use an initial OTA-data blob as an immutable post-boot
  reference.

## L-008 — TE cannot correct a scan-order mismatch caused by panel rotation
- Symptom: the first GPIO43 TE candidate was rotated 180 degrees; correcting its
  CO5300 `MADCTL` value to the upright `0xA0` orientation retained one diagonal
  tear, while Golden Prototype 1 had no diagonal without TE synchronization.
- Cause: the full frame was synchronized to vertical blank but transmitted in the
  address order produced by the controller's 90/270-degree hardware rotation. That
  write wave no longer follows the panel's native refresh scan, so waiting for TE
  alone cannot prevent the two waves from crossing diagonally. The synchronous
  adapter path also serialized software work and the LCD transfer, reducing the
  measured presentation rate to about 14.8 FPS.
- Fix: keep the CO5300 in native `MADCTL=0x00`, use LVGL's documented PARTIAL
  software-rotation path, assemble a canonical native-order frame, copy complete
  generations into two immutable PSRAM snapshots, and let a dedicated
  presenter start only the newest READY snapshot at the next GPIO43 TE edge. A
  slot remains IN_FLIGHT until `on_color_trans_done`. D-055 stages bounded chunks
  through internal DMA after D-054's direct PSRAM transfer failed; exact-board
  visual acceptance remains pending.
- Where: Phase 5, Sprint 6 slice 6.15; `src/display_runtime.cpp`,
  `src/frame_slot_policy.h`, and `include/display_clock_override.h`.
- What failed first: treating hardware rotation as an independent orientation
  detail, then using the adapter's FULL/single-buffer TE mode without accounting
  for native scan direction, asynchronous LCD-buffer ownership, or a possible
  post-TE PSRAM staging copy.
- Check added: three AC-41 native ownership regressions, 34 display/audio source
  invariants, bounded QSPI staging checks, and two-second telemetry
  for completed presentations, TE edges, rotation, snapshot and DMA durations,
  dropped generations, timeouts, transfer errors, and unavailable slots.
- Rule for next time: establish panel scan order and buffer lifetime before tuning
  clocks or phase offsets; never infer tear-free behavior from a TE wait, an LVGL
  FPS counter, or correct visual orientation alone.

## L-009 — Direct PSRAM DMA is not a zero-cost full-frame path at 80 MHz QSPI
- Symptom: D-054 booted with the intended native scan and TE signal, then the
  first full-frame transfer logged `DMA TX underflow detected`; ESP LCD surfaced
  `ESP_ERR_INVALID_STATE`, and completed presentation stayed at 0 FPS.
- Cause: `SPI_TRANS_DMA_USE_PSRAM` shares MSPI bandwidth with the running system.
  ESP-IDF explicitly warns that GPSPI bandwidth must remain below PSRAM bandwidth
  or data can be lost. The D-054 design treated pointer capability and alignment
  as sufficient proof, but neither proves sustained bandwidth at 80 MHz QSPI.
- Fix: keep the immutable PSRAM snapshot, disable direct PSRAM DMA, and let the
  official SPI/LCD path stage 8-row chunks through three internal DMA buffers
  (23,040 bytes maximum) while QSPI remains at 80 MHz. Latch and expose presenter
  failure instead of retrying a transaction queue whose accounting may already be
  poisoned.
- Where: Phase 5, Sprint 6 slice 6.16; `include/display_clock_profile.h`,
  `include/display_clock_override.h`, and `src/display_runtime.cpp`.
- What failed first: assuming `esp_ptr_dma_ext_capable()` meant the transfer was
  operational, and accepting a callback/ownership unit test without a real first
  full-frame DMA test.
- Check added: an AC-41 native bounded-memory profile, 13 source invariants for
  the 80 MHz bounce path, a persistent `fatal` telemetry field, and exact-board
  acceptance that begins with non-zero completed presentations and zero errors.
- Rule for next time: distinguish addressability from bandwidth; for any direct
  external-memory DMA mode, read its loss conditions and prove one complete
  transfer on hardware before optimizing the surrounding pipeline.

## L-010 — LVGL DIRECT buffering can hide full-frame synchronization copies
- Symptom: D-056 removed rotation work but menu presentation remained about
  17–33 FPS, with each full snapshot copy alone measuring 16–21 ms.
- Cause: replacing the explicit snapshot with LVGL `DIRECT` double/triple buffering
  would not eliminate that class of work. Pinned LVGL 9.5 records invalidated areas
  and copies them to the next off-screen buffer before rendering; its triple-buffer
  path also synchronizes the second off-screen buffer. Menu scrolling invalidates
  the complete 480×480 menu object.
- Fix: use two full-screen `RGB565_SWAPPED` buffers in `FULL` mode, queue the
  rendered pointer directly, and release it only after LCD DMA completion.
- Where: Phase 5, Sprint 6 slice 6.18; `src/display_runtime.cpp` and pinned
  `managed_components/lvgl__lvgl/src/core/lv_refr.c`.
- What failed first: the initial D-057 plan favored `DIRECT` triple buffering before
  tracing `refr_sync_areas()` against the menu's full-object invalidation behavior.
- Check added: the 46-row display contract requires `FULL`, two complete buffers,
  panel-endian rendering, no canvas/snapshot copy, and DMA-completion flush release.
- Rule for next time: inspect a framework's buffer-synchronization path using the
  application's real invalidation areas before assuming that direct rendering means
  zero copies.

## L-011 — Exposed USB pads sit immediately beside the expansion I²C pads
- Symptom: after wiring the ADS1115, Windows no longer enumerated the ESP32-S3 and
  usbipd showed only a persisted, disconnected COM entry.
- Cause: ADS1115 SDA/SCL had accidentally occupied P4/P5, which are GPIO19/20 native
  USB D−/D+, instead of P6/P7, which are GPIO14/15 I²C SCL/SDA.
- Fix: with power removed, move the two data leads to P6/P7. The exact `303a:1001`
  USB device immediately returned and the ADS1115 then probed successfully at 0x48.
- Where: Phase 5, Sprint 7 bare-ADS1115 bench bring-up.
- What failed first: relying on pad proximity without rechecking the official
  schematic and the project's P1–P9 map before applying USB power.
- Check added: the pinout names both adjacent USB and I²C pairs; AC-42 requires
  exact USB identity before flash and a successful 0x48 probe after boot.
- Rule for next time: identify P6/P7 by both position and signal; if uncertain,
  verify their approximately 2.2 kΩ pull-up path to 3.3 V with power removed.
