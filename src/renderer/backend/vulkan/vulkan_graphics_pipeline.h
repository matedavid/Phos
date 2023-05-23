#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace Phos {

// Forward declarations
class VulkanDevice;
class VulkanShader;
class VulkanCommandBuffer;
class VulkanFramebuffer;

class VulkanGraphicsPipeline {
  public:
    struct Description {
        std::shared_ptr<VulkanShader> shader;
        std::shared_ptr<VulkanFramebuffer> target_framebuffer;
    };

    explicit VulkanGraphicsPipeline(const Description& description);
    ~VulkanGraphicsPipeline();

    void bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const;

    [[nodiscard]] VkPipeline handle() const { return m_pipeline; }
    [[nodiscard]] VkPipelineLayout layout() const { return m_pipeline_layout; }

  private:
    VkPipeline m_pipeline{};
    VkPipelineLayout m_pipeline_layout{};

    std::shared_ptr<VulkanDevice> m_device;
};

} // namespace Phos
