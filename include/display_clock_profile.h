#pragma once

// Experimental exact-board CO5300 QSPI clock requested by D-050.
// Kept C-compatible because the Waveshare BSP is compiled as C.
#define OIL_GAUGE_DISPLAY_QSPI_HZ (80 * 1000 * 1000)

// ESP-IDF explicitly limits direct PSRAM-to-GPSPI DMA by the available MSPI
// bandwidth. At 80 MHz QSPI the exact board reports TX underflow, so the LCD
// driver stages bounded chunks in internal DMA memory instead. Three queued
// eight-row chunks consume 23,040 bytes at RGB565.
#define OIL_GAUGE_DISPLAY_TRANSFER_ROWS (8)
#define OIL_GAUGE_DISPLAY_QUEUE_DEPTH (3)
