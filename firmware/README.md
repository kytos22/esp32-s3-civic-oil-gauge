# Versioned firmware packages

No production firmware has been released yet. Development output under `build/`
is deliberately ignored and must not be treated as a release.

Each validated release will use an immutable directory named
`firmware/<version>/` and contain, at minimum:

- `civic-oil-gauge-app.bin` — application-only image with its flash offset;
- `civic-oil-gauge-full.bin` — complete bootloader, partition table, OTA data and
  application image;
- `README.md` and `README.es.md` — exact version, compatibility, safety state,
  flashing instructions and validation evidence;
- `SHA256SUMS.txt` — checksums for every distributed file;
- a straight-on display capture or demo showing the exact packaged renderer;
- any wiring diagram applicable to that release.

Release packaging must be generated from a clean, committed tree after native
tests, the complete ESP-IDF build, image inspection and the applicable physical
hardware gates pass. Sensor-reading firmware cannot be released until measured
calibration and vehicle cutover criteria pass.

This project intentionally does not maintain `GOLDEN_VERSION.md` or a golden
firmware snapshot. Pinned dependencies, source-controlled specifications, tests,
hardware evidence and per-release hashes are the reproducibility contract.
