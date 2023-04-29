#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <optional>
#include <unordered_map>

// Forward declarations
class VulkanDevice;
class VulkanDescriptorLayoutCache;
struct SpvReflectShaderModule;

class VulkanShaderModule {
  public:
    enum class Stage {
        Vertex,
        Fragment
    };

    VulkanShaderModule(const std::string& path, Stage stage);
    ~VulkanShaderModule();

    [[nodiscard]] Stage get_stage() const { return m_stage; }
    [[nodiscard]] static VkShaderStageFlagBits get_vulkan_stage(Stage stage);

    [[nodiscard]] VkPipelineShaderStageCreateInfo get_shader_stage_create_info() const;

    [[nodiscard]] std::optional<VkVertexInputBindingDescription> get_binding_description() const {
        return m_binding_description;
    }
    [[nodiscard]] std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions() const {
        return m_attribute_descriptions;
    }
    [[nodiscard]] std::vector<VkDescriptorSetLayout> get_descriptor_sets_layout() const {
        return m_descriptor_sets_layout;
    }

  private:
    VkShaderModule m_shader{};
    Stage m_stage;

    std::optional<VkVertexInputBindingDescription> m_binding_description;
    std::vector<VkVertexInputAttributeDescription> m_attribute_descriptions;
    std::vector<VkDescriptorSetLayout> m_descriptor_sets_layout;

    VkDescriptorPool m_descriptor_pool{};
    std::vector<VkDescriptorSet> m_descriptor_sets;
    std::unordered_map<std::string, uint32_t> m_descriptor_set_index;

    [[nodiscard]] std::vector<char> read_shader_file(const std::string& path) const;

    void retrieve_vertex_input_info(const SpvReflectShaderModule& module);
    void retrieve_descriptor_sets_info(const SpvReflectShaderModule& module);
};
