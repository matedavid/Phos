#pragma once

#include <cstdint>
#include <functional>

namespace Phos {

class UUID {
  public:
    explicit UUID();
    explicit UUID(uint64_t uuid);
    ~UUID() = default;

    UUID(const UUID&) = default;

    bool operator==(const UUID& other) const;
    explicit operator uint64_t() const { return m_uuid; }

  private:
    std::uint64_t m_uuid;
};

} // namespace Phos

// Used to create hash function for UUID, to be used in (for example) unordered_map
namespace std {

template <>
struct hash<Phos::UUID> {
    std::size_t operator()(const Phos::UUID& uuid) const { return static_cast<uint64_t>(uuid); }
};

} // namespace std
