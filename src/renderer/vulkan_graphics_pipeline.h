#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

// Forward declarations
class VulkanDevice;
class VulkanShaderModule;

class VulkanGraphicsPipeline {
  public:
    struct Description {
        std::vector<std::shared_ptr<VulkanShaderModule>> shader_modules;
    };

    VulkanGraphicsPipeline(std::shared_ptr<VulkanDevice> device, const Description& description);
    ~VulkanGraphicsPipeline();

  private:
    VkPipeline m_pipeline{};
    VkPipelineLayout m_pipeline_layout{};
    std::vector<VkDescriptorSetLayout> m_descriptor_set_layouts;
    VkRenderPass m_render_pass{};

    std::shared_ptr<VulkanDevice> m_device;
};
