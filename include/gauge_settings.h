#pragma once

#include <cstdint>

namespace oilgauge {

enum class PressureUnit : std::uint8_t {
  psi = 0,
  bar = 1,
};

enum class TemperatureUnit : std::uint8_t {
  celsius = 0,
  fahrenheit = 1,
};

enum class WarningVisualMode : std::uint8_t {
  elementsBlink = 0,
  fullScreenBlink = 1,
  fixed = 2,
};

enum class DataSource : std::uint8_t {
  demo = 0,
  sensors = 1,
};

struct GaugeSettings {
  std::uint8_t brightnessPercent = 55;
  bool warningSoundEnabled = true;
  std::uint8_t warningVolumePercent = 35;
  std::uint8_t lowPressureWarningPsi = 10;
  std::uint8_t startupLogoSeconds = 1;
  PressureUnit pressureUnit = PressureUnit::psi;
  TemperatureUnit temperatureUnit = TemperatureUnit::celsius;
  WarningVisualMode warningVisualMode = WarningVisualMode::elementsBlink;
  DataSource dataSource = DataSource::demo;
};

struct WarningPresentation {
  bool attentionVisible = true;
  bool fullScreenRedVisible = false;
  bool pressureValueVisible = true;
};

[[nodiscard]] GaugeSettings sanitizeGaugeSettings(GaugeSettings settings);
[[nodiscard]] double pressureForDisplay(double pressurePsi,
                                        PressureUnit unit);
[[nodiscard]] double warningThresholdForDisplay(std::uint8_t pressurePsi,
                                                PressureUnit unit);
[[nodiscard]] std::uint8_t warningThresholdPsiFromDisplay(double value,
                                                          PressureUnit unit);
[[nodiscard]] double temperatureForDisplay(double temperatureC,
                                           TemperatureUnit unit);
[[nodiscard]] WarningPresentation evaluateWarningPresentation(
    WarningVisualMode mode,
    bool warningActive,
    bool elementsBlinkPhaseOn,
    bool fullScreenBlinkPhaseOn);

}  // namespace oilgauge
