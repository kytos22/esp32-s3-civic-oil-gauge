#include "settings_store.h"

#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

#include <cstdint>

namespace oilgauge {

namespace {

constexpr char kTag[] = "settings_store";
constexpr char kNamespace[] = "oil_gauge";

bool readU8(nvs_handle_t handle, const char* key, std::uint8_t& value) {
  const esp_err_t result = nvs_get_u8(handle, key, &value);
  return result == ESP_OK;
}

}  // namespace

bool initSettingsStore() {
  const esp_err_t result = nvs_flash_init();
  if (result != ESP_OK) {
    ESP_LOGW(kTag,
             "NVS unavailable; using safe defaults without erasing storage: %s",
             esp_err_to_name(result));
    return false;
  }
  return true;
}

GaugeSettings loadGaugeSettings(const GaugeSettings& defaults) {
  GaugeSettings settings = sanitizeGaugeSettings(defaults);
  nvs_handle_t handle = 0;
  if (nvs_open(kNamespace, NVS_READONLY, &handle) != ESP_OK) {
    return settings;
  }

  std::uint8_t value = 0;
  if (readU8(handle, "brightness", value)) {
    settings.brightnessPercent = value;
  }
  if (readU8(handle, "sound", value)) {
    settings.warningSoundEnabled = value != 0;
  }
  if (readU8(handle, "volume", value)) {
    settings.warningVolumePercent = value;
  }
  if (readU8(handle, "unit", value)) {
    settings.pressureUnit = static_cast<PressureUnit>(value);
  }
  if (readU8(handle, "temp_unit", value)) {
    settings.temperatureUnit = static_cast<TemperatureUnit>(value);
  }
  if (readU8(handle, "warning", value)) {
    settings.warningVisualMode = static_cast<WarningVisualMode>(value);
  }
  if (readU8(handle, "source", value)) {
    settings.dataSource = static_cast<DataSource>(value);
  }
  nvs_close(handle);
  return sanitizeGaugeSettings(settings);
}

bool saveGaugeSettings(const GaugeSettings& rawSettings) {
  const GaugeSettings settings = sanitizeGaugeSettings(rawSettings);
  nvs_handle_t handle = 0;
  esp_err_t result = nvs_open(kNamespace, NVS_READWRITE, &handle);
  if (result != ESP_OK) {
    ESP_LOGW(kTag, "Unable to open settings namespace: %s", esp_err_to_name(result));
    return false;
  }

  result = nvs_set_u8(handle, "brightness", settings.brightnessPercent);
  if (result == ESP_OK) {
    result = nvs_set_u8(handle, "sound", settings.warningSoundEnabled ? 1 : 0);
  }
  if (result == ESP_OK) {
    result = nvs_set_u8(handle, "volume", settings.warningVolumePercent);
  }
  if (result == ESP_OK) {
    result = nvs_set_u8(handle, "unit", static_cast<std::uint8_t>(settings.pressureUnit));
  }
  if (result == ESP_OK) {
    result = nvs_set_u8(
        handle, "temp_unit", static_cast<std::uint8_t>(settings.temperatureUnit));
  }
  if (result == ESP_OK) {
    result = nvs_set_u8(
        handle, "warning", static_cast<std::uint8_t>(settings.warningVisualMode));
  }
  if (result == ESP_OK) {
    result = nvs_set_u8(
        handle, "source", static_cast<std::uint8_t>(settings.dataSource));
  }
  if (result == ESP_OK) {
    result = nvs_commit(handle);
  }
  nvs_close(handle);

  if (result != ESP_OK) {
    ESP_LOGW(kTag, "Unable to persist settings: %s", esp_err_to_name(result));
    return false;
  }
  return true;
}

}  // namespace oilgauge
