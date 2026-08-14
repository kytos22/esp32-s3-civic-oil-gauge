#include "gauge_settings.h"

#include <algorithm>

namespace oilgauge {

namespace {

constexpr double kBarPerPsi = 0.0689475729;

bool validPressureUnit(PressureUnit unit) {
  return unit == PressureUnit::psi || unit == PressureUnit::bar;
}

bool validTemperatureUnit(TemperatureUnit unit) {
  return unit == TemperatureUnit::celsius ||
         unit == TemperatureUnit::fahrenheit;
}

bool validWarningVisualMode(WarningVisualMode mode) {
  return mode == WarningVisualMode::elementsBlink ||
         mode == WarningVisualMode::fullScreenBlink ||
         mode == WarningVisualMode::fixed;
}

bool validDataSource(DataSource source) {
  return source == DataSource::demo || source == DataSource::sensors;
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
  if (!validTemperatureUnit(settings.temperatureUnit)) {
    settings.temperatureUnit = TemperatureUnit::celsius;
  }
  if (!validWarningVisualMode(settings.warningVisualMode)) {
    settings.warningVisualMode = WarningVisualMode::elementsBlink;
  }
  if (!validDataSource(settings.dataSource)) {
    settings.dataSource = DataSource::demo;
  }
  return settings;
}

double pressureForDisplay(double pressurePsi, PressureUnit unit) {
  return unit == PressureUnit::bar ? pressurePsi * kBarPerPsi : pressurePsi;
}

double temperatureForDisplay(double temperatureC, TemperatureUnit unit) {
  return unit == TemperatureUnit::fahrenheit
             ? temperatureC * 9.0 / 5.0 + 32.0
             : temperatureC;
}

WarningPresentation evaluateWarningPresentation(WarningVisualMode mode,
                                                bool warningActive,
                                                bool elementsBlinkPhaseOn,
                                                bool fullScreenBlinkPhaseOn) {
  WarningPresentation presentation;
  if (!warningActive) {
    return presentation;
  }

  switch (mode) {
    case WarningVisualMode::elementsBlink:
      presentation.attentionVisible = elementsBlinkPhaseOn;
      break;
    case WarningVisualMode::fullScreenBlink:
      presentation.fullScreenRedVisible = fullScreenBlinkPhaseOn;
      break;
    case WarningVisualMode::fixed:
      break;
  }
  presentation.pressureValueVisible = true;
  return presentation;
}

}  // namespace oilgauge
