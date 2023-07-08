#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>

#include "renderer/backend/material.h"
#include "renderer/backend/vulkan/vulkan_shader.h"

namespace Phos {

// Forward declarations
class VulkanShader;
class VulkanTexture;
class VulkanCommandBuffer;
class VulkanUniformBuffer;
class VulkanDescriptorAllocator;
struct VulkanDescriptorInfo;

class VulkanMaterial : public Material {
  public:
    explicit VulkanMaterial(const std::shared_ptr<Shader>& shader, std::string name);
    ~VulkanMaterial() override = default;

    void set(const std::string& name, glm::vec3 data) override;
    void set(const std::string& name, std::shared_ptr<Texture> texture) override;

    bool bake() override;

    void bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const;

  private:
    std::shared_ptr<VulkanShader> m_shader;
    std::string m_name;

    std::unordered_map<std::string, VulkanDescriptorInfo> m_name_to_descriptor_info;

    std::unordered_map<std::string, std::shared_ptr<VulkanTexture>> m_textures;
    std::unordered_map<std::string, std::shared_ptr<VulkanUniformBuffer>> m_uniform_buffers;

    VkDescriptorSet m_set{VK_NULL_HANDLE};
    std::shared_ptr<VulkanDescriptorAllocator> m_allocator;
};

} // namespace Phos
