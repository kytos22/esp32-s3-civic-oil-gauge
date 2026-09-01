#pragma once

#include <cstdint>

#include "civic_aux_receiver.h"
#include "gauge_settings.h"

namespace oilgauge {

inline constexpr std::uint64_t kAutomaticBrightnessFallbackDurationMs = 1'500;
// Reject one-percent target chatter caused by ambient-sensor quantization.
inline constexpr std::uint8_t kAutomaticBrightnessTargetDeadbandPercent = 2;
// AUTO output brightens in about 2 s across the default 20–100% range.
inline constexpr double kAutomaticBrightnessRisePercentPerSecond = 40.0;
// AUTO output dims more gently in about 3.2 s across the same range.
inline constexpr double kAutomaticBrightnessFallPercentPerSecond = 25.0;

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
             std::uint8_t automaticMinimumPercent,
             std::uint8_t automaticMaximumPercent,
             std::uint64_t localNowMs,
             std::int8_t automaticBiasPercent = 0);
  void setPreferences(BrightnessMode mode,
                      std::uint8_t manualBackupPercent,
                      std::uint8_t automaticMinimumPercent,
                      std::uint8_t automaticMaximumPercent,
                      std::uint64_t localNowMs,
                      std::int8_t automaticBiasPercent = 0);
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
  void updateAutomaticRamp(std::uint64_t localNowMs);
  void synchronizeAutomaticRamp(std::uint64_t localNowMs);
  [[nodiscard]] bool recoveryReady(const CivicAuxSnapshot& snapshot) const;
  [[nodiscard]] std::uint8_t constrainedAutomaticPercent(
      std::uint32_t millilux) const;

  BrightnessMode mode_ = BrightnessMode::automatic;
  std::uint8_t manualBackupPercent_ = 55;
  std::uint8_t automaticMinimumPercent_ = 20;
  std::uint8_t automaticMaximumPercent_ = 100;
  std::int8_t automaticBiasPercent_ = 0;
  std::uint8_t fallbackStartPercent_ = 55;
  std::uint64_t fallbackStartedAtMs_ = 0;
  std::uint64_t automaticRampUpdatedAtMs_ = 0;
  double automaticAppliedPercent_ = 55.0;
  std::uint32_t recoveryBaseGeneration_ = 0;
  std::uint32_t lastObservedUsableGeneration_ = 0;
  std::uint32_t lastObservedHubRestarts_ = 0;
  bool automaticCurveSettingsChanged_ = false;
  bool initialized_ = false;
  AutomaticBrightnessStatus status_{};
};

[[nodiscard]] std::uint8_t automaticBrightnessPercentForMillilux(
    std::uint32_t millilux);

}  // namespace oilgauge
