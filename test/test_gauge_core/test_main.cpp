#include <unity.h>

#include <cmath>
#include <initializer_list>

#include "gauge_core.h"
#include "gauge_settings.h"
#include "demo_sequence.h"
#include "warning_tone_gate.h"

using namespace oilgauge;

namespace {

void test_restore_divider() {
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 5.0,
                            restoreDividerInput(2.5, 33000.0, 33000.0));
}

void test_thermistor_resistance_midscale() {
  TEST_ASSERT_DOUBLE_WITHIN(
      1e-9, 4990.0, thermistorResistance(1.65, 3.3, 4990.0));
}

void test_thermistor_rejects_open_and_short() {
  TEST_ASSERT_TRUE(std::isnan(thermistorResistance(0.0, 3.3, 4990.0)));
  TEST_ASSERT_TRUE(std::isnan(thermistorResistance(3.3, 3.3, 4990.0)));
}

void test_pressure_refuses_missing_calibration() {
  const ConvertedValue result =
      convertPressure(1.0, FrontendConfig{}, LinearCalibration{});
  TEST_ASSERT_FALSE(result.valid());
  TEST_ASSERT_EQUAL_INT(static_cast<int>(Fault::calibrationMissing),
                        static_cast<int>(result.fault));
}

void test_pressure_linear_conversion() {
  const LinearCalibration calibration{30.0, -15.0, true};
  const ConvertedValue result =
      convertPressure(1.25, FrontendConfig{}, calibration);
  TEST_ASSERT_TRUE(result.valid());
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 60.0, result.value);
}

void test_temperature_steinhart_hart() {
  const SteinhartHartCalibration calibration{
      1.129148e-3, 2.34125e-4, 8.76741e-8, true};
  FrontendConfig frontend;
  frontend.thermistorPullupOhm = 10000.0;
  const ConvertedValue result =
      convertTemperature(1.65, frontend, calibration);
  TEST_ASSERT_TRUE(result.valid());
  TEST_ASSERT_DOUBLE_WITHIN(0.1, 25.0, result.value);
}

void test_low_pass_clamps_alpha() {
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 10.0, lowPass(10.0, 20.0, -1.0));
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 20.0, lowPass(10.0, 20.0, 2.0));
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 12.5, lowPass(10.0, 20.0, 0.25));
}

void test_alarm_fails_safe() {
  const ConvertedValue pressure{10.0, Fault::none};
  const ConvertedValue temperature{130.0, Fault::none};
  const AlarmState alarms =
      evaluateAlarms(
          pressure, temperature, EngineState{true, 2500}, AlarmThresholds{});
  TEST_ASSERT_TRUE(alarms.lowPressure);
  TEST_ASSERT_TRUE(alarms.highTemperature);
  TEST_ASSERT_FALSE(alarms.sensorFault);

  const AlarmState stopped =
      evaluateAlarms(
          pressure, temperature, EngineState{true, 0}, AlarmThresholds{});
  TEST_ASSERT_FALSE(stopped.lowPressure);

  const AlarmState unknown =
      evaluateAlarms(
          pressure, temperature, EngineState{false, 0}, AlarmThresholds{});
  TEST_ASSERT_FALSE(unknown.lowPressure);

  const AlarmState faulted = evaluateAlarms(
      {0.0, Fault::adcMissing},
      temperature,
      EngineState{true, 2500},
      AlarmThresholds{});
  TEST_ASSERT_TRUE(faulted.sensorFault);
}

void test_pressure_state_boundaries_and_engine_gate() {
  const EngineState running{true, 2500};
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(PressureState::fault),
      static_cast<int>(evaluatePressureState({-0.1, Fault::none}, running)));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(PressureState::engineUnknown),
      static_cast<int>(evaluatePressureState(
          {10.0, Fault::none}, EngineState{false, 0})));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(PressureState::engineStopped),
      static_cast<int>(evaluatePressureState(
          {0.0, Fault::none}, EngineState{true, 0})));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(PressureState::warning),
      static_cast<int>(evaluatePressureState({0.0, Fault::none}, running)));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(PressureState::warning),
      static_cast<int>(evaluatePressureState({10.0, Fault::none}, running)));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(PressureState::low),
      static_cast<int>(evaluatePressureState({10.1, Fault::none}, running)));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(PressureState::low),
      static_cast<int>(evaluatePressureState({14.9, Fault::none}, running)));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(PressureState::ok),
      static_cast<int>(evaluatePressureState({15.0, Fault::none}, running)));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(PressureState::ok),
      static_cast<int>(evaluatePressureState({80.0, Fault::none}, running)));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(PressureState::high),
      static_cast<int>(evaluatePressureState({80.1, Fault::none}, running)));
}

