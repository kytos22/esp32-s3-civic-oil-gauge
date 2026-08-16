#include "gauge_core.h"

#include <cmath>
#include <iterator>
#include <limits>

namespace oilgauge {

namespace {

bool finitePositive(double value) {
  return std::isfinite(value) && value > 0.0;
}

RgbColor interpolateColor(const RgbColor& start,
                          const RgbColor& end,
                          double fraction) {
  const double safeFraction = clamp(fraction, 0.0, 1.0);
  const auto channel = [safeFraction](std::uint8_t from, std::uint8_t to) {
    return static_cast<std::uint8_t>(std::lround(
        static_cast<double>(from) +
        safeFraction * (static_cast<double>(to) - static_cast<double>(from))));
  };
  return {
      channel(start.red, end.red),
      channel(start.green, end.green),
      channel(start.blue, end.blue),
  };
}

}  // namespace

double restoreDividerInput(double adcVolts,
                           double topOhm,
                           double bottomOhm) {
  if (!std::isfinite(adcVolts) || !finitePositive(topOhm) ||
      !finitePositive(bottomOhm)) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return adcVolts * (topOhm + bottomOhm) / bottomOhm;
}

double thermistorResistance(double nodeVolts,
                            double excitationVolts,
                            double pullupOhm) {
  if (!std::isfinite(nodeVolts) || !finitePositive(excitationVolts) ||
      !finitePositive(pullupOhm) || nodeVolts <= 0.0 ||
      nodeVolts >= excitationVolts) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return pullupOhm * nodeVolts / (excitationVolts - nodeVolts);
}

ConvertedValue convertPressure(double pressureAdcVolts,
  const FrontendConfig& frontend,
  const LinearCalibration& calibration) {
  if (!calibration.valid) {
    return {0.0, Fault::calibrationMissing};
  }

  if (!std::isfinite(pressureAdcVolts) || pressureAdcVolts < 0.0 ||
      pressureAdcVolts > 3.3) {
    return {0.0, Fault::inputOutOfRange};
  }

  const double sensorVolts =
      restoreDividerInput(pressureAdcVolts,
                          frontend.pressureDividerTopOhm,
                          frontend.pressureDividerBottomOhm);
  const double psi = calibration.slope * sensorVolts + calibration.offset;
  if (!std::isfinite(psi)) {
    return {0.0, Fault::mathError};
  }

  return {psi, Fault::none};
}

ConvertedValue convertTemperature(
    double thermistorNodeVolts,
    const FrontendConfig& frontend,
    const SteinhartHartCalibration& calibration) {
  if (!calibration.valid) {
    return {0.0, Fault::calibrationMissing};
  }

  const double resistance =
      thermistorResistance(thermistorNodeVolts,
                           frontend.thermistorExcitationVolts,
                           frontend.thermistorPullupOhm);
  if (!finitePositive(resistance)) {
    return {0.0, Fault::inputOutOfRange};
  }

  const double logR = std::log(resistance);
  const double inverseKelvin =
      calibration.a + calibration.b * logR +
      calibration.c * logR * logR * logR;
  if (!finitePositive(inverseKelvin)) {
    return {0.0, Fault::mathError};
  }

  const double celsius = 1.0 / inverseKelvin - 273.15;
  if (!std::isfinite(celsius)) {
    return {0.0, Fault::mathError};
  }

  return {celsius, Fault::none};
}

double clamp(double value, double minimum, double maximum) {
  if (value < minimum) {
    return minimum;
  }
  if (value > maximum) {
    return maximum;
  }
  return value;
}

double lowPass(double previous, double current, double alpha) {
  const double safeAlpha = clamp(alpha, 0.0, 1.0);
  return previous + safeAlpha * (current - previous);
}

AlarmState evaluateAlarms(const ConvertedValue& pressure,
                          const ConvertedValue& temperature,
                          const EngineState& engine,
                          const AlarmThresholds& thresholds) {
  AlarmState result;
  result.sensorFault = !pressure.valid() || !temperature.valid();
  result.lowPressure =
      pressure.valid() && engine.running() && pressure.value >= 0.0 &&
      pressure.value < thresholds.lowPressurePsi;
  result.highTemperature =
      temperature.valid() && temperature.value > thresholds.highTemperatureC;
  return result;
}

PressureState evaluatePressureState(const ConvertedValue& pressure,
                                    const EngineState& engine) {
  if (!pressure.valid() || !std::isfinite(pressure.value) ||
      pressure.value < 0.0) {
    return PressureState::fault;
  }
  if (!engine.known) {
    return PressureState::engineUnknown;
  }
  if (!engine.running()) {
    return PressureState::engineStopped;
  }
  if (pressure.value <= 10.0) {
    return PressureState::warning;
  }
  if (pressure.value < 15.0) {
    return PressureState::low;
  }
  if (pressure.value <= 80.0) {
    return PressureState::ok;
  }
  return PressureState::high;
}

TemperatureState evaluateTemperatureState(
    const ConvertedValue& temperature) {
  if (!temperature.valid() || !std::isfinite(temperature.value)) {
    return TemperatureState::fault;
  }
  if (temperature.value < 50.0) {
    return TemperatureState::belowRange;
  }
  if (temperature.value < 60.0) {
    return TemperatureState::cold;
  }
  if (temperature.value < 76.0) {
    return TemperatureState::warming;
  }
  if (temperature.value < 96.0) {
    return TemperatureState::optimal;
  }
  if (temperature.value <= 100.0) {
    return TemperatureState::hot;
  }
  return TemperatureState::veryHot;
}

RgbColor temperatureColor(double temperatureC) {
  struct Stop {
    double temperature;
    RgbColor color;
  };
  constexpr Stop kStops[] = {
      {50.0, {30, 132, 255}},
      {59.0, {30, 132, 255}},
      {76.0, {174, 205, 167}},
      {90.0, {174, 205, 167}},
      {96.0, {234, 190, 82}},
      {100.0, {255, 118, 28}},
      {138.0, {255, 45, 56}},
  };

  if (!std::isfinite(temperatureC) ||
      temperatureC <= kStops[0].temperature) {
    return kStops[0].color;
  }

  for (std::size_t index = 1; index < std::size(kStops); ++index) {
    if (temperatureC <= kStops[index].temperature) {
      const Stop& start = kStops[index - 1];
      const Stop& end = kStops[index];
      const double fraction =
          (temperatureC - start.temperature) /
          (end.temperature - start.temperature);
      return interpolateColor(start.color, end.color, fraction);
    }
  }
  return kStops[std::size(kStops) - 1].color;
}

DisplayState evaluateDisplayState(const ConvertedValue& pressure,
                                  const ConvertedValue& temperature,
                                  const EngineState& engine,
                                  bool blinkPhaseOn,
                                  bool reducedMotion) {
  constexpr RgbColor kPressureNormal{255, 176, 32};
  constexpr RgbColor kPressureWarning{255, 57, 72};

  DisplayState result;
  result.pressure = evaluatePressureState(pressure, engine);
  result.temperature = evaluateTemperatureState(temperature);
  const bool warning = result.pressure == PressureState::warning;
  result.pressureColor = warning ? kPressureWarning : kPressureNormal;
  result.temperatureColor = temperatureColor(temperature.value);
  result.pressureAttentionVisible =
      warning && (reducedMotion || blinkPhaseOn);
  result.showTemperatureBelowRange =
      result.temperature == TemperatureState::belowRange;
  result.pressureBarFraction =
      pressure.valid() ? clamp(pressure.value / 150.0, 0.0, 1.0) : 0.0;
  result.temperatureBarFraction =
      temperature.valid()
          ? clamp((temperature.value - 50.0) / 88.0, 0.0, 1.0)
          : 0.0;
  return result;
}

const char* faultName(Fault fault) {
  switch (fault) {
    case Fault::none:
      return "none";
    case Fault::adcMissing:
      return "adc_missing";
    case Fault::calibrationMissing:
      return "calibration_missing";
    case Fault::inputOutOfRange:
      return "input_out_of_range";
    case Fault::mathError:
      return "math_error";
  }
  return "unknown";
}

const char* pressureStateName(PressureState state) {
  switch (state) {
    case PressureState::fault:
      return "fault";
    case PressureState::engineUnknown:
      return "engine_unknown";
    case PressureState::engineStopped:
      return "engine_stopped";
    case PressureState::warning:
      return "warning";
    case PressureState::low:
      return "low";
    case PressureState::ok:
      return "ok";
    case PressureState::high:
      return "high";
  }
  return "unknown";
}

const char* temperatureStateName(TemperatureState state) {
  switch (state) {
    case TemperatureState::fault:
      return "fault";
    case TemperatureState::belowRange:
      return "below_range";
    case TemperatureState::cold:
      return "cold";
    case TemperatureState::warming:
      return "warming";
    case TemperatureState::optimal:
      return "optimal";
    case TemperatureState::hot:
      return "hot";
    case TemperatureState::veryHot:
      return "very_hot";
  }
  return "unknown";
}

}  // namespace oilgauge
