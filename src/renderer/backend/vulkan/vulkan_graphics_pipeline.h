#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

#include "renderer/backend/graphics_pipeline.h"

namespace Phos {

// Forward declarations
class VulkanShader;
class VulkanDescriptorBuilder;
class VulkanDescriptorAllocator;
struct VulkanDescriptorInfo;

class VulkanGraphicsPipeline : public GraphicsPipeline {
  public:
    explicit VulkanGraphicsPipeline(const Description& description);
    ~VulkanGraphicsPipeline() override;

    void bind(const std::shared_ptr<CommandBuffer>& command_buffer);

    /// Builds the descriptor sets
    [[nodiscard]] bool bake() override;

    [[nodiscard]] std::shared_ptr<Framebuffer> target_framebuffer() const override;

    void add_input(std::string_view name, const std::shared_ptr<UniformBuffer>& ubo) override;
    void add_input(std::string_view name, const std::shared_ptr<Texture>& texture) override;
    void add_input(std::string_view name, const std::shared_ptr<Cubemap>& cubemap) override;

    [[nodiscard]] VkPipeline handle() const { return m_pipeline; }
    [[nodiscard]] VkPipelineLayout layout() const;

  private:
    VkPipeline m_pipeline{};

    std::shared_ptr<VulkanShader> m_shader;
    std::shared_ptr<Framebuffer> m_target_framebuffer;

    // Descriptors and descriptor sets
    std::shared_ptr<VulkanDescriptorAllocator> m_allocator;

    std::vector<std::pair<VulkanDescriptorInfo, VkDescriptorBufferInfo>> m_buffer_descriptor_info;
    std::vector<std::pair<VulkanDescriptorInfo, VkDescriptorImageInfo>> m_image_descriptor_info;

    VkDescriptorSet m_set{VK_NULL_HANDLE};

    [[nodiscard]] static VkCompareOp get_depth_compare_op(DepthCompareOp op);
    [[nodiscard]] static VkFrontFace get_rasterization_front_face(FrontFace face);
};

} // namespace Phos
