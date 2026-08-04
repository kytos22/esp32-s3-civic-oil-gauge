# Waveshare Arrival Checklist

## Before any vehicle connection

- [ ] Confirm exact model and PCB revision.
- [ ] Photograph board, enclosure, label, and package contents.
- [ ] Leave any supplied LiPo disconnected during bench tests.
- [ ] Measure the enclosure, depth, screw sizes, and approximate 37×34 mm spacing.
- [ ] Check USB-C, microSD, PWR, BOOT, and exposed-pad access with the enclosure fitted.
- [ ] Confirm VBUS/3V3/GND/GPIO14/15/43/44 pads can be used safely.

## First power-on

- [ ] Use USB-C 5 V only.
- [ ] Run the official factory demo before project firmware.
- [ ] Save a display photograph and serial log.
- [ ] Measure current at brightness 64, 128, and 255.
- [ ] Test touch center and corners.
- [ ] Check for pixel faults, resets, heat, or abnormal current.

## Project firmware

- [x] Restore PlatformIO and run native tests plus full build before flashing (12/12
  native tests and ESP-IDF 6.0.2 build repeated on 2026-08-04).
- [x] Obtain explicit user authorization before any flash (granted 2026-08-04 only
  for the locally recorded exact Espressif board).
- [x] Keep `CONFIG_OIL_GAUGE_DEMO_MODE=y` (software evidence 2026-08-04).
- [x] Identify and route only the locally recorded exact Espressif board to WSL as
  `/dev/ttyACM0` (usbipd-win 5.3.0, BUSID `1-6`).
- [x] Complete a restorable factory-flash backup, or explicitly accept proceeding
  without one. The user accepted proceeding without one on 2026-08-04; the current
  USB/IP route stopped a 16 MB read and a chunked retry, and two 1 MB chunks remain
  incomplete evidence only.
- [ ] Confirm clean visible `DEMO` labeling. The first physical photo showed glyph
  fragments from concurrent compressed-font rendering; serialized app `701d0b4` is
  now flashed and awaits a straight-on confirmation photo.
- [x] Save the boot/reset log (serialized app `701d0b4`, 2026-08-04; display/touch
  initialized with no error, reset, or watchdog after startup).
- [x] Save a bounded completed-frame FPS log (app `3e0298a`, 2026-08-04; eight
  consecutive windows at 65–67 FPS on the locally recorded exact board).
- [ ] Save the I²C scan.
- [ ] Confirm built-in peripheral addresses and no conflict at 0x48.
- [ ] Connect ADS1115 only to 3V3/GND/SDA15/SCL14.
- [ ] Verify all four ADC channels first at ground and through a safe 3.3 V divider.

## MTX-D remains installed

- [ ] Locate Innovate P/N 38400 cable and USB-RS232 adapter.
- [ ] Confirm LogWorks sees pressure and temperature.
- [ ] Photograph connectors before disconnecting anything.
- [ ] Back-probe pressure excitation and signal without damaging insulation.
- [ ] Measure thermistor resistance only while disconnected.
- [ ] Build a reversible adapter; never cut the original harness.

## Before a supervised vehicle test

- [ ] Obtain explicit user authorization for the exact test.
- [ ] Install a 2 A fuse near ACC.
- [ ] Verify TVS orientation and insulation.
- [ ] Verify the buck holds 5.0 V under load.
- [ ] Ensure USB and external VBUS are not connected simultaneously.
- [ ] Close the enclosure; expose no copper or terminals.
- [ ] Route wiring away from ignition, pump, alternator, and audio.
- [ ] Use a second person to supervise data; the driver does not operate the device.
