#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bsp/display.h"
#include "bsp/esp-bsp.h"
#include "esp_lv_adapter.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "ads1115_diagnostics.h"
#include "ads1115_protocol.h"
#include "automatic_brightness.h"
#include "calibration_config.h"
#include "civic_aux_uart.h"
#include "demo_sequence.h"
#include "display_runtime.h"
#include "gauge_core.h"
#include "oil_gauge_ui.h"
#include "settings_store.h"
#include "warning_audio.h"
#include "warning_tone_gate.h"

namespace {

using namespace oilgauge;

constexpr char kTag[] = "oil_gauge";
constexpr std::uint64_t kUiFramePeriodUs = kUiFramePeriodMs * 1'000U;
constexpr std::uint64_t kAds1115SampleStaleUs = 500'000U;
constexpr std::uint64_t kTemperatureLogPeriodUs = 1'000'000U;
constexpr double kThermistorMinimumBenchOhm = 40.0;
constexpr double kThermistorMaximumBenchOhm = 6000.0;
constexpr double kTemperatureMinimumBenchC = 10.0;
constexpr double kTemperatureMaximumBenchC = 140.0;
WarningToneGate gWarningToneGate;
GaugeSettings gSettings;
AutomaticBrightnessController gBrightnessController;
CivicAuxSnapshot gCivicAuxSnapshot;
AutomaticBrightnessStatus gAutomaticBrightnessStatus;
OilGaugeBrightnessStatus gBrightnessUiStatus;
int gLastRequestedBrightness = -1;
std::uint32_t gFilteredTemperatureSequence = 0;
bool gFilteredTemperatureValid = false;
double gFilteredTemperatureC = 0.0;
std::uint64_t gLastTemperatureLogUs = 0;

void applyWarningAudioState(bool warningActive) {
  switch (gWarningToneGate.update(warningActive)) {
    case WarningToneCommand::startLoop:
      setWarningAudioActive(true);
      break;
    case WarningToneCommand::stopLoop:
      setWarningAudioActive(false);
      break;
    case WarningToneCommand::none:
      break;
  }
}

GaugeSettings compileTimeDefaults() {
  GaugeSettings defaults;
  defaults.brightnessPercent = CONFIG_OIL_GAUGE_BRIGHTNESS_PERCENT;
  defaults.warningSoundEnabled = CONFIG_OIL_GAUGE_DEMO_WARNING_AUDIO;
  defaults.warningVolumePercent =
      CONFIG_OIL_GAUGE_WARNING_AUDIO_VOLUME_PERCENT;
  return sanitizeGaugeSettings(defaults);
}

void renderDemoFrame(std::uint64_t nowUs) {
  const DemoFrame frame = demoFrameAt(nowUs);
  const bool elementsBlinkPhaseOn = warningBlinkPhaseOn(nowUs);
  const bool fullScreenBlinkPhaseOn = fullScreenWarningPhaseOn(nowUs);
  const ConvertedValue pressure{frame.pressurePsi, Fault::none};
  const ConvertedValue temperature{frame.temperatureC, Fault::none};
  const EngineState engine{true, frame.rpm};

  updateOilGaugeUi(
      pressure,
      temperature,
      engine,
      elementsBlinkPhaseOn,
      fullScreenBlinkPhaseOn,
      gSettings);

  const bool warningActive =
      evaluatePressureState(
          pressure, engine, gSettings.lowPressureWarningPsi) ==
      PressureState::warning;
  applyWarningAudioState(warningActive);
}

ConvertedValue readBenchTemperature(std::uint64_t nowUs) {
  Ads1115Sample sample;
  if (!latestAds1115Sample(sample) || !sample.valid ||
      nowUs < sample.timestampUs ||
      nowUs - sample.timestampUs > kAds1115SampleStaleUs) {
    gFilteredTemperatureValid = false;
    return {0.0, Fault::adcMissing};
  }

  const double nodeVolts = ads1115RawToVolts(sample.raw[1]);
  const double resistance = thermistorResistance(
      nodeVolts,
      calibration::kFrontend.thermistorExcitationVolts,
      calibration::kFrontend.thermistorPullupOhm);
  if (!std::isfinite(resistance) ||
      resistance < kThermistorMinimumBenchOhm ||
      resistance > kThermistorMaximumBenchOhm) {
    gFilteredTemperatureValid = false;
    return {0.0, Fault::inputOutOfRange};
  }

  ConvertedValue temperature = convertTemperature(
      nodeVolts, calibration::kFrontend, calibration::kTemperature);
  if (!temperature.valid()) {
    gFilteredTemperatureValid = false;
    return temperature;
  }
  temperature.value = clamp(temperature.value,
                            kTemperatureMinimumBenchC,
                            kTemperatureMaximumBenchC);
  if (sample.sequence != gFilteredTemperatureSequence) {
    gFilteredTemperatureC =
        gFilteredTemperatureValid
            ? lowPass(gFilteredTemperatureC, temperature.value, 0.35)
            : temperature.value;
    gFilteredTemperatureSequence = sample.sequence;
    gFilteredTemperatureValid = true;
    if (gLastTemperatureLogUs == 0 ||
        nowUs - gLastTemperatureLogUs >= kTemperatureLogPeriodUs) {
      gLastTemperatureLogUs = nowUs;
      ESP_LOGI(kTag,
               "A1 bench NTC: raw=%d node=%.4fV R=%.1f ohm "
               "temperature=%.1fC (provisional)",
               static_cast<int>(sample.raw[1]),
               nodeVolts,
               resistance,
               gFilteredTemperatureC);
    }
  }
  return {gFilteredTemperatureC, Fault::none};
}

void renderSensorFrame(std::uint64_t nowUs) {
  applyWarningAudioState(false);
  updateOilGaugeUi(
      {0.0, Fault::calibrationMissing},
      readBenchTemperature(nowUs),
      EngineState{false, 0},
      warningBlinkPhaseOn(nowUs),
      fullScreenWarningPhaseOn(nowUs),
      gSettings);
}

void applyUiActions(const OilGaugeUiActions& actions,
                    bool settingsStoreAvailable) {
  if (actions.applySettings) {
    gSettings = sanitizeGaugeSettings(actions.settings);
    setWarningAudioEnabled(gSettings.warningSoundEnabled);
    if (warningAudioAvailable() &&
        !setWarningAudioVolume(gSettings.warningVolumePercent)) {
      ESP_LOGW(kTag, "Unable to apply warning volume");
    }
  }
  if (actions.testSound) {
    requestWarningTone();
  }
  if (actions.saveSettings && settingsStoreAvailable &&
      !saveGaugeSettings(actions.settings)) {
    ESP_LOGW(kTag, "Settings remain active but could not be persisted");
  }
}

void updateAutomaticBrightness(std::uint64_t nowUs) {
  const std::uint64_t nowMs = nowUs / 1000U;
  CivicAuxSnapshot latestSnapshot{};
  (void)latestCivicAuxSnapshot(latestSnapshot);
  gCivicAuxSnapshot = latestSnapshot;

  gBrightnessController.setPreferences(
      gSettings.brightnessMode,
      gSettings.brightnessPercent,
      gSettings.automaticBrightnessMinimumPercent,
      gSettings.automaticBrightnessMaximumPercent,
      nowMs,
      gSettings.automaticBrightnessBiasPercent);
  gAutomaticBrightnessStatus =
      gBrightnessController.update(gCivicAuxSnapshot, nowMs);
  if (gLastRequestedBrightness !=
      gAutomaticBrightnessStatus.appliedPercent) {
    gLastRequestedBrightness = gAutomaticBrightnessStatus.appliedPercent;
    requestOilDisplayBrightness(
        gAutomaticBrightnessStatus.appliedPercent);
  }

  gBrightnessUiStatus.receiverRunning =
      gCivicAuxSnapshot.receiverRunning;
  gBrightnessUiStatus.hasAmbientFrame =
      gCivicAuxSnapshot.hasAmbientFrame;
  gBrightnessUiStatus.latestAmbientUsable =
      gCivicAuxSnapshot.latestAmbientUsable;
  gBrightnessUiStatus.luxFresh = gAutomaticBrightnessStatus.luxFresh;
  gBrightnessUiStatus.sensorState = gCivicAuxSnapshot.sensorState;
  gBrightnessUiStatus.rangeProfile = gCivicAuxSnapshot.rangeProfile;
  gBrightnessUiStatus.filteredMillilux =
      gCivicAuxSnapshot.filteredMillilux;
  gBrightnessUiStatus.automaticState =
      gAutomaticBrightnessStatus.state;
  gBrightnessUiStatus.automaticPercent =
      gAutomaticBrightnessStatus.automaticPercent;
  gBrightnessUiStatus.appliedPercent =
      gAutomaticBrightnessStatus.appliedPercent;
  gBrightnessUiStatus.automaticPercentAvailable =
      gAutomaticBrightnessStatus.automaticPercentAvailable;
}

}  // namespace