void test_temperature_state_boundaries() {
  const auto state = [](double celsius) {
    return static_cast<int>(
        evaluateTemperatureState({celsius, Fault::none}));
  };
  TEST_ASSERT_EQUAL_INT(static_cast<int>(TemperatureState::belowRange), state(49.9));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(TemperatureState::cold), state(50.0));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(TemperatureState::cold), state(69.9));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(TemperatureState::warming), state(70.0));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(TemperatureState::warming), state(74.9));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(TemperatureState::optimal), state(75.0));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(TemperatureState::optimal), state(93.9));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(TemperatureState::hot), state(94.0));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(TemperatureState::hot), state(100.0));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(TemperatureState::veryHot), state(100.1));
}

void test_temperature_color_stops_and_interpolation() {
  struct Case {
    double temperature;
    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;
  };
  constexpr Case cases[] = {
      {50.0, 30, 132, 255},
      {57.0, 30, 132, 255},
      {75.0, 174, 205, 167},
      {89.0, 174, 205, 167},
      {94.0, 234, 190, 82},
      {100.0, 255, 118, 28},
      {138.0, 255, 45, 56},
      {147.0, 255, 45, 56},
  };
  for (const Case& expected : cases) {
    const RgbColor actual = temperatureColor(expected.temperature);
    TEST_ASSERT_EQUAL_UINT8(expected.red, actual.red);
    TEST_ASSERT_EQUAL_UINT8(expected.green, actual.green);
    TEST_ASSERT_EQUAL_UINT8(expected.blue, actual.blue);
  }

  const RgbColor midpoint = temperatureColor(66.0);
  TEST_ASSERT_EQUAL_UINT8(102, midpoint.red);
  TEST_ASSERT_EQUAL_UINT8(169, midpoint.green);
  TEST_ASSERT_EQUAL_UINT8(211, midpoint.blue);
}

void test_display_state_warning_motion_and_bars() {
  const ConvertedValue pressure{10.0, Fault::none};
  const ConvertedValue temperature{49.0, Fault::none};
  const EngineState running{true, 2500};

  const DisplayState blinkOff =
      evaluateDisplayState(pressure, temperature, running, false, false);
  TEST_ASSERT_FALSE(blinkOff.pressureAttentionVisible);
  TEST_ASSERT_TRUE(blinkOff.showTemperatureBelowRange);
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 10.0 / 150.0,
                            blinkOff.pressureBarFraction);
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 0.0, blinkOff.temperatureBarFraction);

  const DisplayState blinkOn =
      evaluateDisplayState(pressure, temperature, running, true, false);
  TEST_ASSERT_TRUE(blinkOn.pressureAttentionVisible);

  const DisplayState reduced =
      evaluateDisplayState(pressure, temperature, running, false, true);
  TEST_ASSERT_TRUE(reduced.pressureAttentionVisible);
  TEST_ASSERT_EQUAL_UINT8(255, reduced.pressureColor.red);
  TEST_ASSERT_EQUAL_UINT8(57, reduced.pressureColor.green);
  TEST_ASSERT_EQUAL_UINT8(72, reduced.pressureColor.blue);
}

void test_demo_sequence_interpolates_smoothly() {
  const DemoFrame start = demoFrameAt(0);
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 0.0, start.pressurePsi);
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 49.0, start.temperatureC);
  TEST_ASSERT_EQUAL_UINT32(0, start.rpm);

  const DemoFrame midpoint = demoFrameAt(2'000'000);
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 3.5, midpoint.pressurePsi);
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 53.5, midpoint.temperatureC);
  TEST_ASSERT_EQUAL_UINT32(900, midpoint.rpm);

  const DemoFrame nextFrame = demoFrameAt(2'016'000);
  TEST_ASSERT_TRUE(nextFrame.pressurePsi > midpoint.pressurePsi);
  TEST_ASSERT_TRUE(nextFrame.temperatureC > midpoint.temperatureC);
}

void test_demo_sequence_hits_scenes_and_wraps() {
  const DemoFrame second = demoFrameAt(4'000'000);
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 7.0, second.pressurePsi);
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 58.0, second.temperatureC);
  TEST_ASSERT_EQUAL_UINT32(1800, second.rpm);

  const DemoFrame wrapped = demoFrameAt(28'000'000);
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 0.0, wrapped.pressurePsi);
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 49.0, wrapped.temperatureC);
  TEST_ASSERT_EQUAL_UINT32(0, wrapped.rpm);
}

