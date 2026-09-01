#include "automatic_brightness.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

namespace oilgauge {

namespace {

struct BrightnessPoint {
  std::uint32_t millilux;
  std::uint8_t percent;
};

constexpr std::array<BrightnessPoint, 9> kBrightnessCurve{{
    {0U, 5U},
    {1'000U, 7U},
    {5'000U, 12U},
    {20'000U, 20U},
    {100'000U, 35U},
    {500'000U, 50U},
    {2'000'000U, 65U},
    {10'000'000U, 85U},
    {30'000'000U, 100U},
}};

std::uint8_t clampBrightness(std::uint8_t percent) {
  return std::clamp<std::uint8_t>(percent, 5U, 100U);
}

void normalizeBrightnessRange(std::uint8_t& minimumPercent,
                              std::uint8_t& maximumPercent) {
  minimumPercent = clampBrightness(minimumPercent);
  maximumPercent = clampBrightness(maximumPercent);
  if (minimumPercent > maximumPercent) {
    std::swap(minimumPercent, maximumPercent);
  }
}

}  // namespace

std::uint8_t automaticBrightnessPercentForMillilux(
    std::uint32_t millilux) {
  if (millilux <= kBrightnessCurve.front().millilux) {
    return kBrightnessCurve.front().percent;
  }
  if (millilux >= kBrightnessCurve.back().millilux) {
    return kBrightnessCurve.back().percent;
  }

  for (std::size_t index = 1; index < kBrightnessCurve.size(); ++index) {
    const BrightnessPoint& upper = kBrightnessCurve[index];
    if (millilux > upper.millilux) {
      continue;
    }
    const BrightnessPoint& lower = kBrightnessCurve[index - 1U];
    const double lux = static_cast<double>(millilux) / 1000.0;
    const double lowerLux = static_cast<double>(lower.millilux) / 1000.0;
    const double upperLux = static_cast<double>(upper.millilux) / 1000.0;
    const double denominator = std::log1p(upperLux) - std::log1p(lowerLux);
    const double fraction =
        denominator > 0.0
            ? (std::log1p(lux) - std::log1p(lowerLux)) / denominator
            : 0.0;
    const double interpolated =
        static_cast<double>(lower.percent) +
        std::clamp(fraction, 0.0, 1.0) *
            static_cast<double>(upper.percent - lower.percent);
    return clampBrightness(
        static_cast<std::uint8_t>(std::lround(interpolated)));
  }
  return kBrightnessCurve.back().percent;
}

void AutomaticBrightnessController::reset(BrightnessMode mode,
                                          std::uint8_t manualBackupPercent,
                                          std::uint8_t automaticMinimumPercent,
                                          std::uint8_t automaticMaximumPercent,
                                          std::uint64_t localNowMs) {
  mode_ = mode;
  manualBackupPercent_ = clampBrightness(manualBackupPercent);
  normalizeBrightnessRange(
      automaticMinimumPercent, automaticMaximumPercent);
  automaticMinimumPercent_ = automaticMinimumPercent;
  automaticMaximumPercent_ = automaticMaximumPercent;
  fallbackStartPercent_ = manualBackupPercent_;
  fallbackStartedAtMs_ = localNowMs;
  automaticRampUpdatedAtMs_ = localNowMs;
  automaticAppliedPercent_ = manualBackupPercent_;
  recoveryBaseGeneration_ = 0;
  lastObservedUsableGeneration_ = 0;
  lastObservedHubRestarts_ = 0;
  automaticLimitsChanged_ = false;
  status_ = {};
  status_.state = mode_ == BrightnessMode::manual
                      ? AutomaticBrightnessState::manual
                      : AutomaticBrightnessState::waitingForSamples;
  status_.appliedPercent = manualBackupPercent_;
  initialized_ = true;
}

void AutomaticBrightnessController::setPreferences(
    BrightnessMode mode,
    std::uint8_t manualBackupPercent,
    std::uint8_t automaticMinimumPercent,
    std::uint8_t automaticMaximumPercent,
    std::uint64_t localNowMs) {
  if (!initialized_) {
    reset(mode,
          manualBackupPercent,
          automaticMinimumPercent,
          automaticMaximumPercent,
          localNowMs);
    return;
  }

  normalizeBrightnessRange(
      automaticMinimumPercent, automaticMaximumPercent);
  const bool automaticLimitsChanged =
      automaticMinimumPercent != automaticMinimumPercent_ ||
      automaticMaximumPercent != automaticMaximumPercent_;
  manualBackupPercent_ = clampBrightness(manualBackupPercent);
  automaticMinimumPercent_ = automaticMinimumPercent;
  automaticMaximumPercent_ = automaticMaximumPercent;
  if (automaticLimitsChanged) {
    automaticLimitsChanged_ = true;
    if (status_.automaticPercentAvailable) {
      status_.automaticPercent = std::clamp<std::uint8_t>(
          status_.automaticPercent,
          automaticMinimumPercent_,
          automaticMaximumPercent_);
    }
  }
  if (mode == mode_) {
    if (mode_ == BrightnessMode::manual) {
      status_.appliedPercent = manualBackupPercent_;
      synchronizeAutomaticRamp(localNowMs);
    }
    return;
  }

  mode_ = mode;
  status_.automaticPercentAvailable = false;
  status_.luxFresh = false;
  if (mode_ == BrightnessMode::manual) {
    status_.state = AutomaticBrightnessState::manual;
    status_.appliedPercent = manualBackupPercent_;
  } else {
    status_.state = AutomaticBrightnessState::waitingForSamples;
    status_.appliedPercent = manualBackupPercent_;
    recoveryBaseGeneration_ = lastObservedUsableGeneration_;
    fallbackStartedAtMs_ = localNowMs;
  }
  synchronizeAutomaticRamp(localNowMs);
}

void AutomaticBrightnessController::beginFallback(
    std::uint64_t localNowMs,
    std::uint32_t usableGeneration) {
  if (status_.state != AutomaticBrightnessState::fallback) {
    fallbackStartPercent_ = status_.appliedPercent;
    fallbackStartedAtMs_ = localNowMs;
  }
  recoveryBaseGeneration_ = usableGeneration;
  status_.state = AutomaticBrightnessState::fallback;
}

void AutomaticBrightnessController::updateFallback(
    std::uint64_t localNowMs) {
  const std::uint64_t elapsed = localNowMs >= fallbackStartedAtMs_
                                    ? localNowMs - fallbackStartedAtMs_
                                    : 0;
  if (elapsed >= kAutomaticBrightnessFallbackDurationMs) {
    status_.appliedPercent = manualBackupPercent_;
    return;
  }
  const double fraction =
      static_cast<double>(elapsed) /
      static_cast<double>(kAutomaticBrightnessFallbackDurationMs);
  const double value =
      static_cast<double>(fallbackStartPercent_) +
      (static_cast<double>(manualBackupPercent_) -
       static_cast<double>(fallbackStartPercent_)) *
          fraction;
  status_.appliedPercent = clampBrightness(
      static_cast<std::uint8_t>(std::lround(value)));
}

void AutomaticBrightnessController::synchronizeAutomaticRamp(
    std::uint64_t localNowMs) {
  automaticAppliedPercent_ = status_.appliedPercent;
  automaticRampUpdatedAtMs_ = localNowMs;
}

void AutomaticBrightnessController::updateAutomaticRamp(
    std::uint64_t localNowMs) {
  if (!status_.automaticPercentAvailable) {
    synchronizeAutomaticRamp(localNowMs);
    return;
  }

  const std::uint64_t elapsedMs =
      localNowMs >= automaticRampUpdatedAtMs_
          ? localNowMs - automaticRampUpdatedAtMs_
          : 0;
  automaticRampUpdatedAtMs_ = localNowMs;
  const double target = status_.automaticPercent;
  const double difference = target - automaticAppliedPercent_;
  const double rate = difference >= 0.0
                          ? kAutomaticBrightnessRisePercentPerSecond
                          : kAutomaticBrightnessFallPercentPerSecond;
  const double maximumStep =
      rate * static_cast<double>(elapsedMs) / 1000.0;
  if (std::abs(difference) <= maximumStep) {
    automaticAppliedPercent_ = target;
  } else if (difference > 0.0) {
    automaticAppliedPercent_ += maximumStep;
  } else {
    automaticAppliedPercent_ -= maximumStep;
  }
  status_.appliedPercent = clampBrightness(
      static_cast<std::uint8_t>(std::lround(automaticAppliedPercent_)));
}

bool AutomaticBrightnessController::recoveryReady(
    const CivicAuxSnapshot& snapshot) const {
  const std::uint32_t samplesSinceRecoveryStarted =
      snapshot.usableGeneration - recoveryBaseGeneration_;
  return snapshot.consecutiveUsableAmbientFrames >= 2U &&
         samplesSinceRecoveryStarted >= 2U;
}

std::uint8_t AutomaticBrightnessController::constrainedAutomaticPercent(
    std::uint32_t millilux) const {
  return std::clamp<std::uint8_t>(
      automaticBrightnessPercentForMillilux(millilux),
      automaticMinimumPercent_,
      automaticMaximumPercent_);
}

AutomaticBrightnessStatus AutomaticBrightnessController::update(
    const CivicAuxSnapshot& snapshot,
    std::uint64_t localNowMs) {
  if (!initialized_) {
    reset(mode_,
          manualBackupPercent_,
          automaticMinimumPercent_,
          automaticMaximumPercent_,
          localNowMs);
  }

  if (mode_ == BrightnessMode::manual) {
    lastObservedUsableGeneration_ = snapshot.usableGeneration;
    lastObservedHubRestarts_ = snapshot.diagnostics.hubRestarts;
    status_.state = AutomaticBrightnessState::manual;
    status_.appliedPercent = manualBackupPercent_;
    status_.automaticPercentAvailable = false;
    status_.luxFresh = false;
    status_.persistenceRequested = false;
    synchronizeAutomaticRamp(localNowMs);
    return status_;
  }

  const bool fresh = snapshot.hasUsableLux &&
                     localNowMs < snapshot.lastUsableUntilMs;
  status_.luxFresh = fresh;
  const bool newUsableGeneration =
      snapshot.usableGeneration != lastObservedUsableGeneration_;
  if (newUsableGeneration) {
    lastObservedUsableGeneration_ = snapshot.usableGeneration;
  }
  if (fresh &&
      (newUsableGeneration ||
       (automaticLimitsChanged_ && status_.automaticPercentAvailable))) {
    const std::uint8_t candidatePercent = constrainedAutomaticPercent(
        snapshot.lastUsableMillilux);
    const int targetDifference =
        std::abs(static_cast<int>(candidatePercent) -
                 static_cast<int>(status_.automaticPercent));
    if (!status_.automaticPercentAvailable || automaticLimitsChanged_ ||
        targetDifference >= kAutomaticBrightnessTargetDeadbandPercent) {
      status_.automaticPercent = candidatePercent;
    }
    status_.automaticPercentAvailable = true;
    automaticLimitsChanged_ = false;
  }

  const bool hubRestarted =
      snapshot.diagnostics.hubRestarts != lastObservedHubRestarts_;
  lastObservedHubRestarts_ = snapshot.diagnostics.hubRestarts;
  if (hubRestarted &&
      status_.state == AutomaticBrightnessState::automatic) {
    beginFallback(
        localNowMs,
        snapshot.usableGeneration -
            snapshot.consecutiveUsableAmbientFrames);
  }

  if ((snapshot.invalidTrafficFallback || !fresh) &&
      status_.state == AutomaticBrightnessState::automatic) {
    beginFallback(localNowMs, snapshot.usableGeneration);
  }

  if (fresh && recoveryReady(snapshot)) {
    status_.state = AutomaticBrightnessState::automatic;
    updateAutomaticRamp(localNowMs);
  } else if (status_.state == AutomaticBrightnessState::fallback) {
    updateFallback(localNowMs);
    synchronizeAutomaticRamp(localNowMs);
  } else if (status_.state == AutomaticBrightnessState::waitingForSamples) {
    status_.appliedPercent = manualBackupPercent_;
    synchronizeAutomaticRamp(localNowMs);
  }

  status_.persistenceRequested = false;
  return status_;
}

}  // namespace oilgauge
