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
#include "warning_audio.h"
#include "warning_tone_gate.h"

namespace {

using namespace oilgauge;

constexpr char kTag[] = "oil_gauge";
constexpr std::uint64_t kUiFramePeriodUs = kUiFramePeriodMs * 1'000U;
constexpr std::uint64_t kFpsLogPeriodUs = 2'000'000;
WarningToneGate gWarningToneGate;

void renderDemoFrame(std::uint64_t nowUs) {
  const DemoFrame frame = demoFrameAt(nowUs);
  const bool blinkPhaseOn = warningBlinkPhaseOn(nowUs);
  const ConvertedValue pressure{frame.pressurePsi, Fault::none};
  const ConvertedValue temperature{frame.temperatureC, Fault::none};
  const EngineState engine{true, frame.rpm};

  updateOilGaugeUi(
      pressure,
      temperature,
      engine,
      blinkPhaseOn,
      false);

  const bool warningActive =
      evaluatePressureState(pressure, engine) == PressureState::warning;
  if (gWarningToneGate.update(warningActive)) {
    requestWarningTone();
  }
}

void renderCalibrationGate() {
  updateOilGaugeUi(
      {0.0, Fault::calibrationMissing},
      {0.0, Fault::calibrationMissing},
      EngineState{false, 0},
      true,
      false);
}

}  // namespace

extern "C" void app_main(void) {
  ESP_LOGI(kTag, "Civic oil gauge starting on ESP-IDF %s", IDF_VER);
  ESP_LOGI(kTag,
           "Demo mode: %s",
           CONFIG_OIL_GAUGE_DEMO_MODE ? "enabled" : "disabled");

  lv_display_t* display = bsp_display_start();
  if (display == nullptr) {
    ESP_LOGE(kTag, "Waveshare display initialization failed");
    return;
  }

  ESP_ERROR_CHECK(
      bsp_display_brightness_set(CONFIG_OIL_GAUGE_BRIGHTNESS_PERCENT));

  if (CONFIG_OIL_GAUGE_DEMO_MODE &&
      CONFIG_OIL_GAUGE_DEMO_WARNING_AUDIO) {
    if (!initWarningAudio()) {
      ESP_LOGW(kTag, "Demo will continue without warning audio");
    }
  }

  const esp_err_t initialLockResult = esp_lv_adapter_lock(-1);
  if (initialLockResult != ESP_OK) {
    ESP_LOGE(kTag,
             "Unable to acquire the initial LVGL display lock: %s",
             esp_err_to_name(initialLockResult));
    return;
  }
  createOilGaugeUi(lv_screen_active());
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
        if (CONFIG_OIL_GAUGE_DEMO_MODE) {
          renderDemoFrame(nowUs);
        } else {
          renderCalibrationGate();
        }
        esp_lv_adapter_unlock();
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
