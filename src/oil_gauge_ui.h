#pragma once

#include "gauge_core.h"
#include "gauge_settings.h"
#include "lvgl.h"

#include <cstdint>

namespace oilgauge {

inline constexpr std::uint32_t kUiFramePeriodMs = 13;

struct OilGaugeUiActions {
  GaugeSettings settings{};
  bool applySettings = false;
  bool saveSettings = false;
  bool testSound = false;
};

void createOilGaugeUi(lv_obj_t* screen,
                      const GaugeSettings& settings,
                      const GaugeSettings& defaults);

void updateOilGaugeUi(const ConvertedValue& pressure,
                      const ConvertedValue& temperature,
                      const EngineState& engine,
                      bool elementsBlinkPhaseOn,
                      bool fullScreenBlinkPhaseOn,
                      const GaugeSettings& settings);
[[nodiscard]] bool oilGaugeFullScreenWarningVisible();
[[nodiscard]] bool takeOilGaugeUiActions(OilGaugeUiActions& actions);

}  // namespace oilgauge
