#include "ads1115_protocol.h"

namespace oilgauge {

std::uint16_t ads1115SingleShotConfig(std::uint8_t channel) {
  const std::uint16_t mux =
      static_cast<std::uint16_t>(0x04U + (channel <= 3U ? channel : 0U));
  return static_cast<std::uint16_t>(
      0x8000U | (mux << 12U) | 0x0200U | 0x0100U | 0x0080U | 0x0003U);
}

double ads1115RawToVolts(std::int16_t raw) {
  return static_cast<double>(raw) * 0.000125;
}

}  // namespace oilgauge
