#include "display_runtime.h"

#include "display_clock_profile.h"
#include "esp_err.h"
#include "bsp/display.h"
#include "bsp/esp-bsp.h"
#include "bsp/touch.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_log.h"
#include "esp_lv_adapter.h"
#include "esp_timer.h"

#include <cstdint>

namespace oilgauge {

namespace {

constexpr char kTag[] = "display_runtime";
constexpr gpio_num_t kTeGpio = GPIO_NUM_43;
constexpr int64_t kTeProbeDurationUs = 150'000;
constexpr int64_t kTeMinimumPeriodUs = 8'000;
constexpr int64_t kTeMaximumPeriodUs = 40'000;
constexpr int64_t kTimingLogPeriodUs = 2'000'000;
constexpr uint32_t kTouchReadPeriodMs = 8;
// Line zero makes STESL equivalent to the regular VBlank-only TE mode.
constexpr uint16_t kTeScanLine = 0;
constexpr uint32_t kQspiWriteCommandOpcode = 0x02U << 24;

struct DisplayTimingStats {
  int64_t windowStartUs = 0;
  int64_t renderStartUs = 0;
  int64_t flushStartUs = 0;
  int64_t previousFlushStartUs = 0;
  int64_t renderTotalUs = 0;
  int64_t renderMaximumUs = 0;
  int64_t currentRenderFlushUs = 0;
  int64_t drawTotalUs = 0;
  int64_t drawMaximumUs = 0;
  int64_t flushTotalUs = 0;
  int64_t flushMaximumUs = 0;
  int64_t flushIntervalTotalUs = 0;
  int64_t flushIntervalMaximumUs = 0;
  uint32_t refreshCount = 0;
  uint32_t refreshWithoutFlushCount = 0;
  uint32_t renderCount = 0;
  uint32_t flushCount = 0;
  uint32_t flushIntervalCount = 0;
  bool currentRefreshHadFlush = false;
};

DisplayTimingStats gDisplayTiming;

esp_err_t setTeScanLine(esp_lcd_panel_io_handle_t panelIo) {
  const uint8_t scanLine[] = {
      static_cast<uint8_t>(kTeScanLine >> 8),
      static_cast<uint8_t>(kTeScanLine & 0xFFU),
  };
  const uint32_t command =
      kQspiWriteCommandOpcode | (static_cast<uint32_t>(0x44U) << 8);
  const esp_err_t result =
      esp_lcd_panel_io_tx_param(panelIo, command, scanLine, sizeof(scanLine));
  if (result == ESP_OK) {
    ESP_LOGI(kTag,
             "CO5300 TE scan-line trigger configured at line %u",
             static_cast<unsigned>(kTeScanLine));
  } else {
    ESP_LOGW(kTag,
             "CO5300 TE scan-line trigger failed: %s; retaining VBlank TE",
             esp_err_to_name(result));
  }
  return result;
}

bool probeTeSignal() {
  const gpio_config_t config{
      .pin_bit_mask = 1ULL << kTeGpio,
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_ENABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };
  const esp_err_t configResult = gpio_config(&config);
  if (configResult != ESP_OK) {
    ESP_LOGW(kTag,
             "CO5300 TE probe could not configure GPIO%d: %s",
             static_cast<int>(kTeGpio),
             esp_err_to_name(configResult));
    return false;
  }

  const int64_t startUs = esp_timer_get_time();
  const int64_t deadlineUs = startUs + kTeProbeDurationUs;
  int previousLevel = gpio_get_level(kTeGpio);
  int risingEdges = 0;
  int fallingEdges = 0;
  int validPeriods = 0;
  int measuredHighPulses = 0;
  int64_t previousRisingUs = 0;
  int64_t currentHighStartUs = previousLevel != 0 ? startUs : 0;
  int64_t periodTotalUs = 0;
  int64_t highTotalUs = 0;
  int64_t highMinimumUs = INT64_MAX;
  int64_t highMaximumUs = 0;

  while (esp_timer_get_time() < deadlineUs) {
    const int level = gpio_get_level(kTeGpio);
    if (level == previousLevel) {
      continue;
    }

    const int64_t nowUs = esp_timer_get_time();
    if (level != 0) {
      ++risingEdges;
      if (previousRisingUs != 0) {
        const int64_t periodUs = nowUs - previousRisingUs;
        if (periodUs >= kTeMinimumPeriodUs &&
            periodUs <= kTeMaximumPeriodUs) {
          ++validPeriods;
          periodTotalUs += periodUs;
        }
      }
      previousRisingUs = nowUs;
      currentHighStartUs = nowUs;
    } else {
      ++fallingEdges;
      if (currentHighStartUs != 0) {
        const int64_t highUs = nowUs - currentHighStartUs;
        if (highUs > 0 && highUs < kTeMaximumPeriodUs) {
          ++measuredHighPulses;
          highTotalUs += highUs;
          if (highUs < highMinimumUs) {
            highMinimumUs = highUs;
          }
          if (highUs > highMaximumUs) {
            highMaximumUs = highUs;
          }
        }
      }
      currentHighStartUs = 0;
    }
    previousLevel = level;
  }

  const bool plausible = risingEdges >= 4 && fallingEdges >= 3 &&
                         validPeriods >= 2;
  const uint32_t frequencyMilliHz =
      validPeriods > 0
          ? static_cast<uint32_t>(1'000'000'000LL * validPeriods /
                                  periodTotalUs)
          : 0;
  const uint32_t averageHighUs =
      measuredHighPulses > 0
          ? static_cast<uint32_t>(highTotalUs / measuredHighPulses)
          : 0;
  ESP_LOGI(kTag,
           "CO5300 TE probe on GPIO%d: rising=%d falling=%d periods=%d "
           "frequency=%u.%03u Hz high=%u us (%lld..%lld) result=%s",
           static_cast<int>(kTeGpio),
           risingEdges,
           fallingEdges,
           validPeriods,
           static_cast<unsigned>(frequencyMilliHz / 1000U),
           static_cast<unsigned>(frequencyMilliHz % 1000U),
           static_cast<unsigned>(averageHighUs),
           static_cast<long long>(measuredHighPulses > 0 ? highMinimumUs : 0),
           static_cast<long long>(highMaximumUs),
           plausible ? "usable" : "rejected");
  return plausible;
}

void resetDisplayTimingWindow(DisplayTimingStats& stats, int64_t nowUs) {
  stats.windowStartUs = nowUs;
  stats.renderTotalUs = 0;
  stats.renderMaximumUs = 0;
  stats.drawTotalUs = 0;
  stats.drawMaximumUs = 0;
  stats.flushTotalUs = 0;
  stats.flushMaximumUs = 0;
  stats.flushIntervalTotalUs = 0;
  stats.flushIntervalMaximumUs = 0;
  stats.refreshCount = 0;
  stats.refreshWithoutFlushCount = 0;
  stats.renderCount = 0;
  stats.flushCount = 0;
  stats.flushIntervalCount = 0;
}

void logDisplayTimingWindow(DisplayTimingStats& stats, int64_t nowUs) {
  const int64_t elapsedUs = nowUs - stats.windowStartUs;
  if (elapsedUs < kTimingLogPeriodUs) {
    return;
  }

  const uint32_t transferredMilliFps =
      elapsedUs > 0
          ? static_cast<uint32_t>(
                static_cast<int64_t>(stats.flushCount) * 1'000'000'000LL /
                elapsedUs)
          : 0;
  const int64_t renderAverageUs =
      stats.renderCount > 0 ? stats.renderTotalUs / stats.renderCount : 0;
  const int64_t drawAverageUs =
      stats.renderCount > 0 ? stats.drawTotalUs / stats.renderCount : 0;
  const int64_t flushAverageUs =
      stats.flushCount > 0 ? stats.flushTotalUs / stats.flushCount : 0;
  const int64_t intervalAverageUs =
      stats.flushIntervalCount > 0
          ? stats.flushIntervalTotalUs / stats.flushIntervalCount
          : 0;

  ESP_LOGI(kTag,
           "CO5300 timing: transferred=%u.%03u fps refreshes=%u flushes=%u "
           "idle_refresh=%u render=%lld/%lld us draw=%lld/%lld us "
           "flush=%lld/%lld us "
           "interval=%lld/%lld us",
           static_cast<unsigned>(transferredMilliFps / 1000U),
           static_cast<unsigned>(transferredMilliFps % 1000U),
           static_cast<unsigned>(stats.refreshCount),
           static_cast<unsigned>(stats.flushCount),
           static_cast<unsigned>(stats.refreshWithoutFlushCount),
           static_cast<long long>(renderAverageUs),
           static_cast<long long>(stats.renderMaximumUs),
           static_cast<long long>(drawAverageUs),
           static_cast<long long>(stats.drawMaximumUs),
           static_cast<long long>(flushAverageUs),
           static_cast<long long>(stats.flushMaximumUs),
           static_cast<long long>(intervalAverageUs),
           static_cast<long long>(stats.flushIntervalMaximumUs));
  resetDisplayTimingWindow(stats, nowUs);
}

void recordDisplayTiming(lv_event_t* event) {
  auto& stats = *static_cast<DisplayTimingStats*>(
      lv_event_get_user_data(event));
  const int64_t nowUs = esp_timer_get_time();

  switch (lv_event_get_code(event)) {
    case LV_EVENT_REFR_START:
      stats.currentRefreshHadFlush = false;
      break;
    case LV_EVENT_RENDER_START:
      stats.renderStartUs = nowUs;
      stats.currentRenderFlushUs = 0;
      break;
    case LV_EVENT_RENDER_READY: {
      const int64_t durationUs = nowUs - stats.renderStartUs;
      const int64_t drawDurationUs =
          durationUs >= stats.currentRenderFlushUs
              ? durationUs - stats.currentRenderFlushUs
              : 0;
      stats.renderTotalUs += durationUs;
      if (durationUs > stats.renderMaximumUs) {
        stats.renderMaximumUs = durationUs;
      }
      stats.drawTotalUs += drawDurationUs;
      if (drawDurationUs > stats.drawMaximumUs) {
        stats.drawMaximumUs = drawDurationUs;
      }
      ++stats.renderCount;
      break;
    }
    case LV_EVENT_FLUSH_START:
      stats.currentRefreshHadFlush = true;
      stats.flushStartUs = nowUs;
      if (stats.previousFlushStartUs != 0) {
        const int64_t intervalUs = nowUs - stats.previousFlushStartUs;
        stats.flushIntervalTotalUs += intervalUs;
        if (intervalUs > stats.flushIntervalMaximumUs) {
          stats.flushIntervalMaximumUs = intervalUs;
        }
        ++stats.flushIntervalCount;
      }
      stats.previousFlushStartUs = nowUs;
      break;
    case LV_EVENT_FLUSH_FINISH: {
      const int64_t durationUs = nowUs - stats.flushStartUs;
      stats.currentRenderFlushUs += durationUs;
      stats.flushTotalUs += durationUs;
      if (durationUs > stats.flushMaximumUs) {
        stats.flushMaximumUs = durationUs;
      }
      ++stats.flushCount;
      break;
    }
    case LV_EVENT_REFR_READY:
      ++stats.refreshCount;
      if (!stats.currentRefreshHadFlush) {
        ++stats.refreshWithoutFlushCount;
      }
      logDisplayTimingWindow(stats, nowUs);
      break;
    default:
      break;
  }
}

void roundInvalidationToPanelAlignment(lv_event_t* event) {
  lv_area_t* area = static_cast<lv_area_t*>(lv_event_get_param(event));
  area->x1 = (area->x1 >> 1) << 1;
  area->y1 = (area->y1 >> 1) << 1;
  area->x2 = ((area->x2 >> 1) << 1) + 1;
  area->y2 = ((area->y2 >> 1) << 1) + 1;
}

}  // namespace

OilDisplayRuntime startOilDisplayRuntime() {
  OilDisplayRuntime runtime;

  ESP_LOGI(kTag,
           "CO5300 QSPI clock request: %u MHz",
           static_cast<unsigned>(OIL_GAUGE_DISPLAY_QSPI_HZ / 1'000'000));

  const esp_lv_adapter_config_t adapterConfig{
      .task_stack_size = ESP_LV_ADAPTER_DEFAULT_STACK_SIZE,
      .task_priority = ESP_LV_ADAPTER_DEFAULT_TASK_PRIORITY,
      .task_core_id = ESP_LV_ADAPTER_DEFAULT_TASK_CORE_ID,
      .tick_period_ms = ESP_LV_ADAPTER_DEFAULT_TICK_PERIOD_MS,
      .task_min_delay_ms = ESP_LV_ADAPTER_DEFAULT_TASK_MIN_DELAY_MS,
      .task_max_delay_ms = ESP_LV_ADAPTER_DEFAULT_TASK_MAX_DELAY_MS,
      .stack_in_psram = false,
      .auto_sleep = {
          .enable = false,
          .mode = ESP_LV_ADAPTER_AUTO_SLEEP_MODE_DISABLED,
          .idle_timeout_ms = ESP_LV_ADAPTER_DEFAULT_AUTO_SLEEP_TIMEOUT_MS,
          .callbacks = {},
      },
  };
  if (esp_lv_adapter_init(&adapterConfig) != ESP_OK) {
    ESP_LOGE(kTag, "LVGL adapter initialization failed");
    return runtime;
  }

  const bsp_display_config_t panelConfig{
      .max_transfer_sz =
          BSP_LCD_H_RES * BSP_LCD_V_RES * BSP_LCD_BITS_PER_PIXEL / 8,
  };
  esp_lcd_panel_handle_t panel = nullptr;
  esp_lcd_panel_io_handle_t panelIo = nullptr;
  if (bsp_display_new(&panelConfig, &panel, &panelIo) != ESP_OK) {
    ESP_LOGE(kTag, "CO5300 panel initialization failed");
    return runtime;
  }

  (void)setTeScanLine(panelIo);
  ESP_LOGI(kTag, "CO5300 orientation: preserving Waveshare MADCTL=0xA0");

  if (!probeTeSignal()) {
    ESP_LOGE(kTag,
             "CO5300 TE signal unavailable on GPIO%d; refusing an "
             "unsynchronised fallback",
             static_cast<int>(kTeGpio));
    return runtime;
  }

  esp_lv_adapter_display_config_t displayConfig{};
  displayConfig.panel = panel;
  displayConfig.panel_io = panelIo;
  displayConfig.profile.interface = ESP_LV_ADAPTER_PANEL_IF_OTHER;
  displayConfig.profile.rotation = ESP_LV_ADAPTER_ROTATE_0;
  displayConfig.profile.hor_res = BSP_LCD_H_RES;
  displayConfig.profile.ver_res = BSP_LCD_V_RES;
  displayConfig.profile.buffer_height = BSP_LCD_V_RES;
  displayConfig.profile.use_psram = true;
  displayConfig.profile.enable_ppa_accel = false;
  displayConfig.profile.require_double_buffer = false;
  displayConfig.profile.mono_layout = ESP_LV_ADAPTER_MONO_LAYOUT_NONE;
  displayConfig.tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_TE_SYNC;
  displayConfig.te_sync = {
      .gpio_num = static_cast<int>(kTeGpio),
      .time_tvdl_ms = 13,
      .time_tvdh_ms = 1,
      .bus_freq_hz = OIL_GAUGE_DISPLAY_QSPI_HZ,
      .data_lines = 4,
      .bits_per_pixel = BSP_LCD_BITS_PER_PIXEL,
      .intr_type = GPIO_INTR_POSEDGE,
      .refresh_window_percent = 66,
  };

  runtime.display = esp_lv_adapter_register_display(&displayConfig);
  if (runtime.display == nullptr) {
    ESP_LOGE(kTag, "Full-frame LVGL display registration failed");
    return runtime;
  }
  lv_display_add_event_cb(runtime.display,
                          roundInvalidationToPanelAlignment,
                          LV_EVENT_INVALIDATE_AREA,
                          nullptr);
  gDisplayTiming = {};
  resetDisplayTimingWindow(gDisplayTiming, esp_timer_get_time());
  lv_display_add_event_cb(runtime.display,
                          recordDisplayTiming,
                          LV_EVENT_ALL,
                          &gDisplayTiming);

  bsp_display_cfg_t touchBspConfig{};
  touchBspConfig.touch_flags.swap_xy = 1;
  touchBspConfig.touch_flags.mirror_x = 0;
  touchBspConfig.touch_flags.mirror_y = 1;
  esp_lcd_touch_handle_t touch = nullptr;
  if (bsp_touch_new(&touchBspConfig, &touch) != ESP_OK) {
    ESP_LOGE(kTag, "Touch initialization failed");
    return runtime;
  }
  const esp_lv_adapter_touch_config_t touchConfig{
      .disp = runtime.display,
      .handle = touch,
      .scale = {.x = 1.0F, .y = 1.0F},
      .multi_touch = {
          .mode = ESP_LV_ADAPTER_TOUCH_MODE_SINGLE,
          .pointers = 1,
      },
      .callbacks = {},
  };
  runtime.input = esp_lv_adapter_register_touch(&touchConfig);
  if (runtime.input == nullptr) {
    ESP_LOGE(kTag, "LVGL touch registration failed");
    return runtime;
  }
  lv_timer_set_period(lv_indev_get_read_timer(runtime.input),
                      kTouchReadPeriodMs);

  ESP_LOGI(kTag,
           "CO5300 synchronization: Waveshare MADCTL=0xA0 + adapter "
           "GPIO43 TE_SYNC, single full-frame buffer");

  if (bsp_display_brightness_init() != ESP_OK ||
      esp_lv_adapter_start() != ESP_OK) {
    ESP_LOGE(kTag, "Display runtime start failed");
    runtime = {};
  }
  return runtime;
}

}  // namespace oilgauge
