#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace Phos {

// Forward declarations
class CommandBuffer;

class VulkanComputePipeline {
  public:
    VulkanComputePipeline();
    ~VulkanComputePipeline();

    void bind(const std::shared_ptr<CommandBuffer> & command_buffer);
    void execute(const std::shared_ptr<CommandBuffer>& command_buffer, glm::ivec3 work_groups);

    [[nodiscard]] VkPipeline handle() const { return m_pipeline; }
    [[nodiscard]] VkPipelineLayout layout() const { return m_layout; }

  private:
    VkPipeline m_pipeline{};

    VkShaderModule m_shader{};
    VkDescriptorSetLayout m_descriptor_set_layout{};
    VkPipelineLayout m_layout{};
};

} // namespace Phos
