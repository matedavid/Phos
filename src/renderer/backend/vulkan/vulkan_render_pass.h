#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>

namespace Phos {

// Forward declarations
class VulkanDevice;
class VulkanFramebuffer;
class VulkanCommandBuffer;

class VulkanRenderPass {
  public:
    struct Description {
        std::string debug_name;

        bool presentation_target = false; // Is render pass used in presentation pass
        std::shared_ptr<VulkanFramebuffer> target_framebuffer = nullptr;
    };

    VulkanRenderPass(Description description);
    ~VulkanRenderPass() = default;

    void begin(const VulkanCommandBuffer& command_buffer);
    void begin(const VulkanCommandBuffer& command_buffer, const std::shared_ptr<VulkanFramebuffer>& framebuffer);

    void end(const VulkanCommandBuffer& command_buffer);

  private:
    Description m_description;
    VkRenderPassBeginInfo m_begin_info;

    std::vector<VkClearValue> m_clear_values;
};

} // namespace Phos
