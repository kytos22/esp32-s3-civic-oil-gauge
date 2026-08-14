#include "gauge_settings.h"

#include <algorithm>

namespace oilgauge {

namespace {

constexpr double kBarPerPsi = 0.0689475729;

bool validPressureUnit(PressureUnit unit) {
  return unit == PressureUnit::psi || unit == PressureUnit::bar;
}

bool validWarningVisualMode(WarningVisualMode mode) {
  return mode == WarningVisualMode::elementsBlink ||
         mode == WarningVisualMode::fullScreenBlink ||
         mode == WarningVisualMode::fixed;
}

}  // namespace

GaugeSettings sanitizeGaugeSettings(GaugeSettings settings) {
  settings.brightnessPercent = std::clamp<std::uint8_t>(
      settings.brightnessPercent, 5, 100);
  settings.warningVolumePercent = std::clamp<std::uint8_t>(
      settings.warningVolumePercent, 5, 100);
  if (!validPressureUnit(settings.pressureUnit)) {
    settings.pressureUnit = PressureUnit::psi;
  }
  if (!validWarningVisualMode(settings.warningVisualMode)) {
    settings.warningVisualMode = WarningVisualMode::elementsBlink;
  }
  return settings;
}

double pressureForDisplay(double pressurePsi, PressureUnit unit) {
  return unit == PressureUnit::bar ? pressurePsi * kBarPerPsi : pressurePsi;
}

WarningPresentation evaluateWarningPresentation(WarningVisualMode mode,
                                                bool warningActive,
                                                bool blinkPhaseOn) {
  WarningPresentation presentation;
  if (!warningActive) {
    return presentation;
  }

  switch (mode) {
    case WarningVisualMode::elementsBlink:
      presentation.attentionVisible = blinkPhaseOn;
      break;
    case WarningVisualMode::fullScreenBlink:
      presentation.fullScreenRedVisible = blinkPhaseOn;
      break;
    case WarningVisualMode::fixed:
      break;
  }
  presentation.pressureValueVisible = true;
  return presentation;
}

}  // namespace oilgauge
