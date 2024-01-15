#pragma once

#include <vulkan/vulkan.h>

#include "renderer/backend/compute_pipeline.h"

namespace Phos {

// Forward declarations
class VulkanDescriptorAllocator;
class VulkanShader;
struct VulkanDescriptorInfo;

class VulkanComputePipelineStepBuilder : public ComputePipelineStepBuilder {
  public:
    explicit VulkanComputePipelineStepBuilder(std::shared_ptr<VulkanShader> shader,
                                              std::shared_ptr<VulkanDescriptorAllocator> allocator);
    ~VulkanComputePipelineStepBuilder() override = default;

    void set_push_constants(std::string_view name, uint32_t size, const void* data) override;

    void set(std::string_view name, const std::shared_ptr<Texture>& texture) override;
    void set(std::string_view name, const std::shared_ptr<Texture>& texture, uint32_t mip_level) override;

    [[nodiscard]] VkDescriptorSet build() const;
    [[nodiscard]] std::vector<unsigned char> push_constants() const;

  private:
    std::shared_ptr<VulkanShader> m_shader;
    std::shared_ptr<VulkanDescriptorAllocator> m_allocator;

    std::vector<std::pair<VulkanDescriptorInfo, VkDescriptorBufferInfo>> m_buffer_descriptor_info;
    std::vector<std::pair<VulkanDescriptorInfo, VkDescriptorImageInfo>> m_image_descriptor_info;

    std::vector<std::vector<unsigned char>> m_push_constants_info;
};

class VulkanComputePipeline : public ComputePipeline {
  public:
    explicit VulkanComputePipeline(const Description& description);
    ~VulkanComputePipeline() override;

    void add_step(const std::function<void(StepBuilder&)>& func, glm::uvec3 work_groups) override;
    void execute(const std::shared_ptr<CommandBuffer>& command_buffer) override;

    [[nodiscard]] VkPipeline handle() const { return m_pipeline; }

  private:
    VkPipeline m_pipeline{};
    std::shared_ptr<VulkanShader> m_shader;

    // Descriptors and descriptor sets
    std::shared_ptr<VulkanDescriptorAllocator> m_allocator;

    struct Step {
        VkDescriptorSet set;
        std::vector<unsigned char> push_constant;

        glm::uvec3 work_groups;
    };
    std::vector<Step> m_steps;
};

} // namespace Phos
