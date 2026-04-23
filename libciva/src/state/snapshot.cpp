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

  std::memcpy(&version, data.data() + offset, sizeof(version));
  offset += sizeof(version);

  if (version != VERSION) {
    return false;
  }

  std::memcpy(&unitCount, data.data() + offset, sizeof(unitCount));
  offset += sizeof(unitCount);

  if (unitCount > 3) {
    return false;
  }

  const size_t expectedSize = sizeof(version) + sizeof(unitCount) + (static_cast<size_t>(unitCount) * sizeof(UnitData));
  if (data.size() != expectedSize) {
    return false;
  }

  units.resize(unitCount);
  for (size_t i = 0; i < unitCount; ++i) {
    std::memcpy(&units[i], data.data() + offset, sizeof(UnitData));
    offset += sizeof(UnitData);
  }

  return true;
}

} // namespace libciva