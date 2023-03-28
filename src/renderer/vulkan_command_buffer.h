#pragma once

#include <vulkan/vulkan.h>

class VulkanCommandBuffer {
  public:
    explicit VulkanCommandBuffer(VkCommandBuffer command_buffer);
    ~VulkanCommandBuffer() = default;

    [[nodiscard]] VkCommandBuffer handle() const { return m_command_buffer; }

  private:
    VkCommandBuffer m_command_buffer;
};
