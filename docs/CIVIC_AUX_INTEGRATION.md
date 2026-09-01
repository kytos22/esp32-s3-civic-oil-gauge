# CivicAux ambient-light integration

## Scope and current status

The oil display can receive ambient-light measurements from the separate
`ESP32 Civic Auxiliary Hub` and use them for automatic display brightness. This
integration is complete and native-tested in software, but it has not been
flashed or validated over the physical UART link. It does not enable oil-sensor
use and `CONFIG_OIL_GAUGE_DEMO_MODE=y` remains mandatory.

The hub repository is a read-only protocol authority. This repository contains
only the receiver code and an independent byte-for-byte copy of its positive
test vectors; there are no filesystem links or build dependencies between the
two projects.

## Verified bench wiring

```text
Auxiliary Hub GPIO17 / TX
        │
        └── measured ~326 ohm series resistance ──> Oil display GPIO44 / RX

Auxiliary Hub GND ────────────────────────────────> Oil display GND
                                                    measured ~0.1 ohm

Oil display GPIO43 <────────────────────────────── panel TP3 / TE
```

- UART is one-way: the oil display configures UART1 RX only at 115200, 8N1.
- GPIO44 is the only CivicAux data input; no UART TX pin is configured.
- GPIO43 remains exclusively assigned to the CO5300 TE signal.
- The two 3.3 V rails and the two 5 V rails are not joined.
- This connection does not authorize vehicle 12 V, ACC, or engine sensors.

Do not change or resolder this wiring without concrete fault evidence.

## Protocol authority and copied vectors

The implementation was checked against the hub's `docs/PROTOCOL.md`,
`docs/OIL_INTEGRATION_GATE.md`, public protocol header/implementation, and
`test_vectors/civic_aux_v1_vectors.json`. The copied vector file is
`test/test_civic_aux/civic_aux_v1_vectors.json`; its SHA-256 is:

```text
37A25643C9247650FDF557E4099A7F627F4D148FEB49B4CE274D19FE358E16FF
```

The receiver implements protocol v1 with `A5 5A` synchronization, a 48-byte
maximum payload, little-endian multibyte fields, and CRC-16/CCITT-FALSE. Oil is
target-mask bit 0. Known messages must have their exact length and source;
unknown valid message types are consumed and ignored.

`AmbientLight` type/source `0x10` carries a 12-byte payload: sensor state, range
profile, raw ALS, raw white channel, communicated sample age, and filtered
millilux. A sample is usable only when its CRC, version, source, target, length,
data-valid flag, sensor state, communicated age, and local freshness all pass.
`valid` and `degraded` sensor states are usable; `HubStatus` never renews lux.

## Software ownership

```text
UART1 RX task (CPU0)
  -> fixed-size read chunk
  -> fixed-size streaming parser
  -> coherent snapshot under a critical-section lock
  -> main application loop
  -> automatic-brightness state machine
  -> requestOilDisplayBrightness()
  -> existing TE-serialized AMOLED presenter
```

The receive path allocates no dynamic memory after startup. The UART task never
calls LVGL, the panel driver, or a brightness function. Only the main loop can
request a brightness change, preserving display-transfer serialization.

If UART1 already has an installed driver, startup refuses the CivicAux receiver
and leaves the display on its saved manual backup; it does not choose another
UART or pin.

## Brightness modes and persistence

`AUTO` is the default for a new install, a missing NVS mode key, an invalid mode
value, and factory reset. `MANUAL` ignores lux for applied brightness. The
existing slider always edits the persistent manual value, which is also AUTO's
fallback value.

Automatic lux samples never write NVS. Mode and slider changes are saved only
through the existing explicit settings workflow.

The provisional mapping linearly interpolates over `log1p(lux)`:

| Lux | 0 | 1 | 5 | 20 | 100 | 500 | 2000 | 10000 | 30000+ |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Brightness | 5% | 7% | 12% | 20% | 35% | 50% | 65% | 85% | 100% |

This curve is a software starting point, not an in-vehicle optical calibration.

## State and recovery rules

| Condition | Result |
|---|---|
| AUTO startup or MANUAL -> AUTO | Hold saved manual backup until two new consecutive usable samples |
| Normal AUTO | Apply the mapped value from the newest fresh usable sample |
| Continued invalid traffic for 1 s | Start fallback |
| No usable ambient sample for 2 s, including communicated age | Start fallback |
| Fallback | Interpolate from the current applied value to manual backup over 1.5 s |
| Recovery | Require two consecutive new usable `AmbientLight` frames |
| Invalid frame during recovery | Break the recovery sequence |
| Hub uptime restart | Invalidate continuity and require two post-restart samples |
| 16-bit sequence wrap | Treat as continuous when the next sequence is zero |

The settings page exposes AUTO/MANUAL, the manual/backup slider, received lux,
sensor/range/freshness, mapped AUTO percentage, and applied percentage. The
approved main gauge screen is unchanged.

## Physical validation still required

Software tests do not prove the real wire, UART electrical levels, panel
stability, or display cadence. A later authorized bench session must verify:

1. the oil USB identity before any write;
2. real 5 Hz `AmbientLight` reception and diagnostics;
3. AUTO fallback/recovery with the hub connected and disconnected;
4. no panel errors or tearing while brightness changes and menus/warnings run;
5. display-cadence regression below 5% relative to the same firmware base.

COM10 currently belongs to the auxiliary hub and must never be used as the oil
display target.

## Proposed read-only USB identification

1. Do not start a flash command and keep COM10 excluded.
2. Record the Windows serial-device list with the oil display unplugged.
3. Connect only the oil display and identify the single newly appeared port by
   port name, VID/PID, USB serial where available, and physical location path.
4. Run only an `esptool` identity query on that new port and verify ESP32-S3 plus
   the previously recorded oil-board MAC `28:84:85:90:6E:1C`.
5. Record that port as the temporary candidate, then stop and request a separate,
   port-specific authorization before erasing or flashing anything.
