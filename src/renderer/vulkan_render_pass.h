#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>

// Forward declarations
class VulkanDevice;
class VulkanFramebuffer;
class VulkanCommandBuffer;

class VulkanRenderPass {
  public:
    VulkanRenderPass();
    ~VulkanRenderPass();

    void begin(const VulkanCommandBuffer& command_buffer, const VulkanFramebuffer& framebuffer);
    void end(const VulkanCommandBuffer& command_buffer);

    [[nodiscard]] VkRenderPass handle() const { return m_render_pass; }

  private:
    VkRenderPass m_render_pass{};
};
