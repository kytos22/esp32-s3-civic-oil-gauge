#include "demo_sequence.h"

#include <array>
#include <cmath>
#include <cstddef>

namespace oilgauge {

namespace {

constexpr std::uint64_t kDemoSegmentUs = 4'000'000;
constexpr std::uint64_t kWarningBlinkHalfPeriodUs = 250'000U;
constexpr std::uint64_t kFullScreenWarningHalfPeriodUs = 1'000'000U;

constexpr std::array<DemoFrame, 7> kDemoScenes{{
    {0.0, 49.0, 0},
    {7.0, 58.0, 1800},
    {13.0, 72.0, 1800},
    {61.0, 82.0, 2500},
    {70.0, 92.0, 3200},
    {95.0, 97.0, 3500},
    {55.0, 110.0, 2800},
}};

double interpolate(double start, double end, double fraction) {
  return start + (end - start) * fraction;
}

}  // namespace

DemoFrame demoFrameAt(std::uint64_t nowUs) {
  const std::uint64_t sequenceUs =
      kDemoSegmentUs * static_cast<std::uint64_t>(kDemoScenes.size());
  const std::uint64_t wrappedUs = nowUs % sequenceUs;
  const std::size_t sceneIndex =
      static_cast<std::size_t>(wrappedUs / kDemoSegmentUs);
  const std::size_t nextIndex = (sceneIndex + 1U) % kDemoScenes.size();
  const double linearFraction =
      static_cast<double>(wrappedUs % kDemoSegmentUs) /
      static_cast<double>(kDemoSegmentUs);
  const DemoFrame& start = kDemoScenes[sceneIndex];
  const DemoFrame& end = kDemoScenes[nextIndex];

  return {
      interpolate(start.pressurePsi, end.pressurePsi, linearFraction),
      interpolate(start.temperatureC, end.temperatureC, linearFraction),
      static_cast<std::uint32_t>(std::lround(
          interpolate(static_cast<double>(start.rpm),
                      static_cast<double>(end.rpm),
                      linearFraction))),
  };
}

bool warningBlinkPhaseOn(std::uint64_t nowUs) {
  return ((nowUs / kWarningBlinkHalfPeriodUs) % 2U) == 0U;
}

bool fullScreenWarningPhaseOn(std::uint64_t nowUs) {
  return ((nowUs / kFullScreenWarningHalfPeriodUs) % 2U) == 0U;
}

}  // namespace oilgauge
