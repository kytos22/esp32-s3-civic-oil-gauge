#pragma once

#include "lvgl.h"

namespace oilgauge {

struct OilDisplayRuntime {
  lv_display_t* display = nullptr;
  lv_indev_t* input = nullptr;
};

[[nodiscard]] OilDisplayRuntime startOilDisplayRuntime();

}  // namespace oilgauge
