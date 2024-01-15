#pragma once

#include <vulkan/vulkan.h>
#include <vector>
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

    void begin(const std::shared_ptr<CommandBuffer>& command_buffer) override;
    void begin(const std::shared_ptr<CommandBuffer>& command_buffer,
               const std::shared_ptr<Framebuffer>& framebuffer) override;

    void end(const std::shared_ptr<CommandBuffer>& command_buffer) override;

  private:
    Description m_description;
    VkRenderPassBeginInfo m_begin_info;

    std::vector<VkClearValue> m_clear_values;
};

} // namespace Phos
