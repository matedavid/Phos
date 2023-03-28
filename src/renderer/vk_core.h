#pragma once

#include "core.h"

#define VK_CHECK(expression)            \
    if (expression != VK_SUCCESS) {     \
        CORE_FAIL("Vulkan call failed") \
    }
