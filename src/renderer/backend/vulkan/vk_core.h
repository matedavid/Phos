#pragma once

#include "utility/logging.h"

namespace Phos {

#ifndef NDEBUG

#define VK_CHECK(expression)                 \
    do {                                     \
        if ((expression) != VK_SUCCESS) {    \
            PHOS_FAIL("Vulkan call failed"); \
        }                                    \
    } while (false)

#else

#define VK_CHECK(expression) expression

#endif

} // namespace Phos
