#pragma once

#include <spdlog/spdlog.h>
#include <cassert>

namespace Phos {

#define PS_INFO(...) spdlog::info(__VA_ARGS__);
#define PS_WARNING(...) spdlog::warn(__VA_ARGS__);
#define PS_ERROR(...) spdlog::error(__VA_ARGS__);

#define PS_ASSERT(condition, ...) \
    if (!(condition)) {             \
        PS_ERROR(__VA_ARGS__)       \
        assert(false);              \
    }

#define PS_FAIL(...)    \
    PS_ERROR(__VA_ARGS__) \
    assert(false);

} // namespace Phos