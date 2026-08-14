# Playground — Civic ESP32 Oil Gauge

Current Sprint 6 hardware evidence: 2026-08-14 — after Marcos explicitly authorized
the demo test, app `65ebbfa` was written only to the locally recorded exact display.
All write regions passed esptool digest verification; bootloader, partition table,
and the 750,720-byte application also passed post-boot verification. Retained ignored
capture `.artifacts/hardware/2026-08-14/sprint6-first-boot.typescript`, SHA-256
`78391016db981bda1f6284d51b4b56260c8ccf5b9e86232fb01ad22d2a430da8`, records one
completed warning tone and 24 consecutive 63–76 FPS windows, with no frame window
below 60 and no panic, watchdog, reset, or application error. Marcos confirmed NVS
reboot persistence. He also reported that `SENSORES` was disabled, the menu closed
on inactivity, the full-screen warning tore and lacked a message, and the speaker
puffed at tone edges. No sensor, ADS1115, MTX-D, 12 V, or vehicle connection was made.

Current unflashed correction: `SENSORES` is selectable and persistent but shows only
`--` / `SIN DATOS`; the menu has no inactivity timeout; full-screen warning uses one
prebuilt red layer with the number plus `PELIGRO` / `PRESIÓN MUY BAJA` on an
independent 0.5 Hz cycle; and the codec remains active at digital zero between
enveloped beeps. Red-first checks failed on the absent source/phase and at 0/12
review invariants; the correction passes native 21/21, settings 9/9, warning 11/11,
review 12/12, and a complete ESP-IDF 6.0.2 build. Physical proof awaits explicit
authorization to replace the current app.

Last hardware-assisted verification: 2026-08-14 — a user-supplied 28.423-second,
1920×1080/60 FPS physical demo video and companion photo confirm the indoor 480×480
visual match. Every semantic state is readable at handheld distance with intact
Spanish glyphs, centered values, no clipping or overlap. It also exposed the
dotted-looking low-opacity phase of the former 1 Hz warning blink. AC-06 and Sprint 1
are therefore reopened for a binary 2 Hz correction and new physical proof. The video SHA-256 is
`f27838f6ffb06107d2663e3cd1fef015730935794a4293358aedceede95f749b`.

Current software correction: the AC-06 renderer contract failed first at 0/6 and
then passes 6/6; the native suite passes 16/16 including the exact 250 ms boundaries.
The clean ESP-IDF 6.0.2 app `002581d` builds at 724,336 bytes with SHA-256
`4443c6c7675e17abc105a316004530963569275e03eaba23f9fa0f2d2287e6e3`.
It is now flashed on the exact authorized board. All four regions passed write-time
verification; immutable regions pass after boot. Retained capture
`.artifacts/hardware/2026-08-14/warning-2hz-002581d.typescript`, SHA-256
`6251e71a1df6f8fd38447216e068198d7b3bf9004691d9a33bd4776f26893b63`,
records a clean demo boot, two warning tones, and 16 consecutive 64–77 FPS windows.
Marcos's visual judgment of the corrected blink remains open.

Earlier hardware evidence: 2026-08-11 — the warning-audio entry gate passes 15/15
native tests; exact-board app `bf5c932` and every flash region verified. A retained
bounded log records three completed tone paths and 29 consecutive 66–77 FPS windows
without a runtime fault. Marcos confirmed the physical double beep is audible,
completing Sprint 5.

Earlier deployed/physical evidence: 2026-08-04 — the GitHub Pages simulator is published from
`main:/docs`; its 47,509-byte deployed HTML is byte-identical to the approved
source. Firmware fix commit `9b806cc` passed 12/12 native tests, project
consistency, and a complete ESP-IDF 6.0.2 build. The resulting 669,824-byte app
`701d0b4`, SHA-256
`7591c7cf5a02899252ad8404f67fa93d557c52124b93c2f76aeabd2b9f63ffbe`, was flashed
and region-verified on the exact board. Its bounded boot is clean; the corrected
physical text passed from the 2026-08-14 user photo/video.

## Software playground

This project's current runnable playground is the native measurement/state suite plus
the complete firmware compile. It is headless and does not take over the user's screen.

- Environment check: `./scripts/keel-doctor --check`
- Native tests: `./scripts/pio test -e native`
- Firmware build: `./scripts/idf build`
- Project consistency: `./scripts/keel-verify`
- Reset: `./scripts/idf fullclean` and
  `./scripts/pio test -e native --without-uploading --without-testing` only when a
  clean rebuild is required.
- Synthetic seed: native tests create calibrated `ConvertedValue` fixtures in memory;
  no vehicle or personal data is used.

The interactive HTML reference can be opened locally at
`docs/design/references/oil-gauge-design.html` or, after deployment, at
`https://kytos22.github.io/esp32-s3-civic-oil-gauge/design/references/oil-gauge-design.html`.
Move the pressure, RPM and temperature sliders and confirm that the numeric value,
bar colour/length and semantic state update. This is a design simulator, not a
source of real sensor data and not the driving firmware UI.

## Debug output

Firmware boot diagnostics use ESP-IDF's USB Serial/JTAG console. They report the
framework version, demo state, and display initialization failures, but no secrets or
personal data. A runtime log switch is planned. The current bounded monitor capture
is `.artifacts/hardware/2026-08-04/boot-demo-701d0b4.log`; it is ignored by Git, has
SHA-256 `0f986518eb6093f2b38179fbf33591fba69eda9c9bcf1f2ab4daaf102c044b50`, and
contains no application error, reset after startup, or watchdog event.

## Hardware playground

`⚠ partially verified — HARDWARE`

The board is attached through usbipd-win 5.3.0 as `/dev/ttyACM0`, identified by
Espressif VID/PID `303a:1001` and the exact identifier retained in ignored local
hardware evidence. Serialized-font app
`701d0b4` is written, every region is verified, and its bounded boot is clean. The
screen's indoor physical UI passed on 2026-08-14; daylight/night, glare, and
in-vehicle motion remain unverified. A full factory backup is not available because USB/IP
stopped both the continuous read and the chunked retry; two 1 MB chunks are not
restorable. On 2026-08-04 the user explicitly accepted that limitation and authorized
the calibration-safe demo flash only on this exact serial.
Follow `docs/ARRIVAL_CHECKLIST.md`, then capture:

1. boot/reset log;
2. I²C scan;
3. AMOLED current at several brightness levels;
4. all four ADS1115 channels on a protected bench input;
5. display-state captures;
6. MTS RX-only evidence;
7. only later, supervised vehicle evidence.

Never connect 12 V directly to VBUS, 3V3, or a GPIO; never cut the Innovate harness;
never enable calibrated output from an assumed curve.
