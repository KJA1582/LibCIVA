#include "state/snapshot.h"
#include <cstring>

namespace libciva {

std::vector<uint8_t> Snapshot::serialize() const noexcept {
  std::vector<uint8_t> data;
  data.reserve(sizeof(version) + sizeof(unitCount) + (units.size() * sizeof(UnitData)));

  const uint8_t *versionBytes = reinterpret_cast<const uint8_t *>(&version);
  data.insert(data.end(), versionBytes, versionBytes + sizeof(version));

  const uint8_t *countBytes = reinterpret_cast<const uint8_t *>(&unitCount);
  data.insert(data.end(), countBytes, countBytes + sizeof(unitCount));

  for (const UnitData &unit : units) {
    const uint8_t *unitBytes = reinterpret_cast<const uint8_t *>(&unit);
    data.insert(data.end(), unitBytes, unitBytes + sizeof(UnitData));
  }

  return data;
}

bool Snapshot::deserialize(const std::vector<uint8_t> &data) noexcept {
  if (data.size() < sizeof(version) + sizeof(unitCount)) {
    return false;
  }

  size_t offset = 0;

  uint32_t fileVersion = 0;
  std::memcpy(&fileVersion, data.data() + offset, sizeof(fileVersion));
  offset += sizeof(fileVersion);

  if (fileVersion != LIBCIVA_SNAPSHOT_VERSION && fileVersion != LIBCIVA_SNAPSHOT_VERSION_V1) {
    return false;
  }
  const bool legacy = fileVersion == LIBCIVA_SNAPSHOT_VERSION_V1;
  const size_t unitSize = legacy ? UNIT_DATA_SIZE_V1 : UNIT_DATA_SIZE;

  uint8_t count = 0;
  std::memcpy(&count, data.data() + offset, sizeof(count));
  offset += sizeof(count);

  if (count > 3) {
    return false;
  }

  const size_t expectedSize = sizeof(version) + sizeof(unitCount) + (static_cast<size_t>(count) * unitSize);
  if (data.size() != expectedSize) {
    return false;
  }

  units.assign(count, UnitData{});
  for (size_t i = 0; i < count; ++i) {
    uint8_t *dst = reinterpret_cast<uint8_t *>(&units[i]);
    const uint8_t *src = data.data() + offset;

    if (legacy) {
      std::memcpy(dst, src, V1_REMOVED_OFFSET);
      std::memcpy(dst + V1_REMOVED_OFFSET, src + V1_REMOVED_OFFSET + 1, UNIT_DATA_SIZE - V1_REMOVED_OFFSET);
    } else {
      std::memcpy(dst, src, UNIT_DATA_SIZE);
    }
    offset += unitSize;
  }

  // Upgraded to current version
  version = LIBCIVA_SNAPSHOT_VERSION;
  unitCount = count;

  return true;
}

} // namespace libciva