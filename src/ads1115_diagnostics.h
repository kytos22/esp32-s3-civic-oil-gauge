#pragma once

#include <array>
#include <cstdint>

#include "driver/i2c_master.h"

namespace oilgauge {

struct Ads1115Sample {
  std::array<std::int16_t, 4> raw{};
  std::uint64_t timestampUs = 0;
  std::uint32_t sequence = 0;
  bool valid = false;
};

[[nodiscard]] bool startAds1115Diagnostics(i2c_master_bus_handle_t bus);
[[nodiscard]] bool latestAds1115Sample(Ads1115Sample& sample);

}  // namespace oilgauge
