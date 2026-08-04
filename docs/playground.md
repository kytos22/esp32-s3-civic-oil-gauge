# Playground — Civic ESP32 Oil Gauge

Last verified: 2026-08-04 — fix commit `9b806cc` passed 12/12 native tests, project
consistency, and a complete ESP-IDF 6.0.2 build. The resulting 669,824-byte app
`701d0b4`, SHA-256
`7591c7cf5a02899252ad8404f67fa93d557c52124b93c2f76aeabd2b9f63ffbe`, was flashed
and region-verified on the exact board. Its bounded boot is clean; physical corrected
text remains a user-photo gate.

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

`⚠ unverified — HARDWARE`

The board is attached through usbipd-win 5.3.0 as `/dev/ttyACM0`, identified by
Espressif VID/PID `303a:1001` and the exact identifier retained in ignored local
hardware evidence. Serialized-font app
`701d0b4` is written, every region is verified, and its bounded boot is clean. The
screen needs one straight-on photo to confirm that the earlier corrupted glyph
fragments are gone. A full factory backup is not available because USB/IP
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
