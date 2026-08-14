#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bsp/display.h"
#include "bsp/esp-bsp.h"
#include "esp_lv_adapter.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"

#include <cstdint>
#include <inttypes.h>

#include "demo_sequence.h"
#include "gauge_core.h"
#include "oil_gauge_ui.h"
#include "settings_store.h"
#include "warning_audio.h"
#include "warning_tone_gate.h"

namespace {

using namespace oilgauge;

constexpr char kTag[] = "oil_gauge";
constexpr std::uint64_t kUiFramePeriodUs = kUiFramePeriodMs * 1'000U;
constexpr std::uint64_t kFpsLogPeriodUs = 2'000'000;
WarningToneGate gWarningToneGate;
GaugeSettings gSettings;

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
      evaluatePressureState(pressure, engine) == PressureState::warning;
  if (gWarningToneGate.update(warningActive)) {
    requestWarningTone();
  }
}

void renderCalibrationGate(std::uint64_t nowUs) {
  (void)gWarningToneGate.update(false);
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
    if (bsp_display_brightness_set(gSettings.brightnessPercent) != ESP_OK) {
      ESP_LOGW(kTag, "Unable to apply display brightness");
    }
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

  lv_display_t* display = bsp_display_start();
  if (display == nullptr) {
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

  lv_indev_t* input = bsp_display_get_input_dev();
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

  if (CONFIG_OIL_GAUGE_DEMO_MODE) {
    ESP_ERROR_CHECK(esp_lv_adapter_fps_stats_enable(display, true));
  }

  std::uint64_t lastFrameUs =
      static_cast<std::uint64_t>(esp_timer_get_time());
  std::uint64_t lastFpsLogUs = lastFrameUs;
  while (true) {
    const std::uint64_t nowUs =
        static_cast<std::uint64_t>(esp_timer_get_time());
    if (nowUs - lastFrameUs >= kUiFramePeriodUs) {
      lastFrameUs += kUiFramePeriodUs;
      if (nowUs - lastFrameUs >= kUiFramePeriodUs * 4U) {
        lastFrameUs = nowUs;
      }
      if (esp_lv_adapter_lock(100) == ESP_OK) {
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
    if (CONFIG_OIL_GAUGE_DEMO_MODE &&
        nowUs - lastFpsLogUs >= kFpsLogPeriodUs) {
      lastFpsLogUs = nowUs;
      std::uint32_t fps = 0;
      const esp_err_t fpsResult = esp_lv_adapter_get_fps(display, &fps);
      if (fpsResult == ESP_OK) {
        if (fps >= 60U) {
          ESP_LOGI(kTag, "Display FPS: %" PRIu32 " (target >=60)", fps);
        } else {
          ESP_LOGW(kTag, "Display FPS: %" PRIu32 " (below target 60)", fps);
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}
