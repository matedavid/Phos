#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace Phos {

// Forward declarations
class VulkanCommandBuffer;

class VulkanCommandPool {
  public:
    VulkanCommandPool(VkDevice raw_device, uint32_t queue_family);
    ~VulkanCommandPool();

    [[nodiscard]] std::vector<VkCommandBuffer> allocate(uint32_t count) const;

    void free_command_buffer(VkCommandBuffer command_buffer) const;

    [[nodiscard]] uint32_t get_queue_family() const { return m_queue_family; }

  private:
    VkCommandPool m_command_pool{VK_NULL_HANDLE};
    uint32_t m_queue_family;

    VkDevice m_raw_device{VK_NULL_HANDLE};
};

} // namespace Phos
