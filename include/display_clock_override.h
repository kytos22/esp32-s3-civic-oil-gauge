#pragma once

#include "display_clock_profile.h"
#include "esp_lcd_co5300.h"

// The component's public default is 40 MHz. This project-owned override keeps
// the pinned component byte-for-byte intact while testing the user-approved
// 80 MHz clock on the exact recorded board.
#undef CO5300_PANEL_IO_QSPI_CONFIG
#define CO5300_PANEL_IO_QSPI_CONFIG(cs, cb, cb_ctx)             \
    {                                                           \
        .cs_gpio_num = cs,                                      \
        .dc_gpio_num = -1,                                      \
        .spi_mode = 0,                                          \
        .pclk_hz = OIL_GAUGE_DISPLAY_QSPI_HZ,                   \
        .trans_queue_depth = 10,                                \
        .on_color_trans_done = cb,                              \
        .user_ctx = cb_ctx,                                     \
        .lcd_cmd_bits = 32,                                     \
        .lcd_param_bits = 8,                                    \
        .flags = {                                              \
            .quad_mode = true,                                  \
        },                                                      \
    }
