#include <unity.h>

#include <cmath>
#include <initializer_list>

#include "gauge_core.h"
#include "ads1115_protocol.h"
#include "block_refresh_tracker.h"
#include "calibration_config.h"
#include "gauge_settings.h"
#include "demo_sequence.h"
#include "display_clock_profile.h"
#include "display_profile.h"
#include "frame_slot_policy.h"
#include "frame_damage_policy.h"
#include "warning_tone_gate.h"

using namespace oilgauge;

namespace {

void test_ads1115_single_shot_config_and_voltage_scale() {
  TEST_ASSERT_EQUAL_HEX16(0xC383, ads1115SingleShotConfig(0));
  TEST_ASSERT_EQUAL_HEX16(0xD383, ads1115SingleShotConfig(1));
  TEST_ASSERT_EQUAL_HEX16(0xE383, ads1115SingleShotConfig(2));
  TEST_ASSERT_EQUAL_HEX16(0xF383, ads1115SingleShotConfig(3));
  TEST_ASSERT_EQUAL_HEX16(0xC383, ads1115SingleShotConfig(7));
  TEST_ASSERT_DOUBLE_WITHIN(1e-12, 0.0, ads1115RawToVolts(0));
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 4.095875, ads1115RawToVolts(32767));
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, -4.096, ads1115RawToVolts(-32768));
}

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

void test_provisional_innovate_temperature_curve_matches_bench_points() {
  struct Point {
    double resistanceOhm;
    double expectedC;
  };
  constexpr Point points[] = {
      {5458.0, 10.0},
      {2552.0, 27.0},
      {1036.0, 50.0},
      {470.0, 73.3},
      {220.0, 99.1},
      {89.0, 135.3},
      {86.0, 136.8},
      {84.0, 137.8},
      {80.0, 140.0},
  };
  for (const Point& point : points) {
    const double nodeVolts =
        calibration::kFrontend.thermistorExcitationVolts *
        point.resistanceOhm /
        (calibration::kFrontend.thermistorPullupOhm +
         point.resistanceOhm);
    const ConvertedValue converted = convertTemperature(
        nodeVolts, calibration::kFrontend, calibration::kTemperature);
    TEST_ASSERT_TRUE(converted.valid());
    TEST_ASSERT_DOUBLE_WITHIN(0.2, point.expectedC, converted.value);
  }
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
  TEST_ASSERT_EQUAL_INT(static_cast<int>(TemperatureState::cold), state(59.9));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(TemperatureState::warming), state(60.0));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(TemperatureState::warming), state(75.9));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(TemperatureState::optimal), state(76.0));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(TemperatureState::optimal), state(100.0));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(TemperatureState::veryHot), state(100.1));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(TemperatureState::veryHot), state(119.9));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(TemperatureState::warning), state(120.0));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(TemperatureState::warning), state(140.0));
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
      {59.0, 30, 132, 255},
      {76.0, 174, 205, 167},
      {90.0, 234, 190, 82},
      {100.0, 255, 118, 28},
      {120.0, 255, 45, 56},
      {140.0, 255, 45, 56},
      {147.0, 255, 45, 56},
  };
  for (const Case& expected : cases) {
    const RgbColor actual = temperatureColor(expected.temperature);
    TEST_ASSERT_EQUAL_UINT8(expected.red, actual.red);
    TEST_ASSERT_EQUAL_UINT8(expected.green, actual.green);
    TEST_ASSERT_EQUAL_UINT8(expected.blue, actual.blue);
  }

  const RgbColor midpoint = temperatureColor(83.0);
  TEST_ASSERT_EQUAL_UINT8(204, midpoint.red);
  TEST_ASSERT_EQUAL_UINT8(198, midpoint.green);
  TEST_ASSERT_EQUAL_UINT8(125, midpoint.blue);

  const RgbColor earlyWarning = temperatureColor(110.0, 110.0);
  TEST_ASSERT_EQUAL_UINT8(255, earlyWarning.red);
  TEST_ASSERT_EQUAL_UINT8(45, earlyWarning.green);
  TEST_ASSERT_EQUAL_UINT8(56, earlyWarning.blue);
  const RgbColor earlyTransition = temperatureColor(105.0, 110.0);
  TEST_ASSERT_EQUAL_UINT8(255, earlyTransition.red);
  TEST_ASSERT_EQUAL_UINT8(82, earlyTransition.green);
  TEST_ASSERT_EQUAL_UINT8(42, earlyTransition.blue);
}

