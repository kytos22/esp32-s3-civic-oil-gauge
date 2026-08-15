#pragma once

#include <cstdint>

namespace oilgauge {

// Application state is produced slightly faster than the measured 59.46 Hz
// panel cadence. The TE input, not this timer, decides when a frame is shown.
inline constexpr std::uint32_t kUiFramePeriodMs = 15;
inline constexpr std::uint32_t kDisplayTargetFps = 60;

}  // namespace oilgauge
