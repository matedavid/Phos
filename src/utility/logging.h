#pragma once

#ifndef NDEBUG

#include <spdlog/spdlog.h>
#include <cassert>

namespace Phos {

#define PHOS_LOG_SETUP spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v")

#define PHOS_LOG_INFO(...) SPDLOG_INFO(__VA_ARGS__)
#define PHOS_LOG_WARNING(...) SPDLOG_WARN(__VA_ARGS__)
#define PHOS_LOG_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)

#define PHOS_ASSERT(condition, ...)      \
    do {                                 \
        if (!(condition)) {              \
            PHOS_LOG_ERROR(__VA_ARGS__); \
            assert(false);               \
        }                                \
    } while (false)

#define PHOS_FAIL(...) PHOS_ASSERT(false, __VA_ARGS__)

} // namespace Phos

#else

namespace Phos {

#define PHOS_LOG_SETUP

#define PHOS_LOG_INFO(...)
#define PHOS_LOG_WARNING(...)
#define PHOS_LOG_ERROR(...)
#define PHOS_ASSERT(condition, ...)
#define PHOS_FAIL(...)

} // namespace Phos

#endif