void test_icon_palettes_and_fault_color_are_independent() {
  const EngineState running{true, 2500};
  const DisplayState normal = evaluateDisplayState(
      {45.0, Fault::none}, {80.0, Fault::none}, running, true, false);
  TEST_ASSERT_EQUAL_UINT8(255, normal.pressureIconColor.red);
  TEST_ASSERT_EQUAL_UINT8(255, normal.pressureIconColor.green);
  TEST_ASSERT_EQUAL_UINT8(255, normal.pressureIconColor.blue);
  TEST_ASSERT_EQUAL_UINT8(255, normal.temperatureIconColor.red);
  TEST_ASSERT_EQUAL_UINT8(255, normal.temperatureIconColor.green);
  TEST_ASSERT_EQUAL_UINT8(255, normal.temperatureIconColor.blue);

  const DisplayState cold = evaluateDisplayState(
      {45.0, Fault::none}, {55.0, Fault::none}, running, true, false);
  TEST_ASSERT_EQUAL_UINT8(30, cold.temperatureIconColor.red);
  TEST_ASSERT_EQUAL_UINT8(132, cold.temperatureIconColor.green);
  TEST_ASSERT_EQUAL_UINT8(255, cold.temperatureIconColor.blue);

  const DisplayState fault = evaluateDisplayState(
      {0.0, Fault::calibrationMissing},
      {0.0, Fault::adcMissing},
      running,
      true,
      false);
  TEST_ASSERT_EQUAL_UINT8(154, fault.pressureIconColor.red);
  TEST_ASSERT_EQUAL_UINT8(164, fault.temperatureIconColor.green);
  TEST_ASSERT_EQUAL_UINT8(175, fault.temperatureIconColor.blue);
}

void test_temperature_warning_blinks_only_dynamic_indicator() {
  const ConvertedValue pressure{45.0, Fault::none};
  const ConvertedValue temperature{120.0, Fault::none};
  const EngineState running{true, 2500};

  const DisplayState blinkOff =
      evaluateDisplayState(pressure, temperature, running, false, false);
  const DisplayState blinkOn =
      evaluateDisplayState(pressure, temperature, running, true, false);
  const DisplayState fixed =
      evaluateDisplayState(pressure, temperature, running, false, true);
  TEST_ASSERT_FALSE(blinkOff.temperatureAttentionVisible);
  TEST_ASSERT_TRUE(blinkOn.temperatureAttentionVisible);
  TEST_ASSERT_TRUE(fixed.temperatureAttentionVisible);
  TEST_ASSERT_EQUAL_DOUBLE(blinkOff.temperatureBarFraction,
                           blinkOn.temperatureBarFraction);
}

void test_display_state_warning_motion_and_bars() {
  const ConvertedValue pressure{10.0, Fault::none};
  const ConvertedValue temperature{49.0, Fault::none};
  const EngineState running{true, 2500};

  const DisplayState blinkOff =
      evaluateDisplayState(pressure, temperature, running, false, false);
  TEST_ASSERT_FALSE(blinkOff.pressureAttentionVisible);
  TEST_ASSERT_TRUE(blinkOff.showTemperatureBelowRange);
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 10.0 / 85.0,
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
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 50.0, start.temperatureC);
  TEST_ASSERT_EQUAL_UINT32(0, start.rpm);

  const DemoFrame midpoint = demoFrameAt(2'000'000);
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 3.5, midpoint.pressurePsi);
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 55.0, midpoint.temperatureC);
  TEST_ASSERT_EQUAL_UINT32(900, midpoint.rpm);

  const DemoFrame nextFrame = demoFrameAt(2'016'000);
  TEST_ASSERT_TRUE(nextFrame.pressurePsi > midpoint.pressurePsi);
  TEST_ASSERT_TRUE(nextFrame.temperatureC > midpoint.temperatureC);
}

