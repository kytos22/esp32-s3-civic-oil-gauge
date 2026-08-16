#pragma once

#include <cstddef>
#include <cstdint>

namespace oilgauge {

enum class FrameSlotState : std::uint8_t {
  free,
  snapshot,
  ready,
  inFlight,
};

struct FrameSlotMetadata {
  FrameSlotState state = FrameSlotState::free;
  std::uint64_t generation = 0;
};

// Rendering may reuse a free slot or replace an obsolete complete frame, but it
// must never touch a snapshot being copied or a buffer owned by LCD DMA.
template <std::size_t SlotCount>
[[nodiscard]] constexpr int selectSnapshotSlot(
    const FrameSlotMetadata (&slots)[SlotCount]) {
  for (std::size_t index = 0; index < SlotCount; ++index) {
    if (slots[index].state == FrameSlotState::free) {
      return static_cast<int>(index);
    }
  }

  int oldestReady = -1;
  for (std::size_t index = 0; index < SlotCount; ++index) {
    if (slots[index].state != FrameSlotState::ready) {
      continue;
    }
    if (oldestReady < 0 ||
        slots[index].generation <
            slots[static_cast<std::size_t>(oldestReady)].generation) {
      oldestReady = static_cast<int>(index);
    }
  }
  return oldestReady;
}

// Presentation always consumes the newest coherent generation. Older READY
// generations are stale and can be dropped before the transfer begins.
template <std::size_t SlotCount>
[[nodiscard]] constexpr int selectNewestReadySlot(
    const FrameSlotMetadata (&slots)[SlotCount]) {
  int newestReady = -1;
  for (std::size_t index = 0; index < SlotCount; ++index) {
    if (slots[index].state != FrameSlotState::ready) {
      continue;
    }
    if (newestReady < 0 ||
        slots[index].generation >
            slots[static_cast<std::size_t>(newestReady)].generation) {
      newestReady = static_cast<int>(index);
    }
  }
  return newestReady;
}

}  // namespace oilgauge
