#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>

namespace Phos {

class VulkanComputePipeline {
  public:
    VulkanComputePipeline();
    ~VulkanComputePipeline();

    [[nodiscard]] VkPipeline handle() const { return m_pipeline; }
    [[nodiscard]] VkPipelineLayout layout() const { return m_layout; }

  private:
    VkPipeline m_pipeline{};

    VkShaderModule m_shader{};
    VkDescriptorSetLayout m_descriptor_set_layout{};
    VkPipelineLayout m_layout{};
};

} // namespace Phos
