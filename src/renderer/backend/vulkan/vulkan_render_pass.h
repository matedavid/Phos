#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>

#include "renderer/backend/render_pass.h"

namespace Phos {

// Forward declarations
class VulkanDevice;
class VulkanFramebuffer;
class VulkanCommandBuffer;

class VulkanRenderPass : public RenderPass {
  public:
    explicit VulkanRenderPass(Description description);
    ~VulkanRenderPass() override = default;

    void begin(const VulkanCommandBuffer& command_buffer);
    void begin(const VulkanCommandBuffer& command_buffer, const std::shared_ptr<VulkanFramebuffer>& framebuffer);

    void end(const VulkanCommandBuffer& command_buffer);

  private:
    Description m_description;
    VkRenderPassBeginInfo m_begin_info;

    std::vector<VkClearValue> m_clear_values;
};

} // namespace Phos