void test_demo_sequence_hits_scenes_and_wraps() {
  const DemoFrame second = demoFrameAt(4'000'000);
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 7.0, second.pressurePsi);
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 60.0, second.temperatureC);
  TEST_ASSERT_EQUAL_UINT32(1800, second.rpm);

  const DemoFrame wrapped = demoFrameAt(28'000'000);
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 0.0, wrapped.pressurePsi);
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 50.0, wrapped.temperatureC);
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

void test_display_profile_feeds_the_sixty_hertz_panel() {
  TEST_ASSERT_EQUAL_UINT32(15U, kUiFramePeriodMs);
  TEST_ASSERT_EQUAL_UINT32(60U, kDisplayTargetFps);
}

void test_partial_renderer_never_selects_ready_or_inflight_canvas() {
  const FrameSlotMetadata slots[] = {
      {FrameSlotState::inFlight, 8},
      {FrameSlotState::free, 7},
      {FrameSlotState::free, 9},
  };
  TEST_ASSERT_EQUAL_INT(2, selectRenderSlot(slots));

  const FrameSlotMetadata protectedSlots[] = {
      {FrameSlotState::inFlight, 8},
      {FrameSlotState::ready, 9},
      {FrameSlotState::free, 8},
  };
  TEST_ASSERT_EQUAL_INT(-1, selectRenderSlot(protectedSlots));
}

void test_partial_presenter_selects_only_complete_ready_frame() {
  const FrameSlotMetadata slots[] = {
      {FrameSlotState::ready, 31},
      {FrameSlotState::ready, 32},
  };
  TEST_ASSERT_EQUAL_INT(1, selectNewestReadySlot(slots));

  const FrameSlotMetadata unavailable[] = {
      {FrameSlotState::rendering, 33},
      {FrameSlotState::inFlight, 32},
  };
  TEST_ASSERT_EQUAL_INT(-1, selectNewestReadySlot(unavailable));
}

void test_partial_damage_history_recovers_stale_canvas() {
  DamageHistoryEntry history[4]{};
  history[0].generation = 1;
  history[0].tiles.markArea({0, 0, 31, 31});
  history[1].generation = 2;
  history[1].tiles.markArea({64, 64, 95, 95});
  bool complete = false;
  const DamageTiles damage = damageSince(history, 2, 0, 2, complete);
  TEST_ASSERT_TRUE(complete);
  TEST_ASSERT_TRUE(damage.marked(0, 0));
  TEST_ASSERT_TRUE(damage.marked(2, 2));
  TEST_ASSERT_EQUAL_UINT32(2, damage.tileCount());

  const DamageTiles fallback = damageSince(history, 2, 0, 3, complete);
  TEST_ASSERT_FALSE(complete);
  TEST_ASSERT_EQUAL_UINT32(kDamageTileCount, fallback.tileCount());
}

void test_partial_refresh_closes_only_after_last_flush() {
  BlockRefreshTracker tracker;
  tracker.begin();
  TEST_ASSERT_EQUAL_INT(static_cast<int>(BlockRefreshOutcome::retry),
                        static_cast<int>(tracker.finish()));
  tracker.onFlush(false);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(BlockRefreshOutcome::retry),
                        static_cast<int>(tracker.finish()));
  tracker.onFlush(true);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(BlockRefreshOutcome::ready),
                        static_cast<int>(tracker.finish()));
}

