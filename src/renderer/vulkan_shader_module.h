#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

// Forward declarations
class VulkanDevice;

class VulkanShaderModule {
  public:
    enum class Stage { Vertex, Fragment };

    VulkanShaderModule(const std::string& path, Stage stage, std::shared_ptr<VulkanDevice> device);
    ~VulkanShaderModule();

    [[nodiscard]] VkPipelineShaderStageCreateInfo get_shader_stage_create_info() const;

    [[nodiscard]] VkVertexInputBindingDescription get_binding_description() const { return m_binding_description; }
    [[nodiscard]] std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions() const {
        return m_attribute_descriptions;
    }

  private:
    VkShaderModule m_shader{};
    Stage m_stage;

    std::shared_ptr<VulkanDevice> m_device;

    VkVertexInputBindingDescription m_binding_description{};
    std::vector<VkVertexInputAttributeDescription> m_attribute_descriptions;

    [[nodiscard]] std::vector<char> read_shader_file(const std::string& path) const;
    [[nodiscard]] VkShaderStageFlagBits get_vulkan_stage(Stage stage) const;

    void retrieve_vertex_input_info(const std::vector<char>& content);
};
