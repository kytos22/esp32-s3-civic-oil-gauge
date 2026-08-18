#pragma once

#include <cstdint>

#include "lvgl.h"

namespace oilgauge {

struct OilDisplayRuntime {
  lv_display_t* display = nullptr;
  lv_indev_t* input = nullptr;
};

[[nodiscard]] OilDisplayRuntime startOilDisplayRuntime();
void requestOilDisplayBrightness(std::uint8_t brightnessPercent);

}  // namespace oilgauge
