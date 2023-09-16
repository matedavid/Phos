#pragma once

#include "vk_core.h"
#include <vulkan/vulkan.h>

#include "renderer/backend/compute_pipeline.h"

namespace Phos {

// Forward declarations
class VulkanDescriptorAllocator;
class VulkanShader;
struct VulkanDescriptorInfo;

class VulkanComputePipeline : public ComputePipeline {
  public:
    explicit VulkanComputePipeline(const Description& description);
    ~VulkanComputePipeline() override;

    void bind(const std::shared_ptr<CommandBuffer>& command_buffer) override;
    void execute(const std::shared_ptr<CommandBuffer>& command_buffer, glm::ivec3 work_groups) override;

    /// Builds the descriptor sets
    [[nodiscard]] bool bake() override;
    /// Invalidates de descriptors sets
    void invalidate() override;

    void set(std::string_view name, const std::shared_ptr<Texture>& texture) override;
    void set(std::string_view name, const std::shared_ptr<Texture>& texture, uint32_t mip_level) override;

    [[nodiscard]] VkPipeline handle() const { return m_pipeline; }

    // @TODO: Remove
    [[nodiscard]] VkPipelineLayout layout() const;

  private:
    VkPipeline m_pipeline{};
    std::shared_ptr<VulkanShader> m_shader;

    // Descriptors and descriptor sets
    std::shared_ptr<VulkanDescriptorAllocator> m_allocator;

    std::vector<std::pair<VulkanDescriptorInfo, VkDescriptorBufferInfo>> m_buffer_descriptor_info;
    std::vector<std::pair<VulkanDescriptorInfo, VkDescriptorImageInfo>> m_image_descriptor_info;

    VkDescriptorSet m_set{VK_NULL_HANDLE};

    [[nodiscard]] static bool is_valid_texture_type(VkDescriptorType type);
};

} // namespace Phos
