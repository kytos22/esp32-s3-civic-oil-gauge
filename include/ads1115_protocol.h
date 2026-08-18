#pragma once

#include <cstdint>

namespace oilgauge {

[[nodiscard]] std::uint16_t ads1115SingleShotConfig(std::uint8_t channel);
[[nodiscard]] double ads1115RawToVolts(std::int16_t raw);

}  // namespace oilgauge
