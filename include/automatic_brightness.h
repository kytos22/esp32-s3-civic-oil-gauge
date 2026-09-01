#pragma once

#include <cstdint>

#include "civic_aux_receiver.h"
#include "gauge_settings.h"

namespace oilgauge {

inline constexpr std::uint64_t kAutomaticBrightnessFallbackDurationMs = 1'500;

enum class AutomaticBrightnessState : std::uint8_t {
  manual,
  waitingForSamples,
  automatic,
  fallback,
};

struct AutomaticBrightnessStatus {
  AutomaticBrightnessState state = AutomaticBrightnessState::waitingForSamples;
  std::uint8_t appliedPercent = 55;
  std::uint8_t automaticPercent = 0;
  bool automaticPercentAvailable = false;
  bool luxFresh = false;
  bool persistenceRequested = false;
};

class AutomaticBrightnessController {
 public:
  void reset(BrightnessMode mode,
             std::uint8_t manualBackupPercent,
             std::uint64_t localNowMs);
  void setPreferences(BrightnessMode mode,
                      std::uint8_t manualBackupPercent,
                      std::uint64_t localNowMs);
  [[nodiscard]] AutomaticBrightnessStatus update(
      const CivicAuxSnapshot& snapshot,
      std::uint64_t localNowMs);
  [[nodiscard]] const AutomaticBrightnessStatus& status() const {
    return status_;
  }

 private:
  void beginFallback(std::uint64_t localNowMs,
                     std::uint32_t usableGeneration);
  void updateFallback(std::uint64_t localNowMs);
  [[nodiscard]] bool recoveryReady(const CivicAuxSnapshot& snapshot) const;

  BrightnessMode mode_ = BrightnessMode::automatic;
  std::uint8_t manualBackupPercent_ = 55;
  std::uint8_t fallbackStartPercent_ = 55;
  std::uint64_t fallbackStartedAtMs_ = 0;
  std::uint32_t recoveryBaseGeneration_ = 0;
  std::uint32_t lastObservedUsableGeneration_ = 0;
  std::uint32_t lastObservedHubRestarts_ = 0;
  bool initialized_ = false;
  AutomaticBrightnessStatus status_{};
};

[[nodiscard]] std::uint8_t automaticBrightnessPercentForMillilux(
    std::uint32_t millilux);

}  // namespace oilgauge
