# Display Pipeline

## Scope

This document records the hardware-accepted rollback path, the rejected
experiments, and the current software candidate on
`codex/partial-v2-integration`. The rollback remains `pb5-good-base`; the new
candidate is built and unit-tested but has not yet been flashed or accepted on
the exact display.

## Hardware boundary

The 480×480 CO5300 is a QSPI panel with internal GRAM. It is not an RGB panel
whose scan engine reads an ESP32 framebuffer continuously. Every new image must
therefore cross QSPI into the panel:

```text
ESP32 PSRAM framebuffer
        │
        │ 480 × 480 × RGB565 = 460,800 bytes
        ▼
ESP LCD / QSPI DMA at 80 MHz
        │
        ▼
CO5300 internal GRAM → AMOLED scan
```

The exact board measures approximately 59.5 TE edges per second and 13–15 ms
for a complete QSPI transfer. The physical frame period is about 16.8 ms, so a
60 FPS pipeline has only 2–3 ms of non-overlapped margin.

## Hardware-accepted rollback implementation

The accepted base uses `esp_lvgl_adapter` 0.6.3 for LVGL lifecycle, locking,
timers and touch input. It does **not** use
`esp_lv_adapter_register_display()`. The project creates the LVGL display and
owns presentation itself:

- native CO5300 scan order (`MADCTL=0x00`);
- native touch coordinates;
- two complete 460,800-byte PSRAM draw buffers;
- `LV_COLOR_FORMAT_RGB565_SWAPPED`;
- LVGL `FULL` render mode;
- one project-owned flush callback;
- GPIO43 rising-edge TE presenter;
- bounded internal DMA staging through the ESP LCD driver;
- framebuffer release only after `on_color_trans_done`;
- `lv_display_flush_ready()` only after the complete transfer finishes.

This path is physically accepted: no tearing or diagonal artifact, correct
touch, zero transport faults in the bounded run, and approximately 30.5–32.7
completed presentations per second.

## Why the adapter display bridge is not used

The pinned adapter contains a QSPI `TE_SYNC` mode even though parts of the
current online documentation still describe OTHER interfaces as `NONE` only.
The 0.6.3 implementation selects `FULL`, allocates one LVGL draw buffer, waits
for TE inside the flush, starts `esp_lcd_panel_draw_bitmap()`, waits for DMA,
and only then calls `flush_ready()`.

That strict sequence serializes render, TE wait and transfer. Earlier exact-board
experiments measured approximately 14.8 FPS and also proved that TE cannot fix a
diagonal caused by a write order that does not match the panel's native scan.
Replacing the accepted presenter with the adapter bridge would therefore be a
functional and performance regression.

## Relationship to RGB double/triple buffering

The useful invariant from the LVGL community discussion is independent of
buffer count:

1. one buffer is immutable while the display consumes it;
2. another buffer receives the next render;
3. a reused buffer must contain every change since the generation it holds;
4. only a complete, coherent generation may become visible.

The accepted `FULL` renderer satisfies content coherence by rebuilding the
entire frame. It also protects the transmitted frame until DMA completion. It
does not, however, make TE the sole production clock, so render readiness and
presentation are still indirectly coordinated by the application update loop,
LVGL refresh timer and adapter lock.

An RGB panel can switch which ESP32 framebuffer is scanned without sending a
new full image. The CO5300 cannot: a host-side pointer swap never replaces the
460,800-byte QSPI transfer. The transferable part of the RGB design is buffer
ownership and scheduling, not zero-copy presentation.

## What failed in the first triple-buffer experiment

The isolated branch `codex/triple-buffer-pipeline` at `0594dba` explored:

- three persistent complete frames;
- explicit `FREE`, `RENDERING`, `READY` and `IN_FLIGHT` ownership;
- at most one READY generation;
- PARTIAL LVGL rendering into a complete-frame compositor;
- generation-tagged damage history to bring a stale destination up to date;
- TE hand-off as authorization to start the next render.

