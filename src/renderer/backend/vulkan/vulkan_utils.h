#pragma once

#include <vulkan/vulkan.h>

namespace Phos {

class VulkanUtils {
  public:
    static uint32_t get_format_size(VkFormat format);
};

} // namespace Phos
