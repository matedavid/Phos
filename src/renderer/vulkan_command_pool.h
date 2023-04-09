#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>

// Forward declarations
class VulkanCommandBuffer;

class VulkanCommandPool {
  public:
    VulkanCommandPool(VkDevice raw_device, uint32_t queue_family);
    ~VulkanCommandPool();

    [[nodiscard]] std::vector<std::shared_ptr<VulkanCommandBuffer>> allocate(uint32_t count) const;

    [[nodiscard]] uint32_t get_queue_family() const { return m_queue_family; }

  private:
    VkCommandPool m_command_pool{VK_NULL_HANDLE};
    uint32_t m_queue_family;

    VkDevice m_raw_device{VK_NULL_HANDLE};
};
