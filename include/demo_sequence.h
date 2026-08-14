#pragma once

#include <cstdint>

namespace oilgauge {

struct DemoFrame {
  double pressurePsi = 0.0;
  double temperatureC = 0.0;
  std::uint32_t rpm = 0;
};

[[nodiscard]] DemoFrame demoFrameAt(std::uint64_t nowUs);

/** Return the on/off phase for the binary 2 Hz pressure-warning blink. */
[[nodiscard]] bool warningBlinkPhaseOn(std::uint64_t nowUs);

}  // namespace oilgauge