void test_ac41_qspi_bounce_profile_stays_inside_reserved_internal_dma() {
  constexpr std::uint32_t bytesPerRow = 480U * 2U;
  constexpr std::uint32_t queuedBytes =
      bytesPerRow * OIL_GAUGE_DISPLAY_TRANSFER_ROWS *
      OIL_GAUGE_DISPLAY_QUEUE_DEPTH;

  TEST_ASSERT_EQUAL_UINT32(80'000'000U, OIL_GAUGE_DISPLAY_QSPI_HZ);
  TEST_ASSERT_EQUAL_UINT32(8U, OIL_GAUGE_DISPLAY_TRANSFER_ROWS);
  TEST_ASSERT_EQUAL_UINT32(3U, OIL_GAUGE_DISPLAY_QUEUE_DEPTH);
  TEST_ASSERT_LESS_OR_EQUAL_UINT32(24U * 1024U, queuedBytes);
}

void test_ac32_warning_tone_gate_starts_and_stops_loop() {
  WarningToneGate gate;

  TEST_ASSERT_EQUAL_INT(static_cast<int>(WarningToneCommand::none),
                        static_cast<int>(gate.update(false)));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(WarningToneCommand::startLoop),
                        static_cast<int>(gate.update(true)));
  TEST_ASSERT_TRUE(gate.warningActive());
  TEST_ASSERT_EQUAL_INT(static_cast<int>(WarningToneCommand::none),
                        static_cast<int>(gate.update(true)));
  TEST_ASSERT_TRUE(gate.warningActive());
  TEST_ASSERT_EQUAL_INT(static_cast<int>(WarningToneCommand::stopLoop),
                        static_cast<int>(gate.update(false)));
  TEST_ASSERT_FALSE(gate.warningActive());
  TEST_ASSERT_EQUAL_INT(static_cast<int>(WarningToneCommand::startLoop),
                        static_cast<int>(gate.update(true)));
}

void test_settings_are_sanitized_to_safe_ranges() {
  GaugeSettings settings;
  settings.brightnessPercent = 0;
  settings.warningVolumePercent = 255;
  settings.lowPressureWarningPsi = 0;
  settings.highTemperatureWarningCelsius = 255;
  settings.startupLogoSeconds = 255;
  settings.pressureUnit = static_cast<PressureUnit>(99);
  settings.temperatureUnit = static_cast<TemperatureUnit>(99);
  settings.warningVisualMode = static_cast<WarningVisualMode>(99);
  settings.dataSource = static_cast<DataSource>(99);

  const GaugeSettings sanitized = sanitizeGaugeSettings(settings);
  TEST_ASSERT_EQUAL_UINT8(5, sanitized.brightnessPercent);
  TEST_ASSERT_EQUAL_UINT8(100, sanitized.warningVolumePercent);
  TEST_ASSERT_EQUAL_UINT8(1, sanitized.lowPressureWarningPsi);
  TEST_ASSERT_EQUAL_UINT8(140, sanitized.highTemperatureWarningCelsius);
  TEST_ASSERT_EQUAL_UINT8(10, sanitized.startupLogoSeconds);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(PressureUnit::psi),
                        static_cast<int>(sanitized.pressureUnit));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(TemperatureUnit::celsius),
                        static_cast<int>(sanitized.temperatureUnit));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(WarningVisualMode::elementsBlink),
                        static_cast<int>(sanitized.warningVisualMode));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(DataSource::demo),
                        static_cast<int>(sanitized.dataSource));
}

void test_configurable_pressure_warning_threshold_controls_state() {
  const ConvertedValue pressure{12.0, Fault::none};
  const EngineState running{true, 1800};

  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(PressureState::warning),
      static_cast<int>(evaluatePressureState(pressure, running, 12.0)));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(PressureState::low),
      static_cast<int>(evaluatePressureState(pressure, running, 10.0)));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(PressureState::engineStopped),
      static_cast<int>(evaluatePressureState(pressure, {true, 0}, 30.0)));
}

