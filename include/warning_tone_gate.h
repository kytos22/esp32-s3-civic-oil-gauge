#pragma once

namespace oilgauge {

class WarningToneGate {
 public:
  [[nodiscard]] bool update(bool warningActive);

 private:
  bool warningActive_ = false;
};

}  // namespace oilgauge