Its buffer model was directionally correct, but its LVGL scheduling was not
exclusive. Pausing the display refresh timer was insufficient because LVGL 9.5
resumes it on `LV_EVENT_REFR_REQUEST`. The adapter worker could then call
`lv_timer_handler()` and flush outside the application-owned render, producing
callbacks without an owned destination, freezes and a latched fatal state.

## Current PARTIAL v2 candidate

The candidate has been reconstructed on a dedicated branch from the accepted
rollback base. Its scheduling chain is:

```text
GPIO43 TE
   │
   ▼
READY → IN_FLIGHT; start complete QSPI DMA
   │
   └── request exactly one application-owned LVGL refresh immediately
                         │
                         ▼
                  FREE → RENDERING → READY
```

Implemented invariants:

1. Keep LVGL's display refresh timer object because LVGL 9.5 implements
   `lv_refr_now()` by calling that timer directly; deleting it would make manual
   refresh a no-op.
2. Hold the timer paused and register a later `LV_EVENT_REFR_REQUEST` callback
   that immediately pauses it again after LVGL's built-in callback resumes it.
   This removes the adapter worker as a second display-refresh authority while
   retaining touch and non-display timers.
3. Execute every manual `lv_refr_now()` under the adapter lock from the single
   application owner.
4. Keep three persistent full PSRAM canvases with explicit `FREE`, `RENDERING`,
   `READY` and `IN_FLIGHT` ownership. At most one slot may be READY.
5. At TE, atomically hand READY to IN_FLIGHT, start its complete QSPI transfer,
   and immediately wake the producer for the next generation. Rendering can
   overlap DMA but never modifies the in-flight canvas.
6. Give LVGL one 480×32 `PARTIAL` draw buffer. Each flush copies its completed
   block synchronously into the owned full canvas and then releases only the
   small LVGL buffer.
7. Mark a generation READY only after `lv_display_flush_is_last()` has been
   observed. A render with no terminal flush is discarded and retried.
8. Track 32×32 damage tiles for eight generations. When a stale free canvas is
   reused, invalidate its new damage plus every generation it missed; if the
   history is incomplete, force a safe full redraw.
9. Continue transferring one immutable full frame at TE. LVGL render work is
   partial, but physical multi-window updates are deliberately avoided because
   they would expose intermediate panel-GRAM states.
10. ESP-IDF 6.0.2 splits a large PSRAM color transfer into bounded SPI chunks,
    but enables `on_color_trans_done` only on the final chunk. The DMA semaphore
    therefore releases the IN_FLIGHT canvas only after the whole 480×480 bitmap.
11. Record physical completed FPS, TE edges, render time, producer wake latency,
    READY wait, DMA time, damage tiles, partial bytes, timeouts and ownership
    faults. LVGL render FPS alone is not presentation evidence.

Native tests cover ownership selection, stale-canvas reconstruction and the
terminal-flush rule. The complete ESP-IDF 6.0.2 / LVGL 9.5.0 build succeeds.
Flashing and exact-board validation still require separate authorization; the
first run must check boot stability, menu scroll, full-screen warning, tearing,
transport faults and measured presentation FPS before this can replace the
rollback base.

## Primary references

- [Espressif ESP LVGL Adapter](https://docs.espressif.com/projects/esp-iot-solution/en/latest/display/tools/esp_lvgl_adapter.html)
- [Espressif component 0.6.3](https://components.espressif.com/components/espressif/esp_lvgl_adapter/versions/0.6.3/readme?language=en)
- [LVGL forum buffer-sequencing discussion](https://forum.lvgl.io/t/lvgl-double-buffering-with-esp32-s3-rgb-panel-how-to-draw-asynchronously-to-non-visible-buffer/23410/16)
- [LVGL display refreshing](https://docs.lvgl.io/9.5/details/main-modules/display/refreshing.html)
- [ESP-IDF LCD API](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/lcd/index.html)
