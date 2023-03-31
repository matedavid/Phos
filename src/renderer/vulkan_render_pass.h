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
    explicit VulkanRenderPass(std::shared_ptr<VulkanDevice> device);
    ~VulkanRenderPass();

    void begin(
        const std::shared_ptr<VulkanCommandBuffer>& command_buffer,
        const std::shared_ptr<VulkanFramebuffer>& framebuffer);
    void end(const std::shared_ptr<VulkanCommandBuffer>& command_buffer);

    [[nodiscard]] VkRenderPass handle() const { return m_render_pass; }

  private:
    VkRenderPass m_render_pass{};

    std::shared_ptr<VulkanDevice> m_device;
};
