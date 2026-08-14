#include "warning_audio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bsp/esp-bsp.h"
#include "esp_codec_dev.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace oilgauge {

namespace {

constexpr char kTag[] = "warning_audio";
constexpr std::uint32_t kSampleRate = 22'050;
constexpr std::uint32_t kBeepDurationMs = 120;
constexpr std::uint32_t kGapDurationMs = 90;
constexpr std::uint32_t kEdgeSilenceMs = 40;
constexpr std::uint32_t kFadeSamples = 110;
constexpr std::size_t kChunkSamples = 512;
constexpr UBaseType_t kAudioTaskPriority = 4;
constexpr BaseType_t kAudioTaskCore = 1;
constexpr std::array<std::int16_t, 10> kWave{
    0, 4114, 6657, 6657, 4114, 0, -4114, -6657, -6657, -4114};

esp_codec_dev_handle_t gSpeaker = nullptr;
TaskHandle_t gAudioTask = nullptr;
std::atomic_bool gAudioEnabled{true};

bool writeSegment(bool audible, std::uint32_t durationMs) {
  const std::uint32_t totalSamples =
      (kSampleRate * durationMs) / 1000U;
  std::array<std::int16_t, kChunkSamples> samples{};
  std::uint32_t writtenSamples = 0;

  while (writtenSamples < totalSamples) {
    const std::size_t chunkSamples = std::min<std::size_t>(
        samples.size(), totalSamples - writtenSamples);
    for (std::size_t index = 0; index < chunkSamples; ++index) {
      if (!audible) {
        samples[index] = 0;
        continue;
      }

      const std::uint32_t sampleIndex =
          writtenSamples + static_cast<std::uint32_t>(index);
      const std::uint32_t remainingSamples = totalSamples - sampleIndex;
      const std::uint32_t envelope = std::min(
          kFadeSamples, std::min(sampleIndex + 1U, remainingSamples));
      samples[index] = static_cast<std::int16_t>(
          (static_cast<std::int32_t>(kWave[sampleIndex % kWave.size()]) *
           static_cast<std::int32_t>(envelope)) /
          static_cast<std::int32_t>(kFadeSamples));
    }

    const int result = esp_codec_dev_write(
        gSpeaker,
        samples.data(),
        static_cast<int>(chunkSamples * sizeof(samples.front())));
    if (result != ESP_CODEC_DEV_OK) {
      ESP_LOGE(kTag, "Speaker write failed: %d", result);
      return false;
    }
    writtenSamples += static_cast<std::uint32_t>(chunkSamples);
  }
  return true;
}

void warningAudioTask(void*) {
  while (true) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    const bool played =
        writeSegment(false, kEdgeSilenceMs) &&
        writeSegment(true, kBeepDurationMs) &&
        writeSegment(false, kGapDurationMs) &&
        writeSegment(true, kBeepDurationMs) &&
        writeSegment(false, kEdgeSilenceMs);
    if (!played) {
      ESP_LOGE(kTag, "Warning tone did not complete cleanly");
    } else {
      ESP_LOGI(kTag, "Warning tone completed");
    }
  }
}

}  // namespace

bool initWarningAudio() {
  if (gAudioTask != nullptr) {
    return true;
  }

  gSpeaker = bsp_audio_codec_speaker_init();
  if (gSpeaker == nullptr) {
    ESP_LOGW(kTag, "Integrated ES8311 speaker is unavailable");
    return false;
  }

  esp_codec_dev_sample_info_t sampleInfo{
      .bits_per_sample = 16,
      .channel = 1,
      .channel_mask = 0,
      .sample_rate = kSampleRate,
      .mclk_multiple = 0,
  };
  const int openResult = esp_codec_dev_open(gSpeaker, &sampleInfo);
  if (openResult != ESP_CODEC_DEV_OK) {
    ESP_LOGW(kTag, "Unable to open ES8311 speaker: %d", openResult);
    return false;
  }
  if (esp_codec_dev_set_out_vol(
          gSpeaker, CONFIG_OIL_GAUGE_WARNING_AUDIO_VOLUME_PERCENT) !=
      ESP_CODEC_DEV_OK) {
    ESP_LOGW(kTag, "Unable to set warning-speaker volume");
    esp_codec_dev_close(gSpeaker);
    return false;
  }
  if (esp_codec_dev_set_out_mute(gSpeaker, true) != ESP_CODEC_DEV_OK) {
    ESP_LOGW(kTag, "Unable to mute warning speaker at startup");
    esp_codec_dev_close(gSpeaker);
    return false;
  }
  if (!writeSegment(false, kEdgeSilenceMs) ||
      esp_codec_dev_set_out_mute(gSpeaker, false) != ESP_CODEC_DEV_OK ||
      !writeSegment(false, kEdgeSilenceMs)) {
    ESP_LOGW(kTag, "Unable to settle warning-speaker output");
    esp_codec_dev_close(gSpeaker);
    return false;
  }

  const BaseType_t taskResult = xTaskCreatePinnedToCore(
      warningAudioTask,
      "warning_audio",
      4096,
      nullptr,
      kAudioTaskPriority,
      &gAudioTask,
      kAudioTaskCore);
  if (taskResult != pdPASS) {
    ESP_LOGW(kTag, "Unable to create warning-audio worker");
    esp_codec_dev_close(gSpeaker);
    gAudioTask = nullptr;
    return false;
  }

  ESP_LOGI(kTag,
           "Integrated warning speaker ready at %d%% volume",
           CONFIG_OIL_GAUGE_WARNING_AUDIO_VOLUME_PERCENT);
  return true;
}

void requestWarningTone() {
  if (gAudioTask != nullptr && gAudioEnabled.load()) {
    xTaskNotifyGive(gAudioTask);
  }
}

bool warningAudioAvailable() {
  return gAudioTask != nullptr;
}

void setWarningAudioEnabled(bool enabled) {
  gAudioEnabled.store(enabled);
}

bool setWarningAudioVolume(int percent) {
  if (gSpeaker == nullptr) {
    return false;
  }
  const int bounded = std::clamp(percent, 5, 100);
  if (esp_codec_dev_set_out_vol(gSpeaker, bounded) != ESP_CODEC_DEV_OK) {
    ESP_LOGW(kTag, "Unable to set warning-speaker volume to %d%%", bounded);
    return false;
  }
  return true;
}

}  // namespace oilgauge
