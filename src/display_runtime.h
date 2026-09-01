#pragma once

#include <cstdint>

#include "lvgl.h"

namespace oilgauge {

struct OilDisplayRuntime {
  lv_display_t* display = nullptr;
  lv_indev_t* input = nullptr;
};

[[nodiscard]] OilDisplayRuntime startOilDisplayRuntime();
void startOilDisplayPresentation();
[[nodiscard]] bool oilDisplayBlockFramePending();
[[nodiscard]] bool beginOilDisplayBlockFrame(lv_obj_t* screen);
void finishOilDisplayBlockFrameAttempt();
void waitForOilDisplayWork(std::uint32_t maximumWaitMs);
[[nodiscard]] std::uint32_t oilDisplayPresentedMilliFps();
void requestOilDisplayBrightness(std::uint8_t brightnessPercent);

}  // namespace oilgauge
