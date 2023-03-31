#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>

class VulkanCommandBuffer {
  public:
    explicit VulkanCommandBuffer(VkCommandBuffer command_buffer);
    ~VulkanCommandBuffer() = default;

    void begin();
    void end();

    [[nodiscard]] VkCommandBuffer handle() const { return m_command_buffer; }

  private:
    VkCommandBuffer m_command_buffer;
    bool m_recording = false;
};
