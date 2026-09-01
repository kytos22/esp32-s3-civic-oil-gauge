#include "civic_aux_uart.h"

#include "driver/uart.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <array>
#include <cstdint>
#include <type_traits>

#include "board_pins.h"

namespace oilgauge {

namespace {

constexpr char kTag[] = "civic_aux_uart";
constexpr uart_port_t kUartPort = UART_NUM_1;
constexpr int kBaudRate = 115200;
constexpr int kDriverRxBufferBytes = 512;
constexpr std::size_t kReadChunkBytes = 64;
constexpr TickType_t kReadWait = pdMS_TO_TICKS(20);
constexpr std::uint32_t kTaskStackBytes = 4096;
constexpr UBaseType_t kTaskPriority = 5;
constexpr BaseType_t kTaskCore = 0;
constexpr std::uint64_t kDiagnosticLogPeriodMs = 5'000;

static_assert(std::is_trivially_copyable_v<CivicAuxSnapshot>,
              "The UART snapshot must remain atomically copyable");

portMUX_TYPE gSnapshotMux = portMUX_INITIALIZER_UNLOCKED;
CivicAuxReceiver gReceiver;
CivicAuxSnapshot gPublishedSnapshot;
TaskHandle_t gReceiverTask = nullptr;

std::uint64_t localNowMs() {
  return static_cast<std::uint64_t>(esp_timer_get_time()) / 1000U;
}

void publishSnapshot() {
  const CivicAuxSnapshot snapshot = gReceiver.snapshot();
  portENTER_CRITICAL(&gSnapshotMux);
  gPublishedSnapshot = snapshot;
  portEXIT_CRITICAL(&gSnapshotMux);
}

void receiverTask(void*) {
  std::array<std::uint8_t, kReadChunkBytes> bytes{};
  std::uint64_t lastLogMs = 0;
  while (true) {
    const int received = uart_read_bytes(
        kUartPort, bytes.data(), bytes.size(), kReadWait);
    const std::uint64_t nowMs = localNowMs();
    if (received < 0) {
      gReceiver.noteUartReadError(nowMs);
      publishSnapshot();
      continue;
    }
    if (received == 0) {
      continue;
    }

    for (int index = 0; index < received; ++index) {
      (void)gReceiver.feed(bytes[static_cast<std::size_t>(index)], nowMs);
    }
    publishSnapshot();

    if (lastLogMs == 0 || nowMs - lastLogMs >= kDiagnosticLogPeriodMs) {
      lastLogMs = nowMs;
      const CivicAuxSnapshot& snapshot = gReceiver.snapshot();
      ESP_LOGI(kTag,
               "RX frames=%lu ambient=%lu usable=%lu crc=%lu length=%lu "
               "version=%lu target=%lu restarts=%lu lux=%lu.%03lu",
               static_cast<unsigned long>(
                   snapshot.diagnostics.acceptedFrames),
               static_cast<unsigned long>(snapshot.diagnostics.ambientFrames),
               static_cast<unsigned long>(
                   snapshot.diagnostics.usableAmbientFrames),
               static_cast<unsigned long>(
                   snapshot.diagnostics.parser.crcErrors),
               static_cast<unsigned long>(
                   snapshot.diagnostics.parser.lengthErrors +
                   snapshot.diagnostics.knownLengthErrors),
               static_cast<unsigned long>(
                   snapshot.diagnostics.parser.versionErrors),
               static_cast<unsigned long>(snapshot.diagnostics.targetMisses),
               static_cast<unsigned long>(snapshot.diagnostics.hubRestarts),
               static_cast<unsigned long>(
                   snapshot.lastUsableMillilux / 1000U),
               static_cast<unsigned long>(
                   snapshot.lastUsableMillilux % 1000U));
    }
  }
}

}  // namespace

bool startCivicAuxUartReceiver() {
  if (gReceiverTask != nullptr) {
    return true;
  }
  if (uart_is_driver_installed(kUartPort)) {
    ESP_LOGE(kTag,
             "UART1 is already occupied; CivicAux receiver will not start");
    return false;
  }

  uart_config_t config{};
  config.baud_rate = kBaudRate;
  config.data_bits = UART_DATA_8_BITS;
  config.parity = UART_PARITY_DISABLE;
  config.stop_bits = UART_STOP_BITS_1;
  config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
  config.rx_flow_ctrl_thresh = 0;
  config.source_clk = UART_SCLK_DEFAULT;

  esp_err_t result = uart_param_config(kUartPort, &config);
  if (result == ESP_OK) {
    result = uart_set_pin(kUartPort,
                          UART_PIN_NO_CHANGE,
                          board::kUartRx,
                          UART_PIN_NO_CHANGE,
                          UART_PIN_NO_CHANGE);
  }
  if (result == ESP_OK) {
    result = uart_driver_install(
        kUartPort, kDriverRxBufferBytes, 0, 0, nullptr, 0);
  }
  if (result != ESP_OK) {
    ESP_LOGE(kTag,
             "Unable to initialize UART1 RX-only on GPIO%d: %s",
             board::kUartRx,
             esp_err_to_name(result));
    return false;
  }

  gReceiver.reset();
  gReceiver.setRunning(true);
  publishSnapshot();
  const BaseType_t taskResult = xTaskCreatePinnedToCore(
      receiverTask,
      "civic_aux_rx",
      kTaskStackBytes,
      nullptr,
      kTaskPriority,
      &gReceiverTask,
      kTaskCore);
  if (taskResult != pdPASS) {
    gReceiverTask = nullptr;
    gReceiver.setRunning(false);
    publishSnapshot();
    (void)uart_driver_delete(kUartPort);
    ESP_LOGE(kTag, "Unable to create the CivicAux UART receiver task");
    return false;
  }

  ESP_LOGI(kTag,
           "UART1 RX-only ready: GPIO%d, 115200 8N1, TX disabled",
           board::kUartRx);
  return true;
}

bool latestCivicAuxSnapshot(CivicAuxSnapshot& snapshot) {
  portENTER_CRITICAL(&gSnapshotMux);
  snapshot = gPublishedSnapshot;
  portEXIT_CRITICAL(&gSnapshotMux);
  return snapshot.receiverRunning;
}

}  // namespace oilgauge