void test_configurable_temperature_warning_threshold_controls_state() {
  const ConvertedValue temperature{119.0, Fault::none};
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(TemperatureState::veryHot),
      static_cast<int>(evaluateTemperatureState(temperature, 120.0)));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(TemperatureState::warning),
      static_cast<int>(evaluateTemperatureState(temperature, 115.0)));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(TemperatureState::warning),
      static_cast<int>(evaluateTemperatureState({110.0, Fault::none}, 1.0)));
}

void test_display_bars_reach_full_at_the_approved_limits() {
  const DisplayState state = evaluateDisplayState(
      {85.0, Fault::none},
      {140.0, Fault::none},
      EngineState{true, 2500},
      true,
      false);
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 1.0, state.pressureBarFraction);
  TEST_ASSERT_DOUBLE_WITHIN(1e-9, 1.0, state.temperatureBarFraction);
}

void test_warning_threshold_uses_selected_display_unit() {
  TEST_ASSERT_DOUBLE_WITHIN(
      1e-9, 10.0, warningThresholdForDisplay(10, PressureUnit::psi));
  TEST_ASSERT_DOUBLE_WITHIN(
      1e-6, 0.689475729, warningThresholdForDisplay(10, PressureUnit::bar));
  TEST_ASSERT_EQUAL_UINT8(
      10, warningThresholdPsiFromDisplay(0.7, PressureUnit::bar));
  TEST_ASSERT_EQUAL_UINT8(
      18, warningThresholdPsiFromDisplay(18.0, PressureUnit::psi));
}

void test_sensor_source_can_be_selected_without_enabling_fake_values() {
  GaugeSettings settings;
  settings.dataSource = DataSource::sensors;

  const GaugeSettings sanitized = sanitizeGaugeSettings(settings);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(DataSource::sensors),
                        static_cast<int>(sanitized.dataSource));
}

