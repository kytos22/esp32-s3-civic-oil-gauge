#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace oilgauge {

inline constexpr std::uint8_t kCivicAuxSync0 = 0xA5;
inline constexpr std::uint8_t kCivicAuxSync1 = 0x5A;
inline constexpr std::uint8_t kCivicAuxProtocolVersion = 0x01;
inline constexpr std::size_t kCivicAuxMaximumPayloadSize = 48;
inline constexpr std::size_t kCivicAuxHeaderSize = 14;
inline constexpr std::size_t kCivicAuxCrcSize = 2;
inline constexpr std::size_t kCivicAuxMinimumFrameSize =
    kCivicAuxHeaderSize + kCivicAuxCrcSize;
inline constexpr std::size_t kCivicAuxMaximumFrameSize =
    kCivicAuxHeaderSize + kCivicAuxMaximumPayloadSize + kCivicAuxCrcSize;
inline constexpr std::uint8_t kCivicAuxTargetOil = 1U << 0U;
inline constexpr std::uint8_t kCivicAuxFlagDataValid = 1U << 0U;
inline constexpr std::uint8_t kCivicAuxHubStatusType = 0x01;
inline constexpr std::uint8_t kCivicAuxAmbientLightType = 0x10;
inline constexpr std::uint8_t kCivicAuxHubSource = 0x01;
inline constexpr std::uint8_t kCivicAuxAmbientLightSource = 0x10;
inline constexpr std::size_t kCivicAuxKnownPayloadSize = 12;
inline constexpr std::uint64_t kCivicAuxLuxFreshnessMs = 2'000;
inline constexpr std::uint64_t kCivicAuxInvalidTrafficFallbackMs = 1'000;

enum class CivicAuxSensorState : std::uint8_t {
  initializing = 0,
  valid = 1,
  degraded = 2,
  missing = 3,
};

enum class CivicAuxRangeProfile : std::uint8_t {
  dark = 0,
  normal = 1,
  intense = 2,
};

struct CivicAuxFrameV1 {
  std::uint8_t version = kCivicAuxProtocolVersion;
  std::uint8_t type = 0;
  std::uint8_t source = 0;
  std::uint8_t targetMask = 0;
  std::uint8_t payloadLength = 0;
  std::uint8_t flags = 0;
  std::uint16_t sequence = 0;
  std::uint32_t uptimeMs = 0;
  std::array<std::uint8_t, kCivicAuxMaximumPayloadSize> payload{};
};

struct CivicAuxAmbientLightPayload {
  std::uint8_t sensorState = 0;
  std::uint8_t rangeProfile = 0;
  std::uint16_t alsRaw = 0;
  std::uint16_t whiteRaw = 0;
  std::uint16_t sampleAgeMs = 0;
  std::uint32_t filteredMillilux = 0;
};

struct CivicAuxParserDiagnostics {
  std::uint32_t receivedBytes = 0;
  std::uint32_t structurallyValidFrames = 0;
  std::uint32_t invalidEvents = 0;
  std::uint32_t crcErrors = 0;
  std::uint32_t lengthErrors = 0;
  std::uint32_t versionErrors = 0;
  std::uint32_t discardedBytes = 0;
};

class CivicAuxFrameParser {
 public:
  [[nodiscard]] bool feed(std::uint8_t byte, CivicAuxFrameV1& completedFrame);
  void reset();
  [[nodiscard]] const CivicAuxParserDiagnostics& diagnostics() const {
    return diagnostics_;
  }

 private:
  static constexpr std::size_t kBufferCapacity =
      kCivicAuxMaximumFrameSize * 2U;
  static constexpr std::size_t kNoOffset = static_cast<std::size_t>(-1);

  void discardPrefix(std::size_t count, bool invalid);
  [[nodiscard]] bool alignToSync();
  [[nodiscard]] std::size_t findLaterCompleteFrame() const;
  [[nodiscard]] bool candidateHasValidCrc(std::size_t offset,
                                          std::size_t frameLength) const;
  void decodeCandidate(std::size_t frameLength, CivicAuxFrameV1& frame) const;

