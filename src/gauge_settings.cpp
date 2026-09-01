#include "gauge_settings.h"

#include <algorithm>
#include <cmath>
#include <utility>

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

bool validBrightnessMode(BrightnessMode mode) {
  return mode == BrightnessMode::automatic || mode == BrightnessMode::manual;
}

bool validUiLanguage(UiLanguage language) {
  return language == UiLanguage::spanish || language == UiLanguage::english;
}

}  // namespace

GaugeSettings sanitizeGaugeSettings(GaugeSettings settings) {
  settings.brightnessPercent = std::clamp<std::uint8_t>(
      settings.brightnessPercent, 5, 100);
  settings.automaticBrightnessMinimumPercent = std::clamp<std::uint8_t>(
      settings.automaticBrightnessMinimumPercent, 5, 100);
  settings.automaticBrightnessMaximumPercent = std::clamp<std::uint8_t>(
      settings.automaticBrightnessMaximumPercent, 5, 100);
  if (settings.automaticBrightnessMinimumPercent >
      settings.automaticBrightnessMaximumPercent) {
    std::swap(settings.automaticBrightnessMinimumPercent,
              settings.automaticBrightnessMaximumPercent);
  }
  settings.automaticBrightnessBiasPercent = std::clamp<std::int8_t>(
      settings.automaticBrightnessBiasPercent,
      kAutomaticBrightnessBiasMinimum,
      kAutomaticBrightnessBiasMaximum);
  settings.warningVolumePercent = std::clamp<std::uint8_t>(
      settings.warningVolumePercent, 5, 100);
  settings.lowPressureWarningPsi = std::clamp<std::uint8_t>(
      settings.lowPressureWarningPsi, 1, 30);
  settings.highTemperatureWarningCelsius = std::clamp<std::uint8_t>(
      settings.highTemperatureWarningCelsius, 110, 140);
  settings.startupLogoSeconds = std::min<std::uint8_t>(
      settings.startupLogoSeconds, 10);
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
  if (!validBrightnessMode(settings.brightnessMode)) {
    settings.brightnessMode = BrightnessMode::automatic;
  }
  if (!validUiLanguage(settings.language)) {
    settings.language = UiLanguage::spanish;
  }
  return settings;
}

BrightnessMode brightnessModeFromStoredValue(bool keyPresent,
                                             std::uint8_t storedValue) {
  if (!keyPresent) {
    return BrightnessMode::automatic;
  }
  const auto mode = static_cast<BrightnessMode>(storedValue);
  return validBrightnessMode(mode) ? mode : BrightnessMode::automatic;
}

std::uint8_t brightnessModeStoredValue(BrightnessMode mode) {
  return static_cast<std::uint8_t>(
      validBrightnessMode(mode) ? mode : BrightnessMode::automatic);
}

double pressureForDisplay(double pressurePsi, PressureUnit unit) {
  return unit == PressureUnit::bar ? pressurePsi * kBarPerPsi : pressurePsi;
}

double warningThresholdForDisplay(std::uint8_t pressurePsi,
                                  PressureUnit unit) {
  return pressureForDisplay(pressurePsi, unit);
}

std::uint8_t warningThresholdPsiFromDisplay(double value, PressureUnit unit) {
  const double psi = unit == PressureUnit::bar ? value / kBarPerPsi : value;
  return static_cast<std::uint8_t>(
      std::clamp<long>(std::lround(psi), 1L, 30L));
}

double temperatureForDisplay(double temperatureC, TemperatureUnit unit) {
  return unit == TemperatureUnit::fahrenheit
             ? temperatureC * 9.0 / 5.0 + 32.0
             : temperatureC;
}

double temperatureWarningThresholdForDisplay(
    std::uint8_t temperatureC,
    TemperatureUnit unit) {
  return temperatureForDisplay(
      std::clamp<std::uint8_t>(temperatureC, 110, 140), unit);
}

std::uint8_t temperatureWarningThresholdCelsiusFromDisplay(
    double value,
    TemperatureUnit unit) {
  const double celsius = unit == TemperatureUnit::fahrenheit
                             ? (value - 32.0) * 5.0 / 9.0
                             : value;
  return static_cast<std::uint8_t>(
      std::clamp<long>(std::lround(celsius), 110L, 140L));
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
