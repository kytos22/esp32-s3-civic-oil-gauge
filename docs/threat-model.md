# Threat Model — Civic ESP32 Oil Gauge

## Assumptions

- The firmware is a personal automotive instrument prototype, not a certified safety device.
- Displayed oil data may influence driver decisions; a plausible wrong value is more dangerous than an explicit fault.
- Vehicle 12 V is electrically hostile. USB, I²C, UART, ADC, and ESP32 pins are not automotive power inputs.
- Sensor pinouts, transfer functions, and connector colors are untrusted until measured.
- CAN/OBD/MTS frames and serial channel assignments are untrusted inputs.
- The MTX-D and intact Innovate harness are the recovery path until cutover evidence passes.

## Defended controls

| Threat | Control | State | Evidence / next slice |
|---|---|---|---|
| Uncalibrated voltage shown as PSI/°C | Invalid-by-default calibration and demo/raw-only paths | IN PLACE | `include/calibration_config.h`, conversion tests, `src/main.cpp` |
| Invalid math/input silently converted | Explicit `Fault` values and input range checks | IN PLACE | `src/gauge_core.cpp`, native tests |
| Direct 12 V damage | Written prohibition and protected-supply architecture | MANUAL | AGENTS/architecture; physical implementation unverified |
| Sensor input overvoltage | Divider/protection/RC design and pre-connection measurement | TO BUILD | calibration/front-end sprint |
| Wrong pin assignment by color | Measurement/photo requirement | MANUAL | calibration procedure |
| Harness damage/loss of rollback | Reversible adapter; no cutting | MANUAL | connector/harness sprint |
| False low-pressure alarm at engine stop | RPM/engine-state gate | CORE IN PLACE | `EngineState`/`DisplayState` native boundary tests; real RPM source and renderer integration remain to build |
| Warning missed by color perception | Text + color + flashing/fixed-red fallback | CORE IN PLACE | deterministic blink/reduced-motion tests; physical renderer remains to build |
| Brownout retains plausible stale value | Full gated reboot and explicit faults | VERIFY | physical power/start test |
| Malformed MTS/CAN data accepted | Frame/channel validation and RX-only discovery | TO BUILD | MTS/CAN evidence sprint |
| Dependency/toolchain drift | Pinned PlatformIO dependencies | IN PLACE | `platformio.ini`; supply-chain audit still pending |
| Secret/private data committed | Keel staged-content scan before commits; future Git hook remains deferred by D-008 | PROCESS | valid local repository; Sprint 0 pre-commit scan required |

## Not defended

| Not defended | Consequence | If it matters |
|---|---|---|
| Automotive certification/functional-safety compliance | The device cannot be claimed as an OEM-grade or certified safety instrument | Design and test to the applicable automotive standards with qualified engineering/lab evidence |
| Load-dump/EMC/ESD performance of the final assembly | Resets, damage, or sensor noise may occur in the vehicle | Final protected PCB, transient/EMC tests, thermal validation |
| Accuracy outside measured calibration domain | Extrapolated values may be wrong | Clamp/flag outside domain and expand the calibrated dataset |
| Failure of the underlying Innovate sensors | A failed sensor cannot be made reliable in software | Detect open/short/out-of-range and retain independent mechanical/service checks |
| Compromised or malicious CAN/MTS source | A forged frame could create a false engine state/value | Source whitelisting, plausibility/rate checks, and independent direct sensor cross-check |
| Driver distraction | Brightness, animation, or touch could distract | No required touch; glanceability test; reduced-motion; night brightness limits |
| USB and vehicle 5 V connected simultaneously | Back-feed or ground-current risk | Define and verify the final power-path isolation before vehicle use |
| Consumer/runtime misuse | A future fork can bypass safety gates | Keep defaults fail-safe, document the boundary, and require a recorded decision for bypasses |
| Transitive dependency compromise | Toolchain/library code could be compromised | Minimize dependencies, review updates, record hashes/locks where supported |
| Malicious fork or misleading public binary | Users could run code that is not this project | No public release now; later sign releases and state canonical source |
