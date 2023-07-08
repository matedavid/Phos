#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <optional>
#include <unordered_map>

#include "renderer/backend/shader.h"

// Forward declarations
struct SpvReflectShaderModule;
struct SpvReflectDescriptorBinding;

namespace Phos {

// Forward declarations
class VulkanDevice;
class VulkanDescriptorLayoutCache;

struct VulkanUniformBufferMember {
    std::string name;
    uint32_t size;
    uint32_t offset;
};

struct VulkanDescriptorInfo {
    std::string name;
    VkDescriptorType type;
    VkShaderStageFlags stage;
    uint32_t set;
    uint32_t binding;
    uint32_t size;

    std::vector<VulkanUniformBufferMember> members;
};

class VulkanShader : public Shader {
  public:
    VulkanShader(const std::string& vertex_path, const std::string& fragment_path);
    ~VulkanShader() override;

    [[nodiscard]] VkPipelineShaderStageCreateInfo get_vertex_create_info() const;
    [[nodiscard]] VkPipelineShaderStageCreateInfo get_fragment_create_info() const;

    [[nodiscard]] std::optional<VulkanDescriptorInfo> descriptor_info(std::string_view name) const;
    [[nodiscard]] std::vector<VulkanDescriptorInfo> descriptors_in_set(uint32_t set) const;

    [[nodiscard]] std::optional<VkVertexInputBindingDescription> get_binding_description() const {
        return m_binding_description;
    }
    [[nodiscard]] std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions() const {
        return m_attribute_descriptions;
    }

    [[nodiscard]] VkPipelineLayout get_pipeline_layout() const { return m_pipeline_layout; }

  private:
    VkShaderModule m_vertex_shader{};
    VkShaderModule m_fragment_shader{};

    std::optional<VkVertexInputBindingDescription> m_binding_description;
    std::vector<VkVertexInputAttributeDescription> m_attribute_descriptions;
    std::vector<VkDescriptorSetLayout> m_descriptor_set_layouts;
    std::vector<VkPushConstantRange> m_push_constant_ranges;

    VkPipelineLayout m_pipeline_layout;

    std::unordered_map<std::string, VulkanDescriptorInfo> m_descriptor_info;

    [[nodiscard]] std::vector<char> read_shader_file(const std::string& path) const;

    void retrieve_vertex_input_info(const SpvReflectShaderModule& module);
    void retrieve_descriptor_sets_info(const SpvReflectShaderModule& vertex_module,
                                       const SpvReflectShaderModule& fragment_module);
    void retrieve_push_constants(const SpvReflectShaderModule& vertex_module,
                                 const SpvReflectShaderModule& fragment_module);

    void create_pipeline_layout();
};

} // namespace Phos
