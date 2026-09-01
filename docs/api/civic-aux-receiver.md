# CivicAux receiver API

## Purpose

`include/civic_aux_receiver.h` provides the hardware-independent CivicAux v1
stream parser, ambient payload decoder, acceptance policy, diagnostics, and
coherent receiver snapshot. `src/civic_aux_uart.cpp` is the ESP-IDF UART1 adapter.

## Public surfaces

### `CivicAuxFrameV1`

Decoded protocol-v1 header plus a fixed 48-byte payload array. Multibyte fields
are decoded little-endian. The type does not own dynamic memory.

### `CivicAuxFrameParser`

Consumes one byte at a time through `feed()`. Its 128-byte internal array can
hold two maximum-size frames, enabling recovery when a truncated candidate is
followed by a complete valid frame. It rejects oversized payloads, incompatible
versions, and bad CRCs while searching for the next valid `A5 5A` synchronization
pair.

### `CivicAuxReceiver`

Applies oil-target, known-source, exact-length, sensor-state, data-valid, and
freshness policy to structurally valid frames. Unknown valid types are counted
and ignored after full consumption. `HubStatus` contributes diagnostics and
continuity only; it cannot renew ambient freshness.

### `CivicAuxSnapshot`

Contains the latest ambient fields, last usable millilux and deadline, sequence
and hub uptime, local receive times, recovery generations, receiver status, and
parser/receiver counters. It is required to remain trivially copyable so the
UART adapter can publish one coherent value under its critical-section lock.

### `civicAuxCrc16CcittFalse()`

Computes CRC-16/CCITT-FALSE with polynomial `0x1021`, initial value `0xFFFF`, no
reflection, and no final XOR. `123456789` produces `0x29B1`.

## Runtime boundary

The ESP-IDF adapter installs UART1 as RX-only on GPIO44 at 115200 8N1. It uses a
fixed 64-byte read chunk and never calls LVGL, panel, NVS, or brightness code.
`latestCivicAuxSnapshot()` copies the complete published snapshot for the main
application loop.
