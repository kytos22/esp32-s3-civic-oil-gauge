#include "civic_aux_receiver.h"

#include <algorithm>
#include <cstring>
#include <limits>

namespace oilgauge {

namespace {

constexpr std::size_t kOffsetVersion = 2;
constexpr std::size_t kOffsetType = 3;
constexpr std::size_t kOffsetSource = 4;
constexpr std::size_t kOffsetTarget = 5;
constexpr std::size_t kOffsetPayloadLength = 6;
constexpr std::size_t kOffsetFlags = 7;
constexpr std::size_t kOffsetSequence = 8;
constexpr std::size_t kOffsetUptime = 10;
constexpr std::size_t kOffsetPayload = 14;

std::uint16_t readU16Le(const std::uint8_t* input) {
  return static_cast<std::uint16_t>(input[0]) |
         static_cast<std::uint16_t>(
             static_cast<std::uint16_t>(input[1]) << 8U);
}

std::uint32_t readU32Le(const std::uint8_t* input) {
  return static_cast<std::uint32_t>(input[0]) |
         (static_cast<std::uint32_t>(input[1]) << 8U) |
         (static_cast<std::uint32_t>(input[2]) << 16U) |
         (static_cast<std::uint32_t>(input[3]) << 24U);
}

bool isUptimeWrap(std::uint32_t previous, std::uint32_t current) {
  return previous >= 0xF0000000U && current <= 0x0FFFFFFFU;
}

}  // namespace

std::uint16_t civicAuxCrc16CcittFalse(const std::uint8_t* data,
                                      std::size_t length) {
  std::uint16_t crc = 0xFFFFU;
  for (std::size_t index = 0; index < length; ++index) {
    crc ^= static_cast<std::uint16_t>(data[index]) << 8U;
    for (std::uint8_t bit = 0; bit < 8U; ++bit) {
      crc = (crc & 0x8000U) != 0U
                ? static_cast<std::uint16_t>((crc << 1U) ^ 0x1021U)
                : static_cast<std::uint16_t>(crc << 1U);
    }
  }
  return crc;
}

void CivicAuxFrameParser::discardPrefix(std::size_t count, bool invalid) {
  count = std::min(count, buffered_);
  if (invalid && count != 0U) {
    ++diagnostics_.invalidEvents;
    diagnostics_.discardedBytes += static_cast<std::uint32_t>(count);
  }
  if (count >= buffered_) {
    buffered_ = 0;
    return;
  }
  std::memmove(buffer_.data(), buffer_.data() + count, buffered_ - count);
  buffered_ -= count;
}

bool CivicAuxFrameParser::alignToSync() {
  if (buffered_ < 2U) {
    return false;
  }

  std::size_t syncOffset = 0;
  while (syncOffset + 1U < buffered_ &&
         !(buffer_[syncOffset] == kCivicAuxSync0 &&
           buffer_[syncOffset + 1U] == kCivicAuxSync1)) {
    ++syncOffset;
  }
  if (syncOffset + 1U < buffered_) {
    if (syncOffset != 0U) {
      discardPrefix(syncOffset, true);
    }
    return true;
  }

  const bool retainPossibleSync = buffer_[buffered_ - 1U] == kCivicAuxSync0;
  const std::size_t discardCount = buffered_ - (retainPossibleSync ? 1U : 0U);
  if (discardCount != 0U) {
    discardPrefix(discardCount, true);
  }
  return false;
}

bool CivicAuxFrameParser::candidateHasValidCrc(
    std::size_t offset,
    std::size_t frameLength) const {
  if (offset + frameLength > buffered_ ||
      frameLength < kCivicAuxMinimumFrameSize) {
    return false;
  }
  const std::uint16_t expected = readU16Le(
      buffer_.data() + offset + frameLength - kCivicAuxCrcSize);
  const std::uint16_t actual = civicAuxCrc16CcittFalse(
      buffer_.data() + offset + kOffsetVersion, frameLength - 4U);
  return expected == actual;
}

std::size_t CivicAuxFrameParser::findLaterCompleteFrame() const {
  for (std::size_t offset = 1U; offset + 1U < buffered_; ++offset) {
    if (buffer_[offset] != kCivicAuxSync0 ||
        buffer_[offset + 1U] != kCivicAuxSync1) {
      continue;
    }
    if (offset + kOffsetPayloadLength >= buffered_) {
      continue;
    }
    const std::size_t payloadLength = buffer_[offset + kOffsetPayloadLength];
    if (payloadLength > kCivicAuxMaximumPayloadSize) {
      continue;
    }
    const std::size_t frameLength = kCivicAuxMinimumFrameSize + payloadLength;
    if (offset + frameLength <= buffered_ &&
        buffer_[offset + kOffsetVersion] == kCivicAuxProtocolVersion &&
        candidateHasValidCrc(offset, frameLength)) {
      return offset;
    }
  }
  return kNoOffset;
}

void CivicAuxFrameParser::decodeCandidate(std::size_t frameLength,
                                          CivicAuxFrameV1& frame) const {
  frame.version = buffer_[kOffsetVersion];
  frame.type = buffer_[kOffsetType];
  frame.source = buffer_[kOffsetSource];
  frame.targetMask = buffer_[kOffsetTarget];
  frame.payloadLength = buffer_[kOffsetPayloadLength];
  frame.flags = buffer_[kOffsetFlags];
  frame.sequence = readU16Le(buffer_.data() + kOffsetSequence);
  frame.uptimeMs = readU32Le(buffer_.data() + kOffsetUptime);
  std::fill(frame.payload.begin(), frame.payload.end(), 0U);
  if (frame.payloadLength != 0U) {
    std::memcpy(frame.payload.data(),
                buffer_.data() + kOffsetPayload,
                frame.payloadLength);
  }
  (void)frameLength;
}

bool CivicAuxFrameParser::feed(std::uint8_t byte,
                               CivicAuxFrameV1& completedFrame) {
  ++diagnostics_.receivedBytes;
  if (buffered_ == buffer_.size()) {
    discardPrefix(1U, true);
  }
  buffer_[buffered_++] = byte;

  for (;;) {
    if (!alignToSync()) {
      return false;
    }
    if (buffered_ <= kOffsetPayloadLength) {
      return false;
    }

    const std::size_t payloadLength = buffer_[kOffsetPayloadLength];
    if (payloadLength > kCivicAuxMaximumPayloadSize) {
      ++diagnostics_.lengthErrors;
      discardPrefix(1U, true);
      continue;
    }

    const std::size_t frameLength = kCivicAuxMinimumFrameSize + payloadLength;
    if (buffered_ < frameLength) {
      const std::size_t laterFrame = findLaterCompleteFrame();
      if (laterFrame != kNoOffset) {
        discardPrefix(laterFrame, true);
        continue;
      }
      return false;
    }

    if (!candidateHasValidCrc(0U, frameLength)) {
      ++diagnostics_.crcErrors;
      discardPrefix(1U, true);
      continue;
    }
    if (buffer_[kOffsetVersion] != kCivicAuxProtocolVersion) {
      ++diagnostics_.versionErrors;
      discardPrefix(frameLength, true);
      continue;
    }

    decodeCandidate(frameLength, completedFrame);
    ++diagnostics_.structurallyValidFrames;
    discardPrefix(frameLength, false);
    return true;
  }
}

void CivicAuxFrameParser::reset() {
  buffered_ = 0;
  diagnostics_ = {};
}

bool decodeCivicAuxAmbientLight(const CivicAuxFrameV1& frame,
                                CivicAuxAmbientLightPayload& payload) {
  if (frame.type != kCivicAuxAmbientLightType ||
      frame.payloadLength != kCivicAuxKnownPayloadSize) {
    return false;
  }
  payload.sensorState = frame.payload[0];
  payload.rangeProfile = frame.payload[1];
  payload.alsRaw = readU16Le(frame.payload.data() + 2);
  payload.whiteRaw = readU16Le(frame.payload.data() + 4);
  payload.sampleAgeMs = readU16Le(frame.payload.data() + 6);
  payload.filteredMillilux = readU32Le(frame.payload.data() + 8);
  return true;
}

bool civicAuxAmbientLightUsable(const CivicAuxFrameV1& frame,
                                const CivicAuxAmbientLightPayload& payload) {
  const auto state = static_cast<CivicAuxSensorState>(payload.sensorState);
  const bool stateUsable = state == CivicAuxSensorState::valid ||
                           state == CivicAuxSensorState::degraded;
  return frame.type == kCivicAuxAmbientLightType &&
         frame.source == kCivicAuxAmbientLightSource &&
         (frame.targetMask & kCivicAuxTargetOil) != 0U &&
         frame.payloadLength == kCivicAuxKnownPayloadSize &&
         (frame.flags & kCivicAuxFlagDataValid) != 0U && stateUsable &&
         payload.sampleAgeMs < kCivicAuxLuxFreshnessMs;
}

void CivicAuxReceiver::publishDiagnostics() {
  snapshot_.diagnostics.parser = parser_.diagnostics();
}

void CivicAuxReceiver::noteInvalidTraffic(std::uint64_t localNowMs) {
  if (!invalidTrafficSeen_) {
    invalidTrafficSeen_ = true;
    snapshot_.invalidTrafficStartedAtMs = localNowMs;
  }
  snapshot_.lastInvalidTrafficAtMs = localNowMs;
  snapshot_.consecutiveUsableAmbientFrames = 0;
  if (localNowMs - snapshot_.invalidTrafficStartedAtMs >=
      kCivicAuxInvalidTrafficFallbackMs) {
    snapshot_.invalidTrafficFallback = true;
  }
}

void CivicAuxReceiver::clearInvalidTraffic() {
  invalidTrafficSeen_ = false;
  snapshot_.invalidTrafficFallback = false;
  snapshot_.invalidTrafficStartedAtMs = 0;
  snapshot_.lastInvalidTrafficAtMs = 0;
}

bool CivicAuxReceiver::updateContinuity(const CivicAuxFrameV1& frame) {
  bool continuityBroken = false;
  if (haveContinuity_) {
    const bool uptimeBackwards = frame.uptimeMs < snapshot_.lastHubUptimeMs;
    const bool uptimeWrapped =
        uptimeBackwards &&
        isUptimeWrap(snapshot_.lastHubUptimeMs, frame.uptimeMs);
    const bool hubRestarted = uptimeBackwards && !uptimeWrapped;
    if (uptimeWrapped) {
      ++snapshot_.diagnostics.uptimeWraps;
    } else if (hubRestarted) {
      ++snapshot_.diagnostics.hubRestarts;
      continuityBroken = true;
      snapshot_.hasUsableLux = false;
      snapshot_.lastUsableReceivedAtMs = 0;
      snapshot_.lastUsableUntilMs = 0;
    }

    if (!hubRestarted) {
      const std::uint16_t expected = static_cast<std::uint16_t>(
          snapshot_.lastFrameSequence + 1U);
      if (frame.sequence == expected) {
        if (expected == 0U && snapshot_.lastFrameSequence != 0U) {
          ++snapshot_.diagnostics.sequenceWraps;
        }
      } else {
        ++snapshot_.diagnostics.sequenceDiscontinuities;
        continuityBroken = true;
      }
    }
  } else {
    haveContinuity_ = true;
  }

  snapshot_.lastFrameSequence = frame.sequence;
  snapshot_.lastHubUptimeMs = frame.uptimeMs;
  if (continuityBroken) {
    snapshot_.consecutiveUsableAmbientFrames = 0;
  }
  return continuityBroken;
}

void CivicAuxReceiver::processAmbientLight(const CivicAuxFrameV1& frame,
                                            std::uint64_t localNowMs,
                                            bool continuityBroken) {
  CivicAuxAmbientLightPayload payload{};
  if (!decodeCivicAuxAmbientLight(frame, payload)) {
    ++snapshot_.diagnostics.knownLengthErrors;
    noteInvalidTraffic(localNowMs);
    return;
  }

  snapshot_.hasAmbientFrame = true;
  snapshot_.sensorState = payload.sensorState;
  snapshot_.rangeProfile = payload.rangeProfile;
  snapshot_.alsRaw = payload.alsRaw;
  snapshot_.whiteRaw = payload.whiteRaw;
  snapshot_.sampleAgeMs = payload.sampleAgeMs;
  snapshot_.filteredMillilux = payload.filteredMillilux;
  snapshot_.ambientSequence = frame.sequence;
  snapshot_.ambientUptimeMs = frame.uptimeMs;
  snapshot_.ambientReceivedAtMs = localNowMs;
  ++snapshot_.diagnostics.ambientFrames;

  const bool usable = civicAuxAmbientLightUsable(frame, payload);
  snapshot_.latestAmbientUsable = usable;
  clearInvalidTraffic();
  if (!usable) {
    snapshot_.consecutiveUsableAmbientFrames = 0;
    ++snapshot_.diagnostics.unusableAmbientFrames;
    return;
  }

  if (continuityBroken) {
    snapshot_.consecutiveUsableAmbientFrames = 0;
  }
  if (snapshot_.consecutiveUsableAmbientFrames <
      std::numeric_limits<std::uint8_t>::max()) {
    ++snapshot_.consecutiveUsableAmbientFrames;
  }
  snapshot_.hasUsableLux = true;
  snapshot_.lastUsableMillilux = payload.filteredMillilux;
  snapshot_.lastUsableReceivedAtMs = localNowMs;
  snapshot_.lastUsableUntilMs =
      localNowMs + (kCivicAuxLuxFreshnessMs - payload.sampleAgeMs);
  ++snapshot_.usableGeneration;
  ++snapshot_.diagnostics.usableAmbientFrames;
}

void CivicAuxReceiver::processFrame(const CivicAuxFrameV1& frame,
                                    std::uint64_t localNowMs) {
  if ((frame.targetMask & kCivicAuxTargetOil) == 0U) {
    ++snapshot_.diagnostics.targetMisses;
    noteInvalidTraffic(localNowMs);
    return;
  }

  if (frame.type == kCivicAuxAmbientLightType) {
    if (frame.payloadLength != kCivicAuxKnownPayloadSize) {
      ++snapshot_.diagnostics.knownLengthErrors;
      noteInvalidTraffic(localNowMs);
      return;
    }
    if (frame.source != kCivicAuxAmbientLightSource) {
      ++snapshot_.diagnostics.sourceErrors;
      noteInvalidTraffic(localNowMs);
      return;
    }
    const bool continuityBroken = updateContinuity(frame);
    ++snapshot_.diagnostics.acceptedFrames;
    processAmbientLight(frame, localNowMs, continuityBroken);
    return;
  }

  if (frame.type == kCivicAuxHubStatusType) {
    if (frame.payloadLength != kCivicAuxKnownPayloadSize) {
      ++snapshot_.diagnostics.knownLengthErrors;
      noteInvalidTraffic(localNowMs);
      return;
    }
    if (frame.source != kCivicAuxHubSource) {
      ++snapshot_.diagnostics.sourceErrors;
      noteInvalidTraffic(localNowMs);
      return;
    }
    (void)updateContinuity(frame);
    ++snapshot_.diagnostics.acceptedFrames;
    ++snapshot_.diagnostics.hubStatusFrames;
    clearInvalidTraffic();
    return;
  }

  (void)updateContinuity(frame);
  ++snapshot_.diagnostics.acceptedFrames;
  ++snapshot_.diagnostics.unknownFrames;
  clearInvalidTraffic();
}

bool CivicAuxReceiver::feed(std::uint8_t byte, std::uint64_t localNowMs) {
  snapshot_.lastTrafficAtMs = localNowMs;
  const std::uint32_t invalidBefore = parser_.diagnostics().invalidEvents;
  CivicAuxFrameV1 frame{};
  const bool completed = parser_.feed(byte, frame);
  const bool parserInvalid =
      parser_.diagnostics().invalidEvents != invalidBefore;
  if (parserInvalid) {
    noteInvalidTraffic(localNowMs);
  }
  if (completed) {
    processFrame(frame, localNowMs);
  }
  publishDiagnostics();
  const bool changed = parserInvalid || completed;
  if (changed) {
    ++snapshot_.generation;
  }
  return changed;
}

void CivicAuxReceiver::noteUartReadError(std::uint64_t localNowMs) {
  ++snapshot_.diagnostics.uartReadErrors;
  noteInvalidTraffic(localNowMs);
  ++snapshot_.generation;
}

void CivicAuxReceiver::setRunning(bool running) {
  snapshot_.receiverRunning = running;
  ++snapshot_.generation;
}

void CivicAuxReceiver::reset() {
  const bool running = snapshot_.receiverRunning;
  parser_.reset();
  snapshot_ = {};
  snapshot_.receiverRunning = running;
  haveContinuity_ = false;
  invalidTrafficSeen_ = false;
}

}  // namespace oilgauge
