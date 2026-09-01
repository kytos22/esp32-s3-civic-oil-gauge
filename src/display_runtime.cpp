#include "display_runtime.h"

#include "display_clock_profile.h"
#include "block_refresh_tracker.h"
#include "board_pins.h"
#include "frame_damage_policy.h"
#include "frame_slot_policy.h"
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
constexpr gpio_num_t kTeGpio =
    static_cast<gpio_num_t>(board::kDisplayTe);
constexpr int kDisplayWidth = BSP_LCD_H_RES;
constexpr int kDisplayHeight = BSP_LCD_V_RES;
constexpr std::size_t kRgb565BytesPerPixel = 2;
constexpr std::size_t kFrameBytes =
    static_cast<std::size_t>(kDisplayWidth) * kDisplayHeight *
    kRgb565BytesPerPixel;
constexpr std::size_t kFrameBufferCount = 3;
constexpr int kPartialDrawRows = kDamageTileSize;
constexpr std::size_t kPartialDrawBytes =
    static_cast<std::size_t>(kDisplayWidth) * kPartialDrawRows *
    kRgb565BytesPerPixel;
constexpr std::size_t kDamageHistoryCapacity = 8;
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
static_assert(kDamageFrameWidth == kDisplayWidth);
static_assert(kDamageFrameHeight == kDisplayHeight);
static_assert(kPartialDrawRows == 32);
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
  int64_t readyWaitTotalUs = 0;
  int64_t readyWaitMaximumUs = 0;
  std::uint32_t teEdges = 0;
  std::uint32_t flushes = 0;
  std::uint32_t lvglFrames = 0;
  std::uint32_t presented = 0;
  std::uint32_t dmaCompleted = 0;
  std::uint32_t teTimeouts = 0;
  std::uint32_t dmaErrors = 0;
  std::uint32_t presentationIntervalCount = 0;
  std::uint32_t readyWaitCount = 0;
  std::uint32_t renderRequests = 0;
  std::uint32_t damageTiles = 0;
  std::uint32_t blockFlushes = 0;
  std::uint32_t blockBytes = 0;
  int64_t renderTotalUs = 0;
  int64_t renderMaximumUs = 0;
  std::uint32_t renderCompleted = 0;
  int64_t producerWakeTotalUs = 0;
  int64_t producerWakeMaximumUs = 0;
  std::uint32_t producerWakeCount = 0;
};

