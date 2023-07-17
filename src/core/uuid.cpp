#include "uuid.h"

#include <random>

namespace Phos {

static std::random_device s_random_device;
static std::mt19937_64 s_engine(s_random_device());
static std::uniform_int_distribution<uint64_t> s_uniform_distribution;

UUID::UUID() : m_uuid(s_uniform_distribution(s_engine)) {}

bool UUID::operator==(const UUID& other) const {
    return m_uuid == other.m_uuid;
}

} // namespace Phos
