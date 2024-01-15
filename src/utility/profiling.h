#pragma once

#ifndef NDEBUG

#include <tracy/Tracy.hpp>

namespace Phos {

#define PHOS_PROFILE_FRAMEMARK FrameMark
#define PHOS_PROFILE_ZONE_SCOPED ZoneScoped
#define PHOS_PROFILE_ZONE_SCOPED_NAMED(name) ZoneScopedN(name)

} // namespace Phos

#else

namespace Phos {

#define PHOS_PROFILE_FRAMEMARK
#define PHOS_PROFILE_ZONE_SCOPED
#define PHOS_PROFILE_ZONE_SCOPED_NAMED(name)

} // namespace Phos

#endif
