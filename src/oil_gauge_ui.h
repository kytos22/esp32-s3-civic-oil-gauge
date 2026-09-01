#pragma once

#include "automatic_brightness.h"
#include "display_profile.h"
#include "gauge_core.h"
#include "gauge_settings.h"
#include "lvgl.h"

#include <cstdint>

namespace oilgauge {

struct OilGaugeUiActions {
  GaugeSettings settings{};
  bool applySettings = false;
  bool saveSettings = false;
  bool testSound = false;
};

struct OilGaugeBrightnessStatus {
  bool receiverRunning = false;
  bool hasAmbientFrame = false;
  bool latestAmbientUsable = false;
  bool luxFresh = false;
  std::uint8_t sensorState = 0;
  std::uint8_t rangeProfile = 0;
  std::uint32_t filteredMillilux = 0;
  AutomaticBrightnessState automaticState =
      AutomaticBrightnessState::waitingForSamples;
  std::uint8_t automaticPercent = 0;
  std::uint8_t appliedPercent = 55;
  bool automaticPercentAvailable = false;
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

void setOilGaugeBootSplashVisible(bool visible);
void updateOilGaugeBrightnessStatus(const OilGaugeBrightnessStatus& status);
[[nodiscard]] bool oilGaugeFullScreenWarningVisible();
[[nodiscard]] bool takeOilGaugeUiActions(OilGaugeUiActions& actions);

}  // namespace oilgauge
