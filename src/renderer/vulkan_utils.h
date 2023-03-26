#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>

class VulkanUtils {
  public:
    static uint32_t get_format_size(VkFormat format);
};