void test_ac06_warning_blink_is_binary_two_hertz() {
  TEST_ASSERT_TRUE(warningBlinkPhaseOn(0));
  TEST_ASSERT_TRUE(warningBlinkPhaseOn(249'999));
  TEST_ASSERT_FALSE(warningBlinkPhaseOn(250'000));
  TEST_ASSERT_FALSE(warningBlinkPhaseOn(499'999));
  TEST_ASSERT_TRUE(warningBlinkPhaseOn(500'000));
  TEST_ASSERT_FALSE(warningBlinkPhaseOn(750'000));
  TEST_ASSERT_TRUE(warningBlinkPhaseOn(1'000'000));
}

void test_warning_tone_gate_triggers_once_and_rearms() {
  WarningToneGate gate;

  TEST_ASSERT_FALSE(gate.update(false));
  TEST_ASSERT_TRUE(gate.update(true));
  TEST_ASSERT_FALSE(gate.update(true));
  TEST_ASSERT_FALSE(gate.update(false));
  TEST_ASSERT_TRUE(gate.update(true));
}

void test_settings_are_sanitized_to_safe_ranges() {
  GaugeSettings settings;
  settings.brightnessPercent = 0;
  settings.warningVolumePercent = 255;
  settings.pressureUnit = static_cast<PressureUnit>(99);
  settings.warningVisualMode = static_cast<WarningVisualMode>(99);

  const GaugeSettings sanitized = sanitizeGaugeSettings(settings);
  TEST_ASSERT_EQUAL_UINT8(5, sanitized.brightnessPercent);
  TEST_ASSERT_EQUAL_UINT8(100, sanitized.warningVolumePercent);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(PressureUnit::psi),
                        static_cast<int>(sanitized.pressureUnit));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(WarningVisualMode::elementsBlink),
                        static_cast<int>(sanitized.warningVisualMode));
}

void test_pressure_units_convert_only_the_display_value() {
  TEST_ASSERT_DOUBLE_WITHIN(
      1e-9, 61.0, pressureForDisplay(61.0, PressureUnit::psi));
  TEST_ASSERT_DOUBLE_WITHIN(
      1e-6, 4.205802, pressureForDisplay(61.0, PressureUnit::bar));
}

void test_full_screen_warning_never_hides_pressure_number() {
  for (const bool phaseOn : {false, true}) {
    const WarningPresentation presentation = evaluateWarningPresentation(
        WarningVisualMode::fullScreenBlink, true, phaseOn);
    TEST_ASSERT_TRUE(presentation.pressureValueVisible);
    TEST_ASSERT_EQUAL(phaseOn, presentation.fullScreenRedVisible);
  }

  const WarningPresentation elementsOff = evaluateWarningPresentation(
      WarningVisualMode::elementsBlink, true, false);
  TEST_ASSERT_FALSE(elementsOff.attentionVisible);
  TEST_ASSERT_TRUE(elementsOff.pressureValueVisible);

  const WarningPresentation fixed = evaluateWarningPresentation(
      WarningVisualMode::fixed, true, false);
  TEST_ASSERT_TRUE(fixed.attentionVisible);
  TEST_ASSERT_FALSE(fixed.fullScreenRedVisible);
  TEST_ASSERT_TRUE(fixed.pressureValueVisible);
}

}  // namespace

void setUp() {}
void tearDown() {}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_restore_divider);
  RUN_TEST(test_thermistor_resistance_midscale);
  RUN_TEST(test_thermistor_rejects_open_and_short);
  RUN_TEST(test_pressure_refuses_missing_calibration);
  RUN_TEST(test_pressure_linear_conversion);
  RUN_TEST(test_temperature_steinhart_hart);
  RUN_TEST(test_low_pass_clamps_alpha);
  RUN_TEST(test_alarm_fails_safe);
  RUN_TEST(test_pressure_state_boundaries_and_engine_gate);
  RUN_TEST(test_temperature_state_boundaries);
  RUN_TEST(test_temperature_color_stops_and_interpolation);
  RUN_TEST(test_display_state_warning_motion_and_bars);
  RUN_TEST(test_demo_sequence_interpolates_smoothly);
  RUN_TEST(test_demo_sequence_hits_scenes_and_wraps);
  RUN_TEST(test_ac06_warning_blink_is_binary_two_hertz);
  RUN_TEST(test_warning_tone_gate_triggers_once_and_rearms);
  RUN_TEST(test_settings_are_sanitized_to_safe_ranges);
  RUN_TEST(test_pressure_units_convert_only_the_display_value);
  RUN_TEST(test_full_screen_warning_never_hides_pressure_number);
  return UNITY_END();
}
