# Display Pipeline

## Scope

This document records the as-built display path, the rejected experiments, and
the next safe performance architecture. It is descriptive only: the accepted
firmware remains `pb5-good-base`, and no display code changes are authorized by
this document.

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

## Accepted implementation

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

## Parked triple-buffer experiment

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

## Next experimental architecture

The next display experiment must start from the parked branch, never from the
accepted feature branch, and establish one refresh authority before restoring
the compositor:

```text
GPIO43 TE
   │
   ▼
READY → IN_FLIGHT; start complete QSPI DMA
   │
   └── authorize exactly one application-owned LVGL refresh
                         │
                         ▼
                  FREE → RENDERING → READY
```

Required invariants:

1. Delete the automatic display refresh timer with
   `lv_display_delete_refr_timer()`; pausing is forbidden.
2. Keep the adapter worker only for touch and non-display LVGL timers.
3. Execute manual display refresh under the adapter lock from one application
   owner; never concurrently with `lv_timer_handler()`.
4. TE hand-off, not an independent 15 ms presentation timer, grants permission
   for the next frame.
5. Keep one immutable IN_FLIGHT frame until `on_color_trans_done`.
6. Keep at most one complete READY frame; never overwrite it.
7. If PARTIAL composition is retained, apply every damage generation missing
   from the selected destination before its new changes.
8. Record TE, render start/end, READY wait, DMA start/end, missed TE windows,
   ownership violations and physical presentations. LVGL FPS alone is not
   presentation evidence.

Only after the sole-scheduler invariant passes native/source tests should a new
candidate be built. Flashing and exact-board validation still require separate
authorization.

## Primary references

- [Espressif ESP LVGL Adapter](https://docs.espressif.com/projects/esp-iot-solution/en/latest/display/tools/esp_lvgl_adapter.html)
- [Espressif component 0.6.3](https://components.espressif.com/components/espressif/esp_lvgl_adapter/versions/0.6.3/readme?language=en)
- [LVGL forum buffer-sequencing discussion](https://forum.lvgl.io/t/lvgl-double-buffering-with-esp32-s3-rgb-panel-how-to-draw-asynchronously-to-non-visible-buffer/23410/16)
- [LVGL display refreshing](https://docs.lvgl.io/9.5/details/main-modules/display/refreshing.html)
- [ESP-IDF LCD API](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/lcd/index.html)
