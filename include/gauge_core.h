#pragma once

#include <cstddef>
#include <cstdint>

namespace oilgauge {

struct LinearCalibration {
  double slope = 0.0;
  double offset = 0.0;
  bool valid = false;
};

struct SteinhartHartCalibration {
  double a = 0.0;
  double b = 0.0;
  double c = 0.0;
  bool valid = false;
};

struct FrontendConfig {
  double pressureDividerTopOhm = 33000.0;
  double pressureDividerBottomOhm = 33000.0;
  double thermistorPullupOhm = 4990.0;
  double thermistorExcitationVolts = 3.3;
};

enum class Fault : std::uint8_t {
  none,
  adcMissing,
  calibrationMissing,
  inputOutOfRange,
  mathError,
};

struct ConvertedValue {
  double value = 0.0;
  Fault fault = Fault::calibrationMissing;

  [[nodiscard]] bool valid() const { return fault == Fault::none; }
};

[[nodiscard]] double restoreDividerInput(double adcVolts,
                                         double topOhm,
                                         double bottomOhm);

[[nodiscard]] double thermistorResistance(double nodeVolts,
                                          double excitationVolts,
                                          double pullupOhm);

[[nodiscard]] ConvertedValue convertPressure(
    double pressureAdcVolts,
    const FrontendConfig& frontend,
    const LinearCalibration& calibration);

[[nodiscard]] ConvertedValue convertTemperature(
    double thermistorNodeVolts,
    const FrontendConfig& frontend,
    const SteinhartHartCalibration& calibration);

[[nodiscard]] double clamp(double value, double minimum, double maximum);

[[nodiscard]] double lowPass(double previous,
                             double current,
                             double alpha);

struct AlarmThresholds {
  double lowPressurePsi = 15.0;
  double highTemperatureC = 125.0;
};

struct EngineState {
  bool known = false;
  std::uint32_t rpm = 0;

  [[nodiscard]] bool running() const { return known && rpm > 0; }
};

struct AlarmState {
  bool lowPressure = false;
  bool highTemperature = false;
  bool sensorFault = false;
};

[[nodiscard]] AlarmState evaluateAlarms(
    const ConvertedValue& pressure,
    const ConvertedValue& temperature,
    const EngineState& engine,
    const AlarmThresholds& thresholds);

enum class PressureState : std::uint8_t {
  fault,
  engineUnknown,
  engineStopped,
  warning,
  low,
  ok,
  high,
};

enum class TemperatureState : std::uint8_t {
  fault,
  belowRange,
  cold,
  warming,
  optimal,
  hot,
  veryHot,
};

struct RgbColor {
  std::uint8_t red = 0;
  std::uint8_t green = 0;
  std::uint8_t blue = 0;
};

struct DisplayState {
  PressureState pressure = PressureState::fault;
  TemperatureState temperature = TemperatureState::fault;
  RgbColor pressureColor{};
  RgbColor temperatureColor{};
  bool pressureAttentionVisible = false;
  bool showTemperatureBelowRange = false;
  double pressureBarFraction = 0.0;
  double temperatureBarFraction = 0.0;
};

[[nodiscard]] PressureState evaluatePressureState(
    const ConvertedValue& pressure,
    const EngineState& engine);

[[nodiscard]] TemperatureState evaluateTemperatureState(
    const ConvertedValue& temperature);

[[nodiscard]] RgbColor temperatureColor(double temperatureC);

[[nodiscard]] DisplayState evaluateDisplayState(
    const ConvertedValue& pressure,
    const ConvertedValue& temperature,
    const EngineState& engine,
    bool blinkPhaseOn,
    bool reducedMotion);

const char* faultName(Fault fault);
const char* pressureStateName(PressureState state);
const char* temperatureStateName(TemperatureState state);

}  // namespace oilgauge