extern "C" void app_main(void) {
  ESP_LOGI(kTag, "Civic oil gauge starting on ESP-IDF %s", IDF_VER);
  ESP_LOGI(kTag,
           "Demo mode: %s",
           CONFIG_OIL_GAUGE_DEMO_MODE ? "enabled" : "disabled");

  const GaugeSettings defaults = compileTimeDefaults();
  const bool settingsStoreAvailable = initSettingsStore();
  gSettings = settingsStoreAvailable ? loadGaugeSettings(defaults) : defaults;
  ESP_LOGI(kTag,
           "Settings loaded: brightness=%u%% brightness_mode=%u "
           "auto_range=%u-%u%% auto_bias=%d "
           "pressure_unit=%u temperature_unit=%u source=%u language=%u "
           "warning_mode=%u sound=%u volume=%u pressure_warning=%upsi "
           "temperature_warning=%uC boot_logo=%us",
           static_cast<unsigned>(gSettings.brightnessPercent),
           static_cast<unsigned>(gSettings.brightnessMode),
           static_cast<unsigned>(
               gSettings.automaticBrightnessMinimumPercent),
           static_cast<unsigned>(
               gSettings.automaticBrightnessMaximumPercent),
           static_cast<int>(gSettings.automaticBrightnessBiasPercent),
           static_cast<unsigned>(gSettings.pressureUnit),
           static_cast<unsigned>(gSettings.temperatureUnit),
           static_cast<unsigned>(gSettings.dataSource),
           static_cast<unsigned>(gSettings.language),
           static_cast<unsigned>(gSettings.warningVisualMode),
           gSettings.warningSoundEnabled ? 1U : 0U,
           static_cast<unsigned>(gSettings.warningVolumePercent),
           static_cast<unsigned>(gSettings.lowPressureWarningPsi),
           static_cast<unsigned>(gSettings.highTemperatureWarningCelsius),
           static_cast<unsigned>(gSettings.startupLogoSeconds));

  const OilDisplayRuntime displayRuntime = startOilDisplayRuntime();
  if (displayRuntime.display == nullptr || displayRuntime.input == nullptr) {
    ESP_LOGE(kTag, "Waveshare display initialization failed");
    return;
  }

  const std::uint64_t brightnessStartedMs =
      static_cast<std::uint64_t>(esp_timer_get_time()) / 1000U;
  gBrightnessController.reset(
      gSettings.brightnessMode,
      gSettings.brightnessPercent,
      gSettings.automaticBrightnessMinimumPercent,
      gSettings.automaticBrightnessMaximumPercent,
      brightnessStartedMs,
      gSettings.automaticBrightnessBiasPercent);
  gLastRequestedBrightness = gSettings.brightnessPercent;
  requestOilDisplayBrightness(gSettings.brightnessPercent);

  if (!startCivicAuxUartReceiver()) {
    ESP_LOGW(kTag,
             "CivicAux UART unavailable; brightness remains on manual backup");
  }

  if (CONFIG_OIL_GAUGE_DEMO_MODE &&
      CONFIG_OIL_GAUGE_DEMO_WARNING_AUDIO) {
    if (!initWarningAudio()) {
      ESP_LOGW(kTag, "Demo will continue without warning audio");
    } else {
      setWarningAudioEnabled(gSettings.warningSoundEnabled);
      if (!setWarningAudioVolume(gSettings.warningVolumePercent)) {
        ESP_LOGW(kTag, "Demo will use the initialized warning volume");
      }
    }
  }

#if CONFIG_OIL_GAUGE_ADS1115_DIAGNOSTICS
  if (!startAds1115Diagnostics(bsp_i2c_get_handle())) {
    ESP_LOGW(kTag, "Demo continues without ADS1115 diagnostics");
  }
#endif

  lv_indev_t* input = displayRuntime.input;
  if (input != nullptr) {
    lv_indev_set_long_press_time(input, 700);
  } else {
    ESP_LOGW(kTag, "Touch input unavailable; settings hold is disabled");
  }

  const esp_err_t initialLockResult = esp_lv_adapter_lock(-1);
  if (initialLockResult != ESP_OK) {
    ESP_LOGE(kTag,
             "Unable to acquire the initial LVGL display lock: %s",
             esp_err_to_name(initialLockResult));
    return;
  }
  createOilGaugeUi(lv_screen_active(), gSettings, defaults);
  esp_lv_adapter_unlock();
  startOilDisplayPresentation();

  const std::uint64_t uiStartedUs =
      static_cast<std::uint64_t>(esp_timer_get_time());
  const std::uint64_t bootSplashDeadlineUs =
      uiStartedUs + static_cast<std::uint64_t>(gSettings.startupLogoSeconds) *
                        1'000'000U;
  std::uint64_t lastFrameUs = uiStartedUs;
  while (true) {
    const std::uint64_t nowUs =
        static_cast<std::uint64_t>(esp_timer_get_time());
    updateAutomaticBrightness(nowUs);
    const bool uiUpdateDue = nowUs - lastFrameUs >= kUiFramePeriodUs;
    const bool blockFramePending = oilDisplayBlockFramePending();
    if (uiUpdateDue || blockFramePending) {
      // Keep only the newest state. Presentation is paced independently by
      // the CO5300 TE signal, so replaying missed application ticks adds lag.
      if (uiUpdateDue) {
        lastFrameUs = nowUs;
      }
      if (esp_lv_adapter_lock(-1) == ESP_OK) {
        updateOilGaugeBrightnessStatus(gBrightnessUiStatus);
        setOilGaugeBootSplashVisible(
            gSettings.startupLogoSeconds > 0 && nowUs < bootSplashDeadlineUs);
        OilGaugeUiActions beforeRender;
        const bool hadBeforeRender = takeOilGaugeUiActions(beforeRender);
        if (hadBeforeRender && beforeRender.applySettings) {
          gSettings = sanitizeGaugeSettings(beforeRender.settings);
        }
        if (CONFIG_OIL_GAUGE_DEMO_MODE &&
            gSettings.dataSource == DataSource::demo) {
          renderDemoFrame(nowUs);
        } else {
          renderSensorFrame(nowUs);
        }
        if (beginOilDisplayBlockFrame(
                lv_display_get_screen_active(displayRuntime.display))) {
          lv_refr_now(displayRuntime.display);
          finishOilDisplayBlockFrameAttempt();
        }
        OilGaugeUiActions afterRender;
        const bool hadAfterRender = takeOilGaugeUiActions(afterRender);
        esp_lv_adapter_unlock();
        if (hadBeforeRender) {
          applyUiActions(beforeRender, settingsStoreAvailable);
        }
        if (hadAfterRender) {
          applyUiActions(afterRender, settingsStoreAvailable);
        }
      }
    }
    const std::uint64_t waitStartedUs =
        static_cast<std::uint64_t>(esp_timer_get_time());
    const std::uint64_t nextUiUs = lastFrameUs + kUiFramePeriodUs;
    const std::uint64_t remainingUs =
        nextUiUs > waitStartedUs ? nextUiUs - waitStartedUs : 0;
    const std::uint32_t maximumWaitMs =
        remainingUs == 0
            ? 0
            : static_cast<std::uint32_t>((remainingUs + 999U) / 1'000U);
    waitForOilDisplayWork(maximumWaitMs);
  }
}
