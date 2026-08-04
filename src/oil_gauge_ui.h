#pragma once

#include "gauge_core.h"
#include "lvgl.h"

namespace oilgauge {

void createOilGaugeUi(lv_obj_t* screen);

void updateOilGaugeUi(const ConvertedValue& pressure,
                      const ConvertedValue& temperature,
                      const EngineState& engine,
                      bool blinkPhaseOn,
                      bool reducedMotion);

}  // namespace oilgauge
