#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

#include "renderer/backend/graphics_pipeline.h"

namespace Phos {

// Forward declarations
class VulkanDevice;
class VulkanShader;
class VulkanFramebuffer;

class VulkanGraphicsPipeline : public GraphicsPipeline {
  public:
    explicit VulkanGraphicsPipeline(const Description& description);
    ~VulkanGraphicsPipeline() override;

    void bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const override;

    [[nodiscard]] VkPipeline handle() const { return m_pipeline; }
    [[nodiscard]] VkPipelineLayout layout() const { return m_pipeline_layout; }

  private:
    VkPipeline m_pipeline{};
    VkPipelineLayout m_pipeline_layout{};

    std::shared_ptr<VulkanDevice> m_device;
};

} // namespace Phos
