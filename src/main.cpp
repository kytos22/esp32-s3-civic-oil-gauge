#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bsp/display.h"
#include "bsp/esp-bsp.h"
#include "esp_lv_adapter.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"

#include <cstdint>

#include "demo_sequence.h"
#include "ads1115_diagnostics.h"
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
WarningToneGate gWarningToneGate;
GaugeSettings gSettings;

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

void renderCalibrationGate(std::uint64_t nowUs) {
  applyWarningAudioState(false);
  updateOilGaugeUi(
      {0.0, Fault::calibrationMissing},
      {0.0, Fault::calibrationMissing},
      EngineState{false, 0},
      warningBlinkPhaseOn(nowUs),
      fullScreenWarningPhaseOn(nowUs),
      gSettings);
}

void applyUiActions(const OilGaugeUiActions& actions,
                    bool settingsStoreAvailable) {
  if (actions.applySettings) {
    gSettings = sanitizeGaugeSettings(actions.settings);
    requestOilDisplayBrightness(gSettings.brightnessPercent);
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
           "Settings loaded: pressure_unit=%u temperature_unit=%u source=%u "
           "warning_mode=%u sound=%u volume=%u pressure_warning=%upsi "
           "boot_logo=%us",
           static_cast<unsigned>(gSettings.pressureUnit),
           static_cast<unsigned>(gSettings.temperatureUnit),
           static_cast<unsigned>(gSettings.dataSource),
           static_cast<unsigned>(gSettings.warningVisualMode),
           gSettings.warningSoundEnabled ? 1U : 0U,
           static_cast<unsigned>(gSettings.warningVolumePercent),
           static_cast<unsigned>(gSettings.lowPressureWarningPsi),
           static_cast<unsigned>(gSettings.startupLogoSeconds));

  const OilDisplayRuntime displayRuntime = startOilDisplayRuntime();
  if (displayRuntime.display == nullptr || displayRuntime.input == nullptr) {
    ESP_LOGE(kTag, "Waveshare display initialization failed");
    return;
  }

  ESP_ERROR_CHECK(
      bsp_display_brightness_set(gSettings.brightnessPercent));

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

  const std::uint64_t uiStartedUs =
      static_cast<std::uint64_t>(esp_timer_get_time());
  const std::uint64_t bootSplashDeadlineUs =
      uiStartedUs + static_cast<std::uint64_t>(gSettings.startupLogoSeconds) *
                        1'000'000U;
  std::uint64_t lastFrameUs = uiStartedUs;
  while (true) {
    const std::uint64_t nowUs =
        static_cast<std::uint64_t>(esp_timer_get_time());
    if (nowUs - lastFrameUs >= kUiFramePeriodUs) {
      // Keep only the newest state. Presentation is paced independently by
      // the CO5300 TE signal, so replaying missed application ticks adds lag.
      lastFrameUs = nowUs;
      if (esp_lv_adapter_lock(100) == ESP_OK) {
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
          renderCalibrationGate(nowUs);
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
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}
