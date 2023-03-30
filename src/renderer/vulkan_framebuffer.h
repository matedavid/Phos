#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>

// Forward declarations
class VulkanDevice;
class VulkanRenderPass;

class VulkanFramebuffer {
  public:
    VulkanFramebuffer(
        std::shared_ptr<VulkanDevice> device,
        const std::shared_ptr<VulkanRenderPass>& render_pass,
        uint32_t width,
        uint32_t height,
        const std::vector<VkImageView>& attachments);
    ~VulkanFramebuffer();

  private:
    VkFramebuffer m_framebuffer;

    std::shared_ptr<VulkanDevice> m_device;
};