  std::array<std::uint8_t, kBufferCapacity> buffer_{};
  std::size_t buffered_ = 0;
  CivicAuxParserDiagnostics diagnostics_{};
};

struct CivicAuxReceiverDiagnostics {
  CivicAuxParserDiagnostics parser{};
  std::uint32_t acceptedFrames = 0;
  std::uint32_t hubStatusFrames = 0;
  std::uint32_t ambientFrames = 0;
  std::uint32_t usableAmbientFrames = 0;
  std::uint32_t unusableAmbientFrames = 0;
  std::uint32_t unknownFrames = 0;
  std::uint32_t targetMisses = 0;
  std::uint32_t sourceErrors = 0;
  std::uint32_t knownLengthErrors = 0;
  std::uint32_t sequenceDiscontinuities = 0;
  std::uint32_t sequenceWraps = 0;
  std::uint32_t uptimeWraps = 0;
  std::uint32_t hubRestarts = 0;
  std::uint32_t uartReadErrors = 0;
};

struct CivicAuxSnapshot {
  bool receiverRunning = false;
  bool hasAmbientFrame = false;
  bool latestAmbientUsable = false;
  bool hasUsableLux = false;
  bool invalidTrafficFallback = false;
  std::uint8_t consecutiveUsableAmbientFrames = 0;
  std::uint8_t sensorState = 0;
  std::uint8_t rangeProfile = 0;
  std::uint16_t alsRaw = 0;
  std::uint16_t whiteRaw = 0;
  std::uint16_t sampleAgeMs = 0;
  std::uint16_t ambientSequence = 0;
  std::uint16_t lastFrameSequence = 0;
  std::uint32_t filteredMillilux = 0;
  std::uint32_t lastUsableMillilux = 0;
  std::uint32_t ambientUptimeMs = 0;
  std::uint32_t lastHubUptimeMs = 0;
  std::uint64_t ambientReceivedAtMs = 0;
  std::uint64_t lastUsableReceivedAtMs = 0;
  std::uint64_t lastUsableUntilMs = 0;
  std::uint64_t lastTrafficAtMs = 0;
  std::uint64_t invalidTrafficStartedAtMs = 0;
  std::uint64_t lastInvalidTrafficAtMs = 0;
  std::uint32_t generation = 0;
  std::uint32_t usableGeneration = 0;
  CivicAuxReceiverDiagnostics diagnostics{};
};

class CivicAuxReceiver {
 public:
  [[nodiscard]] bool feed(std::uint8_t byte, std::uint64_t localNowMs);
  void noteUartReadError(std::uint64_t localNowMs);
  void setRunning(bool running);
  void reset();
  [[nodiscard]] const CivicAuxSnapshot& snapshot() const { return snapshot_; }

 private:
  void noteInvalidTraffic(std::uint64_t localNowMs);
  void clearInvalidTraffic();
  [[nodiscard]] bool updateContinuity(const CivicAuxFrameV1& frame);
  void processFrame(const CivicAuxFrameV1& frame, std::uint64_t localNowMs);
  void processAmbientLight(const CivicAuxFrameV1& frame,
                           std::uint64_t localNowMs,
                           bool continuityBroken);
  void publishDiagnostics();

  CivicAuxFrameParser parser_{};
  CivicAuxSnapshot snapshot_{};
  bool haveContinuity_ = false;
  bool invalidTrafficSeen_ = false;
};

[[nodiscard]] std::uint16_t civicAuxCrc16CcittFalse(
    const std::uint8_t* data,
    std::size_t length);
[[nodiscard]] bool decodeCivicAuxAmbientLight(
    const CivicAuxFrameV1& frame,
    CivicAuxAmbientLightPayload& payload);
[[nodiscard]] bool civicAuxAmbientLightUsable(
    const CivicAuxFrameV1& frame,
    const CivicAuxAmbientLightPayload& payload);

}  // namespace oilgauge
