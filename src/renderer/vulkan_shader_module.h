#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>
#include <memory>

// Forward declarations
class VulkanDevice;

class VulkanShaderModule {
  public:
    enum class Stage { Vertex, Fragment };

    VulkanShaderModule(const std::string& path, Stage stage, std::shared_ptr<VulkanDevice> device);
    ~VulkanShaderModule();

    [[nodiscard]] VkPipelineShaderStageCreateInfo get_shader_stage_create_info() const;

  private:
    VkShaderModule m_shader{};
    VkShaderStageFlagBits m_stage;

    std::shared_ptr<VulkanDevice> m_device;

    [[nodiscard]] std::vector<char> read_shader_file(const std::string& path) const;
    [[nodiscard]] VkShaderStageFlagBits get_vulkan_stage(Stage stage) const;
};
