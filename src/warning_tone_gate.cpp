#include "warning_tone_gate.h"

namespace oilgauge {

bool WarningToneGate::update(bool warningActive) {
  const bool shouldPlay = warningActive && !warningActive_;
  warningActive_ = warningActive;
  return shouldPlay;
}

}  // namespace oilgauge
