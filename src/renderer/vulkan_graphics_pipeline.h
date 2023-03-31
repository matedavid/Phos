#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

// Forward declarations
class VulkanDevice;
class VulkanShaderModule;
class VulkanRenderPass;
class VulkanCommandBuffer;

class VulkanGraphicsPipeline {
  public:
    struct Description {
        std::vector<std::shared_ptr<VulkanShaderModule>> shader_modules;
        std::shared_ptr<VulkanRenderPass> render_pass;
    };

    VulkanGraphicsPipeline(std::shared_ptr<VulkanDevice> device, const Description& description);
    ~VulkanGraphicsPipeline();

    void bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const;

  private:
    VkPipeline m_pipeline{};
    VkPipelineLayout m_pipeline_layout{};
    std::vector<VkDescriptorSetLayout> m_descriptor_set_layouts;

    std::shared_ptr<VulkanDevice> m_device;
};
