#include "display_runtime.h"

#include "display_clock_profile.h"
#include "esp_err.h"

#include "bsp/display.h"
#include "bsp/esp-bsp.h"
#include "bsp/touch.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_heap_caps.h"
#include "esp_intr_alloc.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_lv_adapter.h"
#include "esp_memory_utils.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

namespace oilgauge {

namespace {

constexpr char kTag[] = "display_runtime";
constexpr gpio_num_t kTeGpio = GPIO_NUM_43;
constexpr int kDisplayWidth = BSP_LCD_H_RES;
constexpr int kDisplayHeight = BSP_LCD_V_RES;
constexpr std::size_t kRgb565BytesPerPixel = 2;
constexpr std::size_t kFrameBytes =
    static_cast<std::size_t>(kDisplayWidth) * kDisplayHeight *
    kRgb565BytesPerPixel;
constexpr std::size_t kFrameBufferCount = 2;
constexpr int kPanelTransferRows = OIL_GAUGE_DISPLAY_TRANSFER_ROWS;
constexpr std::size_t kPanelTransferBytes =
    static_cast<std::size_t>(kDisplayWidth) * kPanelTransferRows *
    kRgb565BytesPerPixel;
constexpr std::size_t kQueuedBounceBytes =
    kPanelTransferBytes * OIL_GAUGE_DISPLAY_QUEUE_DEPTH;
constexpr int64_t kTeProbeDurationUs = 150'000;
constexpr int64_t kTeMinimumPeriodUs = 8'000;
constexpr int64_t kTeMaximumPeriodUs = 40'000;
constexpr int64_t kTimingLogPeriodUs = 2'000'000;
constexpr TickType_t kTeWaitTicks = pdMS_TO_TICKS(50);
constexpr TickType_t kDmaWaitTicks = pdMS_TO_TICKS(100);
constexpr uint32_t kTouchReadPeriodMs = 8;
constexpr UBaseType_t kPresenterTaskPriority = 7;
constexpr BaseType_t kPresenterTaskCore = 1;
constexpr uint32_t kPresenterTaskStackBytes = 4096;
// Line zero makes STESL equivalent to the regular VBlank-only TE mode.
constexpr uint16_t kTeScanLine = 0;
constexpr uint32_t kQspiWriteCommandOpcode = 0x02U << 24;

static_assert(kFrameBytes == 460'800);
static_assert(kPanelTransferRows > 0);
static_assert(kDisplayHeight % kPanelTransferRows == 0);
static_assert(kQueuedBounceBytes <= 24U * 1024U);

struct PipelineStats {
  int64_t windowStartUs = 0;
  int64_t dmaTotalUs = 0;
  int64_t dmaMaximumUs = 0;
  int64_t presentationIntervalTotalUs = 0;
  int64_t presentationIntervalMaximumUs = 0;
  int64_t previousPresentationStartUs = 0;
  std::uint32_t teEdges = 0;
  std::uint32_t flushes = 0;
  std::uint32_t lvglFrames = 0;
  std::uint32_t presented = 0;
  std::uint32_t dmaCompleted = 0;
  std::uint32_t teTimeouts = 0;
  std::uint32_t dmaErrors = 0;
  std::uint32_t presentationIntervalCount = 0;
};

struct DisplayPipeline {
  esp_lcd_panel_handle_t panel = nullptr;
  esp_lcd_panel_io_handle_t panelIo = nullptr;
  lv_display_t* display = nullptr;
  std::uint8_t* frameBuffers[kFrameBufferCount]{};
  std::uint8_t* pendingFrame = nullptr;
  std::uint64_t nextGeneration = 0;
  SemaphoreHandle_t frameReady = nullptr;
  SemaphoreHandle_t teEdge = nullptr;
  SemaphoreHandle_t dmaDone = nullptr;
  TaskHandle_t presenterTask = nullptr;
  portMUX_TYPE frameMux = portMUX_INITIALIZER_UNLOCKED;
  portMUX_TYPE statsMux = portMUX_INITIALIZER_UNLOCKED;
  std::atomic_bool presenterFailed{false};
  std::atomic_int pendingBrightness{-1};
  PipelineStats stats{};
};

DisplayPipeline gPipeline;

[[nodiscard]] std::uint8_t* allocatePsram(std::size_t bytes,
                                         bool clear) {
  const uint32_t capabilities = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;
  auto* memory = static_cast<std::uint8_t*>(heap_caps_aligned_alloc(
      64, bytes, capabilities));
  if (memory != nullptr && clear) {
    std::memset(memory, 0, bytes);
  }
  return memory;
}

void resetTimingWindow(PipelineStats& stats, int64_t nowUs) {
  const int64_t previousPresentationStartUs =
      stats.previousPresentationStartUs;
  stats = {};
  stats.windowStartUs = nowUs;
  stats.previousPresentationStartUs = previousPresentationStartUs;
}

void maybeLogTiming(DisplayPipeline& pipeline, int64_t nowUs) {
  PipelineStats snapshot;
  int64_t elapsedUs = 0;
  bool shouldLog = false;

  portENTER_CRITICAL(&pipeline.statsMux);
  elapsedUs = nowUs - pipeline.stats.windowStartUs;
  if (elapsedUs >= kTimingLogPeriodUs) {
    snapshot = pipeline.stats;
    resetTimingWindow(pipeline.stats, nowUs);
    shouldLog = true;
  }
  portEXIT_CRITICAL(&pipeline.statsMux);

  if (!shouldLog) {
    return;
  }

  const std::uint32_t milliFps =
      elapsedUs > 0
          ? static_cast<std::uint32_t>(
                static_cast<int64_t>(snapshot.presented) * 1'000'000'000LL /
                elapsedUs)
          : 0;
  const int64_t dmaAverageUs =
      snapshot.dmaCompleted > 0
          ? snapshot.dmaTotalUs / snapshot.dmaCompleted
          : 0;
  const int64_t intervalAverageUs =
      snapshot.presentationIntervalCount > 0
          ? snapshot.presentationIntervalTotalUs /
                snapshot.presentationIntervalCount
          : 0;

  ESP_LOGI(
      kTag,
      "CO5300 pipeline: presented=%u.%03u fps lvgl=%u TE=%u flushes=%u "
      "DMA=%lld/%lld us interval=%lld/%lld us timeouts=%u errors=%u "
      "fatal=%u",
      static_cast<unsigned>(milliFps / 1000U),
      static_cast<unsigned>(milliFps % 1000U),
      static_cast<unsigned>(snapshot.lvglFrames),
      static_cast<unsigned>(snapshot.teEdges),
      static_cast<unsigned>(snapshot.flushes),
      static_cast<long long>(dmaAverageUs),
      static_cast<long long>(snapshot.dmaMaximumUs),
      static_cast<long long>(intervalAverageUs),
      static_cast<long long>(snapshot.presentationIntervalMaximumUs),
      static_cast<unsigned>(snapshot.teTimeouts),
      static_cast<unsigned>(snapshot.dmaErrors),
      pipeline.presenterFailed.load(std::memory_order_relaxed) ? 1U : 0U);
}

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
  int64_t highMinimumUs = std::numeric_limits<int64_t>::max();
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
          highMinimumUs = std::min(highMinimumUs, highUs);
          highMaximumUs = std::max(highMaximumUs, highUs);
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

void IRAM_ATTR teEdgeIsr(void* userContext) {
  auto& pipeline = *static_cast<DisplayPipeline*>(userContext);
  BaseType_t highPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(pipeline.teEdge, &highPriorityTaskWoken);
  portENTER_CRITICAL_ISR(&pipeline.statsMux);
  ++pipeline.stats.teEdges;
  portEXIT_CRITICAL_ISR(&pipeline.statsMux);
  if (highPriorityTaskWoken == pdTRUE) {
    portYIELD_FROM_ISR();
  }
}

bool IRAM_ATTR onColorTransferDone(
    esp_lcd_panel_io_handle_t,
    esp_lcd_panel_io_event_data_t*,
    void* userContext) {
  auto& pipeline = *static_cast<DisplayPipeline*>(userContext);
  BaseType_t highPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(pipeline.dmaDone, &highPriorityTaskWoken);
  return highPriorityTaskWoken == pdTRUE;
}

void applyPendingBrightness(DisplayPipeline& pipeline) {
  const int brightnessPercent =
      pipeline.pendingBrightness.exchange(-1, std::memory_order_acq_rel);
  if (brightnessPercent < 0) {
    return;
  }
  const esp_err_t result = bsp_display_brightness_set(brightnessPercent);
  if (result != ESP_OK) {
    ESP_LOGW(kTag,
             "Unable to apply coalesced display brightness: %s",
             esp_err_to_name(result));
  }
}

void displayPresenterTask(void* argument) {
  auto& pipeline = *static_cast<DisplayPipeline*>(argument);

  while (true) {
    xSemaphoreTake(pipeline.frameReady, portMAX_DELAY);
    std::uint8_t* frame = nullptr;
    portENTER_CRITICAL(&pipeline.frameMux);
    frame = pipeline.pendingFrame;
    pipeline.pendingFrame = nullptr;
    const std::uint64_t generation = pipeline.nextGeneration;
    portEXIT_CRITICAL(&pipeline.frameMux);
    if (frame == nullptr) {
      continue;
    }

    while (xSemaphoreTake(pipeline.teEdge, 0) == pdTRUE) {
    }
    while (xSemaphoreTake(pipeline.teEdge, kTeWaitTicks) != pdTRUE) {
      portENTER_CRITICAL(&pipeline.statsMux);
      ++pipeline.stats.teTimeouts;
      portEXIT_CRITICAL(&pipeline.statsMux);
      maybeLogTiming(pipeline, esp_timer_get_time());
    }

    while (xSemaphoreTake(pipeline.dmaDone, 0) == pdTRUE) {
    }
    const int64_t transferStartUs = esp_timer_get_time();
    const esp_err_t drawResult = esp_lcd_panel_draw_bitmap(
        pipeline.panel,
        0,
        0,
        kDisplayWidth,
        kDisplayHeight,
        frame);
    if (drawResult != ESP_OK) {
      ESP_LOGE(kTag,
               "CO5300 generation %llu could not start: %s",
               static_cast<unsigned long long>(generation),
               esp_err_to_name(drawResult));
      pipeline.presenterFailed.store(true, std::memory_order_relaxed);
      portENTER_CRITICAL(&pipeline.statsMux);
      ++pipeline.stats.dmaErrors;
      portEXIT_CRITICAL(&pipeline.statsMux);
      maybeLogTiming(pipeline, esp_timer_get_time());
      vTaskDelete(nullptr);
      return;
    }

    portENTER_CRITICAL(&pipeline.statsMux);
    if (pipeline.stats.previousPresentationStartUs != 0) {
      const int64_t intervalUs =
          transferStartUs - pipeline.stats.previousPresentationStartUs;
      pipeline.stats.presentationIntervalTotalUs += intervalUs;
      pipeline.stats.presentationIntervalMaximumUs = std::max(
          pipeline.stats.presentationIntervalMaximumUs, intervalUs);
      ++pipeline.stats.presentationIntervalCount;
    }
    pipeline.stats.previousPresentationStartUs = transferStartUs;
    portEXIT_CRITICAL(&pipeline.statsMux);

    if (xSemaphoreTake(pipeline.dmaDone, kDmaWaitTicks) != pdTRUE) {
      ESP_LOGE(kTag,
               "CO5300 generation %llu DMA timed out; preserving its buffer",
               static_cast<unsigned long long>(generation));
      pipeline.presenterFailed.store(true, std::memory_order_relaxed);
      portENTER_CRITICAL(&pipeline.statsMux);
      ++pipeline.stats.dmaErrors;
      portEXIT_CRITICAL(&pipeline.statsMux);
      vTaskDelete(nullptr);
      return;
    }

    const int64_t dmaDurationUs = esp_timer_get_time() - transferStartUs;
    portENTER_CRITICAL(&pipeline.statsMux);
    pipeline.stats.dmaTotalUs += dmaDurationUs;
    pipeline.stats.dmaMaximumUs =
        std::max(pipeline.stats.dmaMaximumUs, dmaDurationUs);
    ++pipeline.stats.presented;
    ++pipeline.stats.dmaCompleted;
    portEXIT_CRITICAL(&pipeline.statsMux);
    // Brightness shares the panel IO with frame transport. Coalesce fast slider
    // events and send command 0x51 only after the prior DMA has completed.
    applyPendingBrightness(pipeline);
    // LVGL may reuse this framebuffer only after the LCD driver reports that
    // the complete QSPI transfer has finished.
    lv_display_flush_ready(pipeline.display);
    maybeLogTiming(pipeline, esp_timer_get_time());
  }
}

void flushToNativeFrame(lv_display_t* display,
                        const lv_area_t*,
                        std::uint8_t* pixels) {
  auto& pipeline = *static_cast<DisplayPipeline*>(
      lv_display_get_user_data(display));
  portENTER_CRITICAL(&pipeline.frameMux);
  pipeline.pendingFrame = pixels;
  ++pipeline.nextGeneration;
  portEXIT_CRITICAL(&pipeline.frameMux);
  portENTER_CRITICAL(&pipeline.statsMux);
  ++pipeline.stats.flushes;
  ++pipeline.stats.lvglFrames;
  portEXIT_CRITICAL(&pipeline.statsMux);
  xSemaphoreGive(pipeline.frameReady);
}

bool initializePipelineResources(DisplayPipeline& pipeline) {
  for (std::size_t index = 0; index < kFrameBufferCount; ++index) {
    pipeline.frameBuffers[index] = allocatePsram(kFrameBytes, true);
  }
  for (const auto* frameBuffer : pipeline.frameBuffers) {
    if (frameBuffer == nullptr || !esp_ptr_external_ram(frameBuffer)) {
      ESP_LOGE(kTag, "Unable to allocate a full framebuffer in PSRAM");
      return false;
    }
  }

  pipeline.frameReady = xSemaphoreCreateBinary();
  pipeline.teEdge = xSemaphoreCreateBinary();
  pipeline.dmaDone = xSemaphoreCreateBinary();
  if (pipeline.frameReady == nullptr || pipeline.teEdge == nullptr ||
      pipeline.dmaDone == nullptr) {
    ESP_LOGE(kTag, "Unable to allocate display-pipeline semaphores");
    return false;
  }
  pipeline.stats.windowStartUs = esp_timer_get_time();
  return true;
}

bool startPresenter(DisplayPipeline& pipeline) {
  const esp_lcd_panel_io_callbacks_t callbacks{
      .on_color_trans_done = onColorTransferDone,
  };
  if (esp_lcd_panel_io_register_event_callbacks(
          pipeline.panelIo, &callbacks, &pipeline) != ESP_OK) {
    ESP_LOGE(kTag, "Unable to register CO5300 DMA completion callback");
    return false;
  }

  const esp_err_t isrServiceResult =
      gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
  if (isrServiceResult != ESP_OK &&
      isrServiceResult != ESP_ERR_INVALID_STATE) {
    ESP_LOGE(kTag,
             "Unable to install GPIO ISR service: %s",
             esp_err_to_name(isrServiceResult));
    return false;
  }
  if (gpio_set_intr_type(kTeGpio, GPIO_INTR_POSEDGE) != ESP_OK ||
      gpio_isr_handler_add(kTeGpio, teEdgeIsr, &pipeline) != ESP_OK ||
      gpio_intr_enable(kTeGpio) != ESP_OK) {
    ESP_LOGE(kTag, "Unable to arm GPIO%d TE interrupt", kTeGpio);
    return false;
  }

  const BaseType_t taskResult = xTaskCreatePinnedToCore(
      displayPresenterTask,
      "display_presenter",
      kPresenterTaskStackBytes,
      &pipeline,
      kPresenterTaskPriority,
      &pipeline.presenterTask,
      kPresenterTaskCore);
  if (taskResult != pdPASS) {
    ESP_LOGE(kTag, "Unable to start the TE display presenter");
    pipeline.presenterTask = nullptr;
    return false;
  }
  return true;
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
      .max_transfer_sz = kPanelTransferBytes,
  };
  if (bsp_display_new(&panelConfig, &gPipeline.panel, &gPipeline.panelIo) !=
      ESP_OK) {
    ESP_LOGE(kTag, "CO5300 panel initialization failed");
    return runtime;
  }

  // Waveshare initializes the panel at 0xA0. Clear MV/MX/MY through the
  // driver's public API so its cached MADCTL value and the controller agree.
  if (esp_lcd_panel_swap_xy(gPipeline.panel, false) != ESP_OK ||
      esp_lcd_panel_mirror(gPipeline.panel, false, false) != ESP_OK) {
    ESP_LOGE(kTag, "Unable to restore the CO5300 native scan order");
    return runtime;
  }
  ESP_LOGI(kTag, "CO5300 native scan: MADCTL=0x00");

  (void)setTeScanLine(gPipeline.panelIo);
  if (!probeTeSignal()) {
    ESP_LOGE(kTag,
             "CO5300 TE signal unavailable on GPIO%d; refusing an "
             "unsynchronised fallback",
             static_cast<int>(kTeGpio));
    return runtime;
  }
  if (!initializePipelineResources(gPipeline) ||
      !startPresenter(gPipeline)) {
    return runtime;
  }

  runtime.display = lv_display_create(BSP_LCD_H_RES, BSP_LCD_V_RES);
  if (runtime.display == nullptr) {
    ESP_LOGE(kTag, "LVGL display creation failed");
    return runtime;
  }
  gPipeline.display = runtime.display;
  lv_display_set_color_format(runtime.display, LV_COLOR_FORMAT_RGB565_SWAPPED);
  lv_display_set_user_data(runtime.display, &gPipeline);
  lv_display_set_buffers(runtime.display,
                         gPipeline.frameBuffers[0],
                         gPipeline.frameBuffers[1],
                         kFrameBytes,
                         LV_DISPLAY_RENDER_MODE_FULL);
  lv_display_set_flush_cb(runtime.display, flushToNativeFrame);

  bsp_display_cfg_t touchBspConfig{};
  // Display and touch both remain in the controller's native coordinates.
  touchBspConfig.touch_flags.swap_xy = 0;
  touchBspConfig.touch_flags.mirror_x = 0;
  touchBspConfig.touch_flags.mirror_y = 0;
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
           "CO5300 synchronization: native scan + FULL double framebuffer + "
           "GPIO43 TE; flush releases only after DMA completion");
  ESP_LOGI(kTag,
           "CO5300 buffers: full=2x%u bytes RGB565_SWAPPED "
           "bounce<=%ux%u internal",
           static_cast<unsigned>(kFrameBytes),
           static_cast<unsigned>(OIL_GAUGE_DISPLAY_QUEUE_DEPTH),
           static_cast<unsigned>(kPanelTransferBytes));

  if (bsp_display_brightness_init() != ESP_OK ||
      esp_lv_adapter_start() != ESP_OK) {
    ESP_LOGE(kTag, "Display runtime start failed");
    runtime = {};
  }
  return runtime;
}

void requestOilDisplayBrightness(std::uint8_t brightnessPercent) {
  gPipeline.pendingBrightness.store(
      std::clamp<int>(brightnessPercent, 5, 100),
      std::memory_order_release);
}

}  // namespace oilgauge
