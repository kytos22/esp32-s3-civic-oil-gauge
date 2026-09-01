#pragma once

#include "civic_aux_receiver.h"

namespace oilgauge {

[[nodiscard]] bool startCivicAuxUartReceiver();
[[nodiscard]] bool latestCivicAuxSnapshot(CivicAuxSnapshot& snapshot);

}  // namespace oilgauge