struct DisplayPipeline {
  esp_lcd_panel_handle_t panel = nullptr;
  esp_lcd_panel_io_handle_t panelIo = nullptr;
  lv_display_t* display = nullptr;
  std::uint8_t* frameBuffers[kFrameBufferCount]{};
  std::uint8_t* partialDrawBuffer = nullptr;
  FrameSlotMetadata slots[kFrameBufferCount]{};
  std::uint8_t* pendingFrame = nullptr;
  int pendingSlot = -1;
  int inFlightSlot = -1;
  int renderSlot = -1;
  int64_t readySinceUs = 0;
  int64_t renderStartedUs = 0;
  std::uint64_t nextGeneration = 0;
  std::uint64_t activeGeneration = 0;
  DamageTiles pendingDamage{};
  DamageTiles activeDamage{};
  DamageHistoryEntry damageHistory[kDamageHistoryCapacity]{};
  std::size_t damageHistoryEntries = 0;
  bool replayingDamage = false;
  bool acceptDamageEvents = true;
  BlockRefreshTracker refreshTracker{};
  SemaphoreHandle_t frameReady = nullptr;
  SemaphoreHandle_t teEdge = nullptr;
  SemaphoreHandle_t dmaDone = nullptr;
  TaskHandle_t presenterTask = nullptr;
  std::atomic<TaskHandle_t> producerTask{nullptr};
  portMUX_TYPE frameMux = portMUX_INITIALIZER_UNLOCKED;
  portMUX_TYPE statsMux = portMUX_INITIALIZER_UNLOCKED;
  std::atomic_bool presenterFailed{false};
  std::atomic_bool frameRequested{false};
  std::atomic_bool damagePending{false};
  std::atomic<std::int64_t> renderRequestStartedUs{0};
  std::atomic_uint32_t presentedMilliFps{0};
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
  pipeline.presentedMilliFps.store(milliFps, std::memory_order_relaxed);
  const int64_t dmaAverageUs =
      snapshot.dmaCompleted > 0
          ? snapshot.dmaTotalUs / snapshot.dmaCompleted
          : 0;
  const int64_t intervalAverageUs =
      snapshot.presentationIntervalCount > 0
          ? snapshot.presentationIntervalTotalUs /
                snapshot.presentationIntervalCount
          : 0;
  const int64_t readyWaitAverageUs =
      snapshot.readyWaitCount > 0
          ? snapshot.readyWaitTotalUs / snapshot.readyWaitCount
          : 0;
  const int64_t renderAverageUs =
      snapshot.renderCompleted > 0
          ? snapshot.renderTotalUs / snapshot.renderCompleted
          : 0;
  const int64_t producerWakeAverageUs =
      snapshot.producerWakeCount > 0
          ? snapshot.producerWakeTotalUs / snapshot.producerWakeCount
          : 0;
  ESP_LOGI(
      kTag,
      "CO5300 pipeline: presented=%u.%03u fps lvgl=%u TE=%u flushes=%u "
      "DMA=%lld/%lld us interval=%lld/%lld us ready=%lld/%lld us "
      "render=%lld/%lld us wake=%lld/%lld us tiles=%u blocks=%u bytes=%u "
      "render_req=%u timeouts=%u errors=%u "
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
      static_cast<long long>(readyWaitAverageUs),
      static_cast<long long>(snapshot.readyWaitMaximumUs),
      static_cast<long long>(renderAverageUs),
      static_cast<long long>(snapshot.renderMaximumUs),
      static_cast<long long>(producerWakeAverageUs),
      static_cast<long long>(snapshot.producerWakeMaximumUs),
      static_cast<unsigned>(snapshot.damageTiles),
      static_cast<unsigned>(snapshot.blockFlushes),
      static_cast<unsigned>(snapshot.blockBytes),
      static_cast<unsigned>(snapshot.renderRequests),
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

void damageEventCallback(lv_event_t* event) {
  auto* pipeline = static_cast<DisplayPipeline*>(lv_event_get_user_data(event));
  const auto* area = static_cast<const lv_area_t*>(lv_event_get_param(event));
  if (pipeline == nullptr || area == nullptr || pipeline->replayingDamage ||
      !pipeline->acceptDamageEvents) {
    return;
  }
  pipeline->pendingDamage.markArea({area->x1, area->y1, area->x2, area->y2});
  pipeline->damagePending.store(true, std::memory_order_release);
}

// LVGL's built-in LV_EVENT_REFR_REQUEST callback resumes the display refresh
// timer whenever an object is invalidated. That created a second refresh
// authority in the first triple-buffer experiment: the adapter task could
// enter flush_cb without a canvas owned by the producer. This callback runs
// synchronously after the built-in callback and immediately pauses the timer
// again. Manual lv_refr_now() calls still work with a paused timer.
void holdAutomaticRefreshTimer(lv_event_t* event) {
  auto* display = static_cast<lv_display_t*>(lv_event_get_target(event));
  if (display == nullptr) {
    return;
  }
  lv_timer_t* refreshTimer = lv_display_get_refr_timer(display);
  if (refreshTimer != nullptr) {
    lv_timer_pause(refreshTimer);
  }
}

void invalidateDamageTiles(DisplayPipeline& pipeline,
                           lv_obj_t* screen,
                           const DamageTiles& damage) {
  pipeline.replayingDamage = true;
  int openX1 = -1;
  int openX2 = -1;
  int openY1 = -1;
  for (int row = 0; row <= kDamageTileRows; ++row) {
    int firstColumn = -1;
    int lastColumn = -1;
    if (row < kDamageTileRows) {
      for (int column = 0; column < kDamageTileColumns; ++column) {
        if (damage.marked(row, column)) {
          if (firstColumn < 0) {
            firstColumn = column;
          }
          lastColumn = column;
        }
      }
    }
    const int x1 = firstColumn < 0 ? -1 : firstColumn * kDamageTileSize;
    const int x2 = lastColumn < 0
                       ? -1
                       : std::min(kDisplayWidth - 1,
                                  (lastColumn + 1) * kDamageTileSize - 1);
    if (row > 0 && openX1 >= 0 && (x1 != openX1 || x2 != openX2)) {
      lv_area_t area{
          openX1,
          openY1,
          openX2,
          std::min(kDisplayHeight - 1, row * kDamageTileSize - 1),
      };
      lv_obj_invalidate_area(screen, &area);
      openX1 = -1;
    }
    if (x1 >= 0 && openX1 < 0) {
      openX1 = x1;
      openX2 = x2;
      openY1 = row * kDamageTileSize;
    }
  }
  pipeline.replayingDamage = false;
}

void copyBlockIntoFrame(std::uint8_t* destination,
                        const lv_area_t& area,
                        const std::uint8_t* pixels) {
  const std::size_t rowBytes =
      static_cast<std::size_t>(lv_area_get_width(&area)) *
      kRgb565BytesPerPixel;
  const int rows = lv_area_get_height(&area);
  for (int row = 0; row < rows; ++row) {
    const std::size_t destinationOffset =
        (static_cast<std::size_t>(area.y1 + row) * kDisplayWidth + area.x1) *
        kRgb565BytesPerPixel;
    std::memcpy(destination + destinationOffset,
                pixels + static_cast<std::size_t>(row) * rowBytes,
                rowBytes);
  }
}

void displayPresenterTask(void* argument) {
  auto& pipeline = *static_cast<DisplayPipeline*>(argument);

  while (true) {
    xSemaphoreTake(pipeline.frameReady, portMAX_DELAY);
    while (xSemaphoreTake(pipeline.teEdge, 0) == pdTRUE) {
    }
    while (xSemaphoreTake(pipeline.teEdge, kTeWaitTicks) != pdTRUE) {
      portENTER_CRITICAL(&pipeline.statsMux);
      ++pipeline.stats.teTimeouts;
      portEXIT_CRITICAL(&pipeline.statsMux);
      maybeLogTiming(pipeline, esp_timer_get_time());
    }

    std::uint8_t* frame = nullptr;
    int slot = -1;
    int64_t readySinceUs = 0;
    portENTER_CRITICAL(&pipeline.frameMux);
    frame = pipeline.pendingFrame;
    slot = pipeline.pendingSlot;
    readySinceUs = pipeline.readySinceUs;
    pipeline.pendingFrame = nullptr;
    pipeline.pendingSlot = -1;
    pipeline.readySinceUs = 0;
    pipeline.inFlightSlot = slot;
    if (slot >= 0) {
      pipeline.slots[static_cast<std::size_t>(slot)].state =
          FrameSlotState::inFlight;
    }
    const std::uint64_t generation =
        slot >= 0
            ? pipeline.slots[static_cast<std::size_t>(slot)].generation
            : 0;
    portEXIT_CRITICAL(&pipeline.frameMux);
    if (frame == nullptr || slot < 0) {
      continue;
    }

    while (xSemaphoreTake(pipeline.dmaDone, 0) == pdTRUE) {
    }
    const int64_t transferStartUs = esp_timer_get_time();
    // Ownership has moved from READY to IN_FLIGHT on this TE edge. Publish one
    // render request to app_main on the other core before feeding the bounded
    // DMA queue, so UI update + LVGL rendering can overlap this transfer without
    // introducing a second task that competes for LVGL's global lock.
    pipeline.renderRequestStartedUs.store(transferStartUs,
                                          std::memory_order_release);
    pipeline.frameRequested.store(true, std::memory_order_release);
    const TaskHandle_t producerTask =
        pipeline.producerTask.load(std::memory_order_acquire);
    if (producerTask != nullptr) {
      xTaskNotifyGive(producerTask);
    }
    portENTER_CRITICAL(&pipeline.statsMux);
    ++pipeline.stats.renderRequests;
    portEXIT_CRITICAL(&pipeline.statsMux);
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
    if (readySinceUs > 0) {
      const int64_t readyWaitUs = transferStartUs - readySinceUs;
      pipeline.stats.readyWaitTotalUs += readyWaitUs;
      pipeline.stats.readyWaitMaximumUs =
          std::max(pipeline.stats.readyWaitMaximumUs, readyWaitUs);
      ++pipeline.stats.readyWaitCount;
    }
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

    // ESP-IDF 6.0.2 may split this PSRAM bitmap into bounded SPI chunks, but
    // esp_lcd_panel_io_spi marks en_trans_done_cb only on the final chunk. The
    // semaphore therefore releases ownership after the complete bitmap, not
    // after the first eight-row staging transfer.
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
    portENTER_CRITICAL(&pipeline.frameMux);
    pipeline.slots[static_cast<std::size_t>(slot)].state =
        FrameSlotState::free;
    pipeline.inFlightSlot = -1;
    portEXIT_CRITICAL(&pipeline.frameMux);
    // Brightness shares the panel IO with frame transport. Coalesce fast slider
    // events and send command 0x51 only after the prior DMA has completed.
    applyPendingBrightness(pipeline);
    // The PARTIAL LVGL draw buffer was released synchronously after its block
    // copy. Only the project-owned complete presentation frame remains guarded
    // until this DMA completion.
    maybeLogTiming(pipeline, esp_timer_get_time());
  }
}

void flushToNativeFrame(lv_display_t* display,
                        const lv_area_t* area,
                        std::uint8_t* pixels) {
  auto& pipeline = *static_cast<DisplayPipeline*>(
      lv_display_get_user_data(display));
  const int slot = pipeline.renderSlot;
  if (slot < 0 || area == nullptr || pixels == nullptr) {
    ESP_LOGE(kTag, "LVGL flushed a block without an owned destination");
    pipeline.presenterFailed.store(true, std::memory_order_relaxed);
    lv_display_flush_ready(display);
    return;
  }
  lv_timer_pause(lv_display_get_refr_timer(display));
  copyBlockIntoFrame(pipeline.frameBuffers[static_cast<std::size_t>(slot)],
                     *area,
                     pixels);
  const std::uint32_t copiedBytes =
      static_cast<std::uint32_t>(lv_area_get_width(area) *
                                 lv_area_get_height(area) *
                                 kRgb565BytesPerPixel);
  const bool lastBlock = lv_display_flush_is_last(display);
  pipeline.refreshTracker.onFlush(lastBlock);
  portENTER_CRITICAL(&pipeline.statsMux);
  ++pipeline.stats.blockFlushes;
  pipeline.stats.blockBytes += copiedBytes;
  portEXIT_CRITICAL(&pipeline.statsMux);
  // The block copy is synchronous; LVGL may immediately reuse its small PARTIAL
  // draw buffer. The complete destination frame has separate project ownership.
  lv_display_flush_ready(display);
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
  pipeline.partialDrawBuffer = allocatePsram(kPartialDrawBytes, false);
  if (pipeline.partialDrawBuffer == nullptr ||
      !esp_ptr_external_ram(pipeline.partialDrawBuffer)) {
    ESP_LOGE(kTag, "Unable to allocate the PARTIAL LVGL draw buffer in PSRAM");
    return false;
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
                         gPipeline.partialDrawBuffer,
                         nullptr,
                         kPartialDrawBytes,
                         LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(runtime.display, flushToNativeFrame);
  lv_display_add_event_cb(runtime.display,
                          damageEventCallback,
                          LV_EVENT_INVALIDATE_AREA,
                          &gPipeline);
  lv_display_add_event_cb(runtime.display,
                          holdAutomaticRefreshTimer,
                          LV_EVENT_REFR_REQUEST,
                          nullptr);
  // Keep the timer object so lv_refr_now() can call it manually, but never let
  // the adapter task run it. The event guard above neutralizes every later
  // LV_EVENT_REFR_REQUEST, including touch/scroll invalidations.
  lv_timer_pause(lv_display_get_refr_timer(runtime.display));

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
           "CO5300 synchronization: native scan + PARTIAL block compositor + "
           "three persistent full frames + GPIO43 TE");
  ESP_LOGI(kTag,
           "CO5300 buffers: present=3x%u partial=%u bytes "
           "RGB565_SWAPPED "
           "bounce<=%ux%u internal",
           static_cast<unsigned>(kFrameBytes),
           static_cast<unsigned>(kPartialDrawBytes),
           static_cast<unsigned>(OIL_GAUGE_DISPLAY_QUEUE_DEPTH),
           static_cast<unsigned>(kPanelTransferBytes));

  if (bsp_display_brightness_init() != ESP_OK ||
      esp_lv_adapter_start() != ESP_OK) {
    ESP_LOGE(kTag, "Display runtime start failed");
    runtime = {};
  }
  return runtime;
}

void startOilDisplayPresentation() {
  // Called only after createOilGaugeUi() has completed. This is the bootstrap
  // request; all later requests are published by READY -> IN_FLIGHT at TE.
  gPipeline.producerTask.store(xTaskGetCurrentTaskHandle(),
                               std::memory_order_release);
  gPipeline.renderRequestStartedUs.store(esp_timer_get_time(),
                                         std::memory_order_release);
  gPipeline.pendingDamage.markFull();
  gPipeline.damagePending.store(true, std::memory_order_release);
  gPipeline.frameRequested.store(true, std::memory_order_release);
}

bool oilDisplayBlockFramePending() {
  return gPipeline.frameRequested.load(std::memory_order_acquire) &&
         gPipeline.damagePending.load(std::memory_order_acquire);
}

bool beginOilDisplayBlockFrame(lv_obj_t* screen) {
  auto& pipeline = gPipeline;
  if (screen == nullptr || !oilDisplayBlockFramePending() ||
      pipeline.pendingDamage.empty()) {
    return false;
  }

  int slot = -1;
  std::uint64_t slotGeneration = 0;
  portENTER_CRITICAL(&pipeline.frameMux);
  slot = selectRenderSlot(pipeline.slots);
  if (slot >= 0) {
    slotGeneration =
        pipeline.slots[static_cast<std::size_t>(slot)].generation;
    pipeline.slots[static_cast<std::size_t>(slot)].state =
        FrameSlotState::rendering;
    pipeline.renderSlot = slot;
  }
  portEXIT_CRITICAL(&pipeline.frameMux);
  if (slot < 0) {
    return false;
  }

  pipeline.frameRequested.store(false, std::memory_order_release);
  const int64_t renderRequestStartedUs =
      pipeline.renderRequestStartedUs.exchange(0, std::memory_order_acq_rel);
  pipeline.activeDamage = pipeline.pendingDamage;
  pipeline.pendingDamage.clear();
  pipeline.damagePending.store(false, std::memory_order_release);
  pipeline.acceptDamageEvents = false;
  pipeline.refreshTracker.begin();
  pipeline.activeGeneration = pipeline.nextGeneration + 1;
  bool historyComplete = true;
  DamageTiles frameDamage = damageSince(pipeline.damageHistory,
                                        pipeline.damageHistoryEntries,
                                        slotGeneration,
                                        pipeline.nextGeneration,
                                        historyComplete);
  frameDamage.merge(pipeline.activeDamage);
  if (!historyComplete) {
    ESP_LOGW(kTag,
             "Damage history gap for slot %d (%llu -> %llu); full redraw",
             slot,
             static_cast<unsigned long long>(slotGeneration),
             static_cast<unsigned long long>(pipeline.activeGeneration));
  }
  pipeline.renderStartedUs = esp_timer_get_time();
  portENTER_CRITICAL(&pipeline.statsMux);
  pipeline.stats.damageTiles += frameDamage.tileCount();
  if (renderRequestStartedUs > 0) {
    const int64_t producerWakeUs =
        pipeline.renderStartedUs - renderRequestStartedUs;
    pipeline.stats.producerWakeTotalUs += producerWakeUs;
    pipeline.stats.producerWakeMaximumUs =
        std::max(pipeline.stats.producerWakeMaximumUs, producerWakeUs);
    ++pipeline.stats.producerWakeCount;
  }
  portEXIT_CRITICAL(&pipeline.statsMux);
  invalidateDamageTiles(pipeline, screen, frameDamage);
  return true;
}

void finishOilDisplayBlockFrameAttempt() {
  auto& pipeline = gPipeline;
  pipeline.acceptDamageEvents = true;
  const int slot = pipeline.renderSlot;
  if (slot < 0) {
    return;
  }
  if (pipeline.refreshTracker.finish() == BlockRefreshOutcome::ready) {
    const int64_t nowUs = esp_timer_get_time();
    const int64_t renderDurationUs = nowUs - pipeline.renderStartedUs;
    const std::size_t historyIndex = static_cast<std::size_t>(
        (pipeline.activeGeneration - 1U) % kDamageHistoryCapacity);
    pipeline.damageHistory[historyIndex] = {
        pipeline.activeGeneration,
        pipeline.activeDamage,
    };
    pipeline.damageHistoryEntries = std::min(
        pipeline.damageHistoryEntries + 1, kDamageHistoryCapacity);
    pipeline.nextGeneration = pipeline.activeGeneration;

    portENTER_CRITICAL(&pipeline.frameMux);
    if (pipeline.pendingFrame != nullptr ||
        pipeline.slots[static_cast<std::size_t>(slot)].state !=
            FrameSlotState::rendering) {
      portEXIT_CRITICAL(&pipeline.frameMux);
      ESP_LOGE(kTag, "Block compositor ownership violation on slot %d", slot);
      pipeline.presenterFailed.store(true, std::memory_order_relaxed);
      return;
    }
    pipeline.pendingFrame =
        pipeline.frameBuffers[static_cast<std::size_t>(slot)];
    pipeline.pendingSlot = slot;
    pipeline.readySinceUs = nowUs;
    pipeline.slots[static_cast<std::size_t>(slot)] = {
        FrameSlotState::ready,
        pipeline.activeGeneration,
    };
    pipeline.renderSlot = -1;
    portEXIT_CRITICAL(&pipeline.frameMux);
    portENTER_CRITICAL(&pipeline.statsMux);
    ++pipeline.stats.flushes;
    ++pipeline.stats.lvglFrames;
    pipeline.stats.renderTotalUs += renderDurationUs;
    pipeline.stats.renderMaximumUs =
        std::max(pipeline.stats.renderMaximumUs, renderDurationUs);
    ++pipeline.stats.renderCompleted;
    portEXIT_CRITICAL(&pipeline.statsMux);
    xSemaphoreGive(pipeline.frameReady);
    return;
  }

  portENTER_CRITICAL(&pipeline.frameMux);
  if (pipeline.renderSlot == slot &&
      pipeline.slots[static_cast<std::size_t>(slot)].state ==
          FrameSlotState::rendering) {
    pipeline.slots[static_cast<std::size_t>(slot)].state =
        FrameSlotState::free;
    pipeline.renderSlot = -1;
  }
  portEXIT_CRITICAL(&pipeline.frameMux);
  pipeline.pendingDamage.merge(pipeline.activeDamage);
  pipeline.damagePending.store(!pipeline.pendingDamage.empty(),
                               std::memory_order_release);
  pipeline.frameRequested.store(true, std::memory_order_release);
  ESP_LOGW(kTag, "PARTIAL refresh produced no complete presentation frame");
}

void waitForOilDisplayWork(std::uint32_t maximumWaitMs) {
  const TickType_t waitTicks = maximumWaitMs == 0
                                   ? 0
                                   : std::max<TickType_t>(
                                         1, pdMS_TO_TICKS(maximumWaitMs));
  (void)ulTaskNotifyTake(pdTRUE, waitTicks);
}

std::uint32_t oilDisplayPresentedMilliFps() {
  return gPipeline.presentedMilliFps.load(std::memory_order_relaxed);
}

void requestOilDisplayBrightness(std::uint8_t brightnessPercent) {
  gPipeline.pendingBrightness.store(
      std::clamp<int>(brightnessPercent, 5, 100),
      std::memory_order_release);
}

}  // namespace oilgauge
