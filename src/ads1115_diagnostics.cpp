#include "ads1115_diagnostics.h"

#include <array>
#include <cstdint>

#include "ads1115_protocol.h"
#include "board_pins.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace oilgauge {
namespace {

constexpr char kTag[] = "ads1115_diag";
constexpr std::uint8_t kConversionRegister = 0x00;
constexpr std::uint8_t kConfigRegister = 0x01;
constexpr int kI2cTimeoutMs = 50;
constexpr TickType_t kConversionDelay = pdMS_TO_TICKS(9);
constexpr TickType_t kSamplePeriod = pdMS_TO_TICKS(1000);
i2c_master_dev_handle_t gAds1115 = nullptr;

bool readChannel(std::uint8_t channel, std::int16_t& raw) {
  const std::uint16_t config = ads1115SingleShotConfig(channel);
  const std::array<std::uint8_t, 3> command{
      kConfigRegister,
      static_cast<std::uint8_t>(config >> 8U),
      static_cast<std::uint8_t>(config & 0xFFU)};
  esp_err_t result = i2c_master_transmit(
      gAds1115, command.data(), command.size(), kI2cTimeoutMs);
  if (result != ESP_OK) {
    ESP_LOGW(kTag, "A%u conversion start failed: %s", channel,
             esp_err_to_name(result));
    return false;
  }

  vTaskDelay(kConversionDelay);
  std::array<std::uint8_t, 2> bytes{};
  result = i2c_master_transmit_receive(
      gAds1115,
      &kConversionRegister,
      1,
      bytes.data(),
      bytes.size(),
      kI2cTimeoutMs);
  if (result != ESP_OK) {
    ESP_LOGW(kTag, "A%u conversion read failed: %s", channel,
             esp_err_to_name(result));
    return false;
  }

  raw = static_cast<std::int16_t>(
      (static_cast<std::uint16_t>(bytes[0]) << 8U) | bytes[1]);
  return true;
}

void diagnosticTask(void*) {
  while (true) {
    std::array<std::int16_t, 4> raw{};
    bool complete = true;
    for (std::uint8_t channel = 0; channel < raw.size(); ++channel) {
      complete = readChannel(channel, raw[channel]) && complete;
    }
    if (complete) {
      ESP_LOGI(kTag,
               "raw-only floating inputs: A0=%d %.6fV A1=%d %.6fV "
               "A2=%d %.6fV A3=%d %.6fV",
               raw[0], ads1115RawToVolts(raw[0]),
               raw[1], ads1115RawToVolts(raw[1]),
               raw[2], ads1115RawToVolts(raw[2]),
               raw[3], ads1115RawToVolts(raw[3]));
    }
    vTaskDelay(kSamplePeriod);
  }
}

}  // namespace

bool startAds1115Diagnostics(i2c_master_bus_handle_t bus) {
  if (bus == nullptr) {
    ESP_LOGE(kTag, "Shared display I2C bus is unavailable");
    return false;
  }

  const esp_err_t probe =
      i2c_master_probe(bus, board::kAds1115Address, kI2cTimeoutMs);
  if (probe != ESP_OK) {
    ESP_LOGW(kTag, "ADS1115 not detected at 0x%02X: %s",
             board::kAds1115Address, esp_err_to_name(probe));
    return false;
  }
  ESP_LOGI(kTag, "I2C probe found ADS1115 candidate at 0x%02X",
           board::kAds1115Address);

  i2c_device_config_t config{};
  config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  config.device_address = board::kAds1115Address;
  config.scl_speed_hz = 100000;
  const esp_err_t addResult =
      i2c_master_bus_add_device(bus, &config, &gAds1115);
  if (addResult != ESP_OK) {
    ESP_LOGE(kTag, "Unable to register ADS1115: %s",
             esp_err_to_name(addResult));
    return false;
  }

  if (xTaskCreate(diagnosticTask,
                  "ads1115_diag",
                  4096,
                  nullptr,
                  2,
                  nullptr) != pdPASS) {
    ESP_LOGE(kTag, "Unable to start ADS1115 diagnostic task");
    i2c_master_bus_rm_device(gAds1115);
    gAds1115 = nullptr;
    return false;
  }
  return true;
}

}  // namespace oilgauge
