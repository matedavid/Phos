#pragma once

#include "vk_core.h"

#include <vulkan/vulkan.h>

#include "renderer/backend/material.h"
#include "renderer/backend/vulkan/vulkan_shader.h"

namespace Phos {

// Forward declarations
class VulkanShader;
class VulkanCommandBuffer;
class VulkanDescriptorAllocator;
struct VulkanDescriptorInfo;

class VulkanMaterial : public Material {
  public:
    explicit VulkanMaterial(const Definition& definition);
    ~VulkanMaterial() override = default;

    void bind(const std::shared_ptr<VulkanCommandBuffer>& command_buffer) const;

  private:
    Definition m_definition;
    std::shared_ptr<VulkanShader> m_shader;

    std::vector<std::pair<VulkanDescriptorInfo, VkDescriptorImageInfo>> m_image_descriptor_info;

    VkDescriptorSet m_set{VK_NULL_HANDLE};
    std::shared_ptr<VulkanDescriptorAllocator> m_allocator;
};

} // namespace Phos
