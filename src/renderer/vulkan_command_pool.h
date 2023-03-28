#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>

// Forward declarations
class VulkanDevice;
class VulkanCommandBuffer;

class VulkanCommandPool {
  public:
    VulkanCommandPool(std::shared_ptr<VulkanDevice> device, uint32_t queue_family, uint32_t command_buffer_count);
    ~VulkanCommandPool();

    [[nodiscard]] std::vector<std::shared_ptr<VulkanCommandBuffer>> get_command_buffers() const {
        return m_command_buffers;
    }
    [[nodiscard]] uint32_t get_queue_family() const { return m_queue_family; }

  private:
    VkCommandPool m_command_pool;
    uint32_t m_queue_family;

    std::vector<std::shared_ptr<VulkanCommandBuffer>> m_command_buffers;

    std::shared_ptr<VulkanDevice> m_device;
};
