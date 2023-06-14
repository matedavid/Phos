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

struct VulkanDescriptorInfo {
    std::string name;
    VkDescriptorType type;
    VkShaderStageFlags stage;
    uint32_t binding;
};

class VulkanShader : public Shader {
  public:
    VulkanShader(const std::string& vertex_path, const std::string& fragment_path);
    ~VulkanShader() override;

    [[nodiscard]] VkPipelineShaderStageCreateInfo get_vertex_create_info() const;
    [[nodiscard]] VkPipelineShaderStageCreateInfo get_fragment_create_info() const;

    [[nodiscard]] std::optional<VulkanDescriptorInfo> descriptor_info(std::string_view name) const;

    [[nodiscard]] std::optional<VkVertexInputBindingDescription> get_binding_description() const {
        return m_binding_description;
    }
    [[nodiscard]] std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions() const {
        return m_attribute_descriptions;
    }
    [[nodiscard]] std::vector<VkDescriptorSetLayout> get_descriptor_set_layouts() const {
        return m_descriptor_set_layouts;
    }
    [[nodiscard]] std::vector<VkPushConstantRange> get_push_constant_ranges() const { return m_push_constant_ranges; }

  private:
    VkShaderModule m_vertex_shader{};
    VkShaderModule m_fragment_shader{};

    std::optional<VkVertexInputBindingDescription> m_binding_description;
    std::vector<VkVertexInputAttributeDescription> m_attribute_descriptions;
    std::vector<VkDescriptorSetLayout> m_descriptor_set_layouts;
    std::vector<VkPushConstantRange> m_push_constant_ranges;

    std::unordered_map<std::string, VulkanDescriptorInfo> m_descriptor_info;

    [[nodiscard]] std::vector<char> read_shader_file(const std::string& path) const;

    void retrieve_vertex_input_info(const SpvReflectShaderModule& module);
    void retrieve_descriptor_sets_info(const SpvReflectShaderModule& vertex_module,
                                       const SpvReflectShaderModule& fragment_module);
    void retrieve_push_constants(const SpvReflectShaderModule& vertex_module,
                                 const SpvReflectShaderModule& fragment_module);
};

} // namespace Phos
