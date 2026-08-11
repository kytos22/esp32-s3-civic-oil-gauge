#pragma once

#include "gauge_core.h"
#include "lvgl.h"

#include <cstdint>

namespace oilgauge {

inline constexpr std::uint32_t kUiFramePeriodMs = 14;

void createOilGaugeUi(lv_obj_t* screen);

void updateOilGaugeUi(const ConvertedValue& pressure,
                      const ConvertedValue& temperature,
                      const EngineState& engine,
                      bool blinkPhaseOn,
                      bool reducedMotion);

}  // namespace oilgauge
