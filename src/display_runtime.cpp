#include "display_runtime.h"

#include "display_clock_profile.h"
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
constexpr int kDrawBufferRows = 120;
constexpr std::size_t kDrawBufferBytes =
    static_cast<std::size_t>(kDisplayWidth) * kDrawBufferRows *
    kRgb565BytesPerPixel;
constexpr int kRotationTilePixels = 32;
constexpr std::size_t kTransmitSlotCount = 2;
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

static_assert(kTransmitSlotCount == 2);
static_assert(kFrameBytes == 460'800);

struct PipelineStats {
  int64_t windowStartUs = 0;
  int64_t rotateTotalUs = 0;
  int64_t rotateMaximumUs = 0;
  int64_t snapshotTotalUs = 0;
  int64_t snapshotMaximumUs = 0;
  int64_t dmaTotalUs = 0;
  int64_t dmaMaximumUs = 0;
  int64_t presentationIntervalTotalUs = 0;
  int64_t presentationIntervalMaximumUs = 0;
  int64_t previousPresentationStartUs = 0;
  std::uint64_t rotatedPixels = 0;
  std::uint32_t teEdges = 0;
  std::uint32_t flushes = 0;
  std::uint32_t lvglFrames = 0;
  std::uint32_t snapshots = 0;
  std::uint32_t overwrittenReady = 0;
  std::uint32_t droppedReady = 0;
  std::uint32_t presented = 0;
  std::uint32_t dmaCompleted = 0;
  std::uint32_t teTimeouts = 0;
  std::uint32_t dmaErrors = 0;
  std::uint32_t noSnapshotSlot = 0;
  std::uint32_t presentationIntervalCount = 0;
};

struct DisplayPipeline {
  esp_lcd_panel_handle_t panel = nullptr;
  esp_lcd_panel_io_handle_t panelIo = nullptr;
  lv_display_t* display = nullptr;
  std::uint8_t* canvas = nullptr;
  std::uint8_t* drawBuffers[2]{};
  std::uint8_t* transmitBuffers[kTransmitSlotCount]{};
  FrameSlotMetadata slots[kTransmitSlotCount]{};
  std::uint64_t nextGeneration = 0;
  SemaphoreHandle_t frameReady = nullptr;
  SemaphoreHandle_t teEdge = nullptr;
  SemaphoreHandle_t dmaDone = nullptr;
  TaskHandle_t presenterTask = nullptr;
  portMUX_TYPE slotMux = portMUX_INITIALIZER_UNLOCKED;
  portMUX_TYPE statsMux = portMUX_INITIALIZER_UNLOCKED;
  PipelineStats stats{};
};

DisplayPipeline gPipeline;

[[nodiscard]] std::uint8_t* allocatePsram(std::size_t bytes,
                                         bool clear,
                                         bool dmaCapable = false) {
  const uint32_t capabilities =
      MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT |
      (dmaCapable ? MALLOC_CAP_DMA : 0);
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
  const int64_t rotateAverageUs =
      snapshot.flushes > 0 ? snapshot.rotateTotalUs / snapshot.flushes : 0;
  const int64_t snapshotAverageUs =
      snapshot.snapshots > 0
          ? snapshot.snapshotTotalUs / snapshot.snapshots
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
      "CO5300 pipeline: presented=%u.%03u fps lvgl=%u snapshots=%u "
      "overwritten=%u dropped=%u TE=%u flushes=%u pixels=%llu "
      "rotate=%lld/%lld us snapshot=%lld/%lld us DMA=%lld/%lld us "
      "interval=%lld/%lld us timeouts=%u errors=%u no_slot=%u",
      static_cast<unsigned>(milliFps / 1000U),
      static_cast<unsigned>(milliFps % 1000U),
      static_cast<unsigned>(snapshot.lvglFrames),
      static_cast<unsigned>(snapshot.snapshots),
      static_cast<unsigned>(snapshot.overwrittenReady),
      static_cast<unsigned>(snapshot.droppedReady),
      static_cast<unsigned>(snapshot.teEdges),
      static_cast<unsigned>(snapshot.flushes),
      static_cast<unsigned long long>(snapshot.rotatedPixels),
      static_cast<long long>(rotateAverageUs),
      static_cast<long long>(snapshot.rotateMaximumUs),
      static_cast<long long>(snapshotAverageUs),
      static_cast<long long>(snapshot.snapshotMaximumUs),
      static_cast<long long>(dmaAverageUs),
      static_cast<long long>(snapshot.dmaMaximumUs),
      static_cast<long long>(intervalAverageUs),
      static_cast<long long>(snapshot.presentationIntervalMaximumUs),
      static_cast<unsigned>(snapshot.teTimeouts),
      static_cast<unsigned>(snapshot.dmaErrors),
      static_cast<unsigned>(snapshot.noSnapshotSlot));
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

bool anyReadySlot(DisplayPipeline& pipeline) {
  bool ready = false;
  portENTER_CRITICAL(&pipeline.slotMux);
  ready = selectNewestReadySlot(pipeline.slots) >= 0;
  portEXIT_CRITICAL(&pipeline.slotMux);
  return ready;
}

int claimNewestReadySlot(DisplayPipeline& pipeline,
                         std::uint64_t& generation) {
  int selected = -1;
  std::uint32_t dropped = 0;
  portENTER_CRITICAL(&pipeline.slotMux);
  selected = selectNewestReadySlot(pipeline.slots);
  if (selected >= 0) {
    generation = pipeline.slots[selected].generation;
    for (std::size_t index = 0; index < kTransmitSlotCount; ++index) {
      if (static_cast<int>(index) == selected) {
        pipeline.slots[index].state = FrameSlotState::inFlight;
      } else if (pipeline.slots[index].state == FrameSlotState::ready) {
        pipeline.slots[index].state = FrameSlotState::free;
        ++dropped;
      }
    }
  }
  portEXIT_CRITICAL(&pipeline.slotMux);

  if (dropped > 0) {
    portENTER_CRITICAL(&pipeline.statsMux);
    pipeline.stats.droppedReady += dropped;
    portEXIT_CRITICAL(&pipeline.statsMux);
  }
  return selected;
}

void releaseTransmitSlot(DisplayPipeline& pipeline, int slot) {
  portENTER_CRITICAL(&pipeline.slotMux);
  pipeline.slots[slot].state = FrameSlotState::free;
  portEXIT_CRITICAL(&pipeline.slotMux);
}

void displayPresenterTask(void* argument) {
  auto& pipeline = *static_cast<DisplayPipeline*>(argument);

  while (true) {
    xSemaphoreTake(pipeline.frameReady, portMAX_DELAY);
    if (!anyReadySlot(pipeline)) {
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

    std::uint64_t generation = 0;
    const int slot = claimNewestReadySlot(pipeline, generation);
    if (slot < 0) {
      continue;
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
        pipeline.transmitBuffers[slot]);
    if (drawResult != ESP_OK) {
      ESP_LOGE(kTag,
               "CO5300 generation %llu could not start: %s",
               static_cast<unsigned long long>(generation),
               esp_err_to_name(drawResult));
      releaseTransmitSlot(pipeline, slot);
      portENTER_CRITICAL(&pipeline.statsMux);
      ++pipeline.stats.dmaErrors;
      portEXIT_CRITICAL(&pipeline.statsMux);
      continue;
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
               "CO5300 generation %llu DMA timed out; preserving its "
               "IN_FLIGHT buffer",
               static_cast<unsigned long long>(generation));
      portENTER_CRITICAL(&pipeline.statsMux);
      ++pipeline.stats.dmaErrors;
      portEXIT_CRITICAL(&pipeline.statsMux);
      vTaskDelete(nullptr);
      return;
    }

    const int64_t dmaDurationUs = esp_timer_get_time() - transferStartUs;
    releaseTransmitSlot(pipeline, slot);
    portENTER_CRITICAL(&pipeline.statsMux);
    pipeline.stats.dmaTotalUs += dmaDurationUs;
    pipeline.stats.dmaMaximumUs =
        std::max(pipeline.stats.dmaMaximumUs, dmaDurationUs);
    ++pipeline.stats.presented;
    ++pipeline.stats.dmaCompleted;
    portEXIT_CRITICAL(&pipeline.statsMux);
    maybeLogTiming(pipeline, esp_timer_get_time());
  }
}

bool snapshotCompleteFrame(DisplayPipeline& pipeline) {
  int selected = -1;
  bool overwroteReady = false;
  portENTER_CRITICAL(&pipeline.slotMux);
  selected = selectSnapshotSlot(pipeline.slots);
  if (selected >= 0) {
    overwroteReady =
        pipeline.slots[selected].state == FrameSlotState::ready;
    pipeline.slots[selected].generation = ++pipeline.nextGeneration;
    pipeline.slots[selected].state = FrameSlotState::snapshot;
  }
  portEXIT_CRITICAL(&pipeline.slotMux);

  if (selected < 0) {
    portENTER_CRITICAL(&pipeline.statsMux);
    ++pipeline.stats.noSnapshotSlot;
    portEXIT_CRITICAL(&pipeline.statsMux);
    return false;
  }

  const int64_t snapshotStartUs = esp_timer_get_time();
  std::memcpy(pipeline.transmitBuffers[selected],
              pipeline.canvas,
              kFrameBytes);
  const int64_t snapshotDurationUs = esp_timer_get_time() - snapshotStartUs;

  portENTER_CRITICAL(&pipeline.slotMux);
  pipeline.slots[selected].state = FrameSlotState::ready;
  portEXIT_CRITICAL(&pipeline.slotMux);

  portENTER_CRITICAL(&pipeline.statsMux);
  pipeline.stats.snapshotTotalUs += snapshotDurationUs;
  pipeline.stats.snapshotMaximumUs =
      std::max(pipeline.stats.snapshotMaximumUs, snapshotDurationUs);
  ++pipeline.stats.snapshots;
  if (overwroteReady) {
    ++pipeline.stats.overwrittenReady;
  }
  portEXIT_CRITICAL(&pipeline.statsMux);
  xSemaphoreGive(pipeline.frameReady);
  return true;
}

void rotateAreaIntoCanvas(DisplayPipeline& pipeline,
                          const lv_area_t& logicalArea,
                          const lv_area_t& rotatedArea,
                          const std::uint8_t* pixels) {
  const int sourceWidth = lv_area_get_width(&logicalArea);
  const int sourceHeight = lv_area_get_height(&logicalArea);
  const uint32_t sourceStride = lv_draw_buf_width_to_stride(
      sourceWidth, LV_COLOR_FORMAT_RGB565);
  constexpr uint32_t destinationStride =
      kDisplayWidth * kRgb565BytesPerPixel;

  // Tiling keeps both the strided source reads and the rotated destination
  // writes cache-local. LVGL performs the actual 270-degree transform; byte
  // swapping each completed destination segment stores permanent panel-endian
  // RGB565 in the canonical framebuffer.
  for (int sourceY = 0; sourceY < sourceHeight;
       sourceY += kRotationTilePixels) {
    const int tileHeight =
        std::min(kRotationTilePixels, sourceHeight - sourceY);
    for (int sourceX = 0; sourceX < sourceWidth;
         sourceX += kRotationTilePixels) {
      const int tileWidth =
          std::min(kRotationTilePixels, sourceWidth - sourceX);
      const int destinationX =
          rotatedArea.x1 + sourceHeight - sourceY - tileHeight;
      const int destinationY = rotatedArea.y1 + sourceX;
      const auto* tileSource =
          pixels + static_cast<std::size_t>(sourceY) * sourceStride +
          static_cast<std::size_t>(sourceX) * kRgb565BytesPerPixel;
      auto* tileDestination =
          pipeline.canvas +
          (static_cast<std::size_t>(destinationY) * kDisplayWidth +
           destinationX) *
              kRgb565BytesPerPixel;

      lv_draw_sw_rotate(tileSource,
                        tileDestination,
                        tileWidth,
                        tileHeight,
                        sourceStride,
                        destinationStride,
                        LV_DISPLAY_ROTATION_270,
                        LV_COLOR_FORMAT_RGB565);
      for (int destinationRow = 0; destinationRow < tileWidth;
           ++destinationRow) {
        lv_draw_sw_rgb565_swap(
            tileDestination +
                static_cast<std::size_t>(destinationRow) *
                    destinationStride,
            tileHeight);
      }
    }
  }
}

void flushToNativeFrame(lv_display_t* display,
                        const lv_area_t* area,
                        std::uint8_t* pixels) {
  auto& pipeline = *static_cast<DisplayPipeline*>(
      lv_display_get_user_data(display));
  const bool lastFlush = lv_display_flush_is_last(display);
  lv_area_t rotatedArea = *area;
  lv_display_rotate_area(display, &rotatedArea);

  const int64_t rotateStartUs = esp_timer_get_time();
  rotateAreaIntoCanvas(pipeline, *area, rotatedArea, pixels);
  const int64_t rotateDurationUs = esp_timer_get_time() - rotateStartUs;
  const std::uint64_t pixelCount =
      static_cast<std::uint64_t>(lv_area_get_size(area));

  portENTER_CRITICAL(&pipeline.statsMux);
  pipeline.stats.rotateTotalUs += rotateDurationUs;
  pipeline.stats.rotateMaximumUs =
      std::max(pipeline.stats.rotateMaximumUs, rotateDurationUs);
  pipeline.stats.rotatedPixels += pixelCount;
  ++pipeline.stats.flushes;
  portEXIT_CRITICAL(&pipeline.statsMux);

  if (lastFlush) {
    snapshotCompleteFrame(pipeline);
    portENTER_CRITICAL(&pipeline.statsMux);
    ++pipeline.stats.lvglFrames;
    portEXIT_CRITICAL(&pipeline.statsMux);
  }

  lv_display_flush_ready(display);
  maybeLogTiming(pipeline, esp_timer_get_time());
}

bool initializePipelineResources(DisplayPipeline& pipeline) {
  pipeline.canvas = allocatePsram(kFrameBytes, true);
  pipeline.drawBuffers[0] = allocatePsram(kDrawBufferBytes, false);
  pipeline.drawBuffers[1] = allocatePsram(kDrawBufferBytes, false);
  for (std::size_t index = 0; index < kTransmitSlotCount; ++index) {
    pipeline.transmitBuffers[index] =
        allocatePsram(kFrameBytes, false, true);
  }
  if (pipeline.canvas == nullptr || pipeline.drawBuffers[0] == nullptr ||
      pipeline.drawBuffers[1] == nullptr ||
      pipeline.transmitBuffers[0] == nullptr ||
      pipeline.transmitBuffers[1] == nullptr) {
    ESP_LOGE(kTag, "Unable to allocate native-scan display buffers in PSRAM");
    return false;
  }
  for (const auto* transmitBuffer : pipeline.transmitBuffers) {
    if (!esp_ptr_external_ram(transmitBuffer) ||
        !esp_ptr_dma_ext_capable(transmitBuffer)) {
      ESP_LOGE(kTag,
               "A transmit snapshot is not direct-PSRAM-DMA capable");
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
      .max_transfer_sz = kFrameBytes,
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
  lv_display_set_color_format(runtime.display, LV_COLOR_FORMAT_RGB565);
  lv_display_set_user_data(runtime.display, &gPipeline);
  lv_display_set_buffers(runtime.display,
                         gPipeline.drawBuffers[0],
                         gPipeline.drawBuffers[1],
                         kDrawBufferBytes,
                         LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(runtime.display, flushToNativeFrame);
  lv_display_set_rotation(runtime.display, LV_DISPLAY_ROTATION_270);

  bsp_display_cfg_t touchBspConfig{};
  // LVGL applies the same (y, width - 1 - x) transform that the old
  // swap_xy+mirror_y hardware mapping supplied for MADCTL 0xA0.
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
           "CO5300 synchronization: native scan + LVGL 270 software "
           "rotation + immutable double snapshots + GPIO43 TE");
  ESP_LOGI(kTag,
           "CO5300 buffers: canvas=%u bytes snapshots=2x%u draw=2x%u",
           static_cast<unsigned>(kFrameBytes),
           static_cast<unsigned>(kFrameBytes),
           static_cast<unsigned>(kDrawBufferBytes));

  if (bsp_display_brightness_init() != ESP_OK ||
      esp_lv_adapter_start() != ESP_OK) {
    ESP_LOGE(kTag, "Display runtime start failed");
    runtime = {};
  }
  return runtime;
}

}  // namespace oilgauge
