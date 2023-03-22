#pragma once

#include <spdlog/spdlog.h>
#include <cassert>

#define CORE_INFO(...) spdlog::info(__VA_ARGS__)
#define CORE_ERROR(...) spdlog::error(__VA_ARGS__)

#define CORE_ASSERT(condition, ...) \
    if (!(condition)) {               \
        CORE_ERROR(__VA_ARGS__);    \
        assert(false);              \
    }

#define CORE_FAIL(...)       \
    CORE_ERROR(__VA_ARGS__); \
    assert(false)
