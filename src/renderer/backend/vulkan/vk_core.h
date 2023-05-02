#pragma once

#include "core.h"

namespace Phos {

#define VK_CHECK(expression)          \
    if (expression != VK_SUCCESS) {   \
        PS_FAIL("Vulkan call failed") \
    }

} // namespace Phos
