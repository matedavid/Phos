#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <memory>

#ifndef NDEBUG

#include <tracy/Tracy.hpp>
#include <spdlog/spdlog.h>
#include <cassert>

#endif

namespace Phos {

// Profiling
#ifndef NDEBUG

#define PHOS_PROFILE_FRAMEMARK FrameMark
#define PHOS_PROFILE_ZONE_SCOPED ZoneScoped
#define PHOS_PROFILE_ZONE_SCOPED_NAMED(name) ZoneScopedN(name)

#else

#define PHOS_PROFILE_FRAMEMARK FrameMark
#define PHOS_PROFILE_ZONE_SCOPED
#define PHOS_PROFILE_ZONE_SCOPED_NAMED(name)

#endif

// Logging & Assertions
#ifndef NDEBUG

#define PS_INFO(...) spdlog::info(__VA_ARGS__)
#define PS_WARNING(...) spdlog::warn(__VA_ARGS__)
#define PS_ERROR(...) spdlog::error(__VA_ARGS__)

#define PS_ASSERT(condition, ...) \
    if (!(condition)) {           \
        PS_ERROR(__VA_ARGS__);    \
        assert(false);            \
    }

#define PS_FAIL(...)       \
    PS_ERROR(__VA_ARGS__); \
    assert(false);

#else

#define PS_INFO(...)
#define PS_WARNING(...)
#define PS_ERROR(...)
#define PS_ASSERT(condition, ...)
#define PS_FAIL(...)

#endif

// Constants
constexpr uint32_t MAX_NUM_ENTITIES = 500;

} // namespace Phos