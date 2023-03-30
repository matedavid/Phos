#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>

// Forward declarations
class VulkanDevice;

class VulkanRenderPass {
  public:
    explicit VulkanRenderPass(std::shared_ptr<VulkanDevice> device);
    ~VulkanRenderPass();

    [[nodiscard]] VkRenderPass handle() const { return m_render_pass; }

  private:
    VkRenderPass m_render_pass{};

    std::shared_ptr<VulkanDevice> m_device;
};
