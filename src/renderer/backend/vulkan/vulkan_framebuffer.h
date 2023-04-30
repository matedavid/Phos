#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>

namespace Phos {

// Forward declarations
class VulkanDevice;
class VulkanRenderPass;

class VulkanFramebuffer {
  public:
    VulkanFramebuffer(const std::shared_ptr<VulkanRenderPass>& render_pass,
                      uint32_t width,
                      uint32_t height,
                      const std::vector<VkImageView>& attachments);
    ~VulkanFramebuffer();

    [[nodiscard]] uint32_t width() const { return m_width; }
    [[nodiscard]] uint32_t height() const { return m_height; }

    [[nodiscard]] VkFramebuffer handle() const { return m_framebuffer; }

  private:
    VkFramebuffer m_framebuffer{VK_NULL_HANDLE};

    uint32_t m_width;
    uint32_t m_height;

    std::shared_ptr<VulkanDevice> m_device;
};

} // namespace Phos