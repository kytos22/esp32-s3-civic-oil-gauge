#pragma once

#include <cstdint>

namespace oilgauge {

enum class WarningToneCommand : std::uint8_t {
  none,
  startLoop,
  stopLoop,
};

class WarningToneGate {
 public:
  [[nodiscard]] WarningToneCommand update(bool warningActive);
  [[nodiscard]] bool warningActive() const;

 private:
  bool warningActive_ = false;
};

}  // namespace oilgauge