void test_full_screen_warning_uses_an_independent_half_hertz_cycle() {
  TEST_ASSERT_TRUE(fullScreenWarningPhaseOn(0));
  TEST_ASSERT_TRUE(fullScreenWarningPhaseOn(999'999));
  TEST_ASSERT_FALSE(fullScreenWarningPhaseOn(1'000'000));
  TEST_ASSERT_FALSE(fullScreenWarningPhaseOn(1'999'999));
  TEST_ASSERT_TRUE(fullScreenWarningPhaseOn(2'000'000));

  const WarningPresentation red = evaluateWarningPresentation(
      WarningVisualMode::fullScreenBlink, true, false, true);
  TEST_ASSERT_TRUE(red.attentionVisible);
  TEST_ASSERT_TRUE(red.fullScreenRedVisible);
  TEST_ASSERT_TRUE(red.pressureValueVisible);
}

void test_pressure_units_convert_only_the_display_value() {
  TEST_ASSERT_DOUBLE_WITHIN(
      1e-9, 61.0, pressureForDisplay(61.0, PressureUnit::psi));
  TEST_ASSERT_DOUBLE_WITHIN(
      1e-6, 4.205802, pressureForDisplay(61.0, PressureUnit::bar));
}

void test_temperature_units_convert_only_the_display_value() {
  TEST_ASSERT_DOUBLE_WITHIN(
      1e-9, 50.0, temperatureForDisplay(50.0, TemperatureUnit::celsius));
  TEST_ASSERT_DOUBLE_WITHIN(
      1e-9, 122.0, temperatureForDisplay(50.0, TemperatureUnit::fahrenheit));
  TEST_ASSERT_DOUBLE_WITHIN(
      1e-9, 167.0, temperatureForDisplay(75.0, TemperatureUnit::fahrenheit));
  TEST_ASSERT_DOUBLE_WITHIN(
      1e-9, 201.2, temperatureForDisplay(94.0, TemperatureUnit::fahrenheit));
  TEST_ASSERT_DOUBLE_WITHIN(
      1e-9, 212.0, temperatureForDisplay(100.0, TemperatureUnit::fahrenheit));
}

void test_full_screen_warning_never_hides_pressure_number() {
  for (const bool phaseOn : {false, true}) {
    const WarningPresentation presentation = evaluateWarningPresentation(
        WarningVisualMode::fullScreenBlink, true, true, phaseOn);
    TEST_ASSERT_TRUE(presentation.pressureValueVisible);
    TEST_ASSERT_EQUAL(phaseOn, presentation.fullScreenRedVisible);
  }

  const WarningPresentation elementsOff = evaluateWarningPresentation(
      WarningVisualMode::elementsBlink, true, false, true);
  TEST_ASSERT_FALSE(elementsOff.attentionVisible);
  TEST_ASSERT_TRUE(elementsOff.pressureValueVisible);

  const WarningPresentation fixed = evaluateWarningPresentation(
      WarningVisualMode::fixed, true, false, false);
  TEST_ASSERT_TRUE(fixed.attentionVisible);
  TEST_ASSERT_FALSE(fixed.fullScreenRedVisible);
  TEST_ASSERT_TRUE(fixed.pressureValueVisible);
}

}  // namespace

void setUp() {}
void tearDown() {}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_ads1115_single_shot_config_and_voltage_scale);
  RUN_TEST(test_restore_divider);
  RUN_TEST(test_thermistor_resistance_midscale);
  RUN_TEST(test_thermistor_rejects_open_and_short);
  RUN_TEST(test_pressure_refuses_missing_calibration);
  RUN_TEST(test_pressure_linear_conversion);
  RUN_TEST(test_temperature_steinhart_hart);
  RUN_TEST(test_provisional_innovate_temperature_curve_matches_bench_points);
  RUN_TEST(test_low_pass_clamps_alpha);
  RUN_TEST(test_alarm_fails_safe);
  RUN_TEST(test_pressure_state_boundaries_and_engine_gate);
  RUN_TEST(test_temperature_state_boundaries);
  RUN_TEST(test_temperature_color_stops_and_interpolation);
  RUN_TEST(test_icon_palettes_and_fault_color_are_independent);
  RUN_TEST(test_temperature_warning_blinks_only_dynamic_indicator);
  RUN_TEST(test_display_state_warning_motion_and_bars);
  RUN_TEST(test_demo_sequence_interpolates_smoothly);
  RUN_TEST(test_demo_sequence_hits_scenes_and_wraps);
  RUN_TEST(test_ac06_warning_blink_is_binary_two_hertz);
  RUN_TEST(test_display_profile_feeds_the_sixty_hertz_panel);
  RUN_TEST(test_partial_renderer_never_selects_ready_or_inflight_canvas);
  RUN_TEST(test_partial_presenter_selects_only_complete_ready_frame);
  RUN_TEST(test_partial_damage_history_recovers_stale_canvas);
  RUN_TEST(test_partial_refresh_closes_only_after_last_flush);
  RUN_TEST(test_ac41_qspi_bounce_profile_stays_inside_reserved_internal_dma);
  RUN_TEST(test_ac32_warning_tone_gate_starts_and_stops_loop);
  RUN_TEST(test_settings_are_sanitized_to_safe_ranges);
  RUN_TEST(test_configurable_pressure_warning_threshold_controls_state);
  RUN_TEST(test_configurable_temperature_warning_threshold_controls_state);
  RUN_TEST(test_display_bars_reach_full_at_the_approved_limits);
  RUN_TEST(test_warning_threshold_uses_selected_display_unit);
  RUN_TEST(test_sensor_source_can_be_selected_without_enabling_fake_values);
  RUN_TEST(test_full_screen_warning_uses_an_independent_half_hertz_cycle);
  RUN_TEST(test_pressure_units_convert_only_the_display_value);
  RUN_TEST(test_temperature_units_convert_only_the_display_value);
  RUN_TEST(test_full_screen_warning_never_hides_pressure_number);
  return UNITY_END();
}
